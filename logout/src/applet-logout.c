/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-reboot-required.h"
#include "applet-timer.h"
#include "applet-logout.h"


static void cd_logout_shut_down_full (gboolean bDemandConfirmation);
static void cd_logout_restart (void);
static void cd_logout_suspend (GtkMenuItem*, gpointer);
static void cd_logout_hibernate (GtkMenuItem*, gpointer);
static void cd_logout_hybridSleep (GtkMenuItem*, gpointer);
static void cd_logout_switch_to_user (GtkMenuItem*, const gchar *cUser);
static void cd_logoout_lock_screen (void);
static void cd_logout_switch_to_greeter (GtkMenuItem*, gpointer);


/*
 * New user switching: use only logind (DisplayManager interfaces do not exist anymore)
 * org.freedesktop.login1 interface:
 * /org/freedesktop/login1 -> org.freedesktop.login1.Manager:
 * 		-> ListSessions method (returns ID, object path (not needed) and user)
 * 		-> SessionNew signal: only ID and object path, need to get user separately
 * + get user names from org.freedesktop.Accounts (as done currently)
 * 
 * Switch to login screen
 * org.gnome.DisplayManager -> /org/gnome/DisplayManager/LocalDisplayFactory
 *   -> org.gnome.DisplayManager.LocalDisplayFactory -> CreateTransientDisplay
 * (not sure how to do for other cases -- logind allows VT switching, but we
 * would need to guess the VT number)
 *
 * If there is at least one more session or we can switch to login screen:
 * display menu, with the user names (typical case: only one session per user)
 * + "new session" to activate the login screen
 */



  ////////////
 /// MENU ///
////////////

gchar *cd_logout_check_icon (const gchar *cIconStock, gint iIconSize)
{
	gchar *cImagePath = cairo_dock_search_icon_s_path (cIconStock, iIconSize);
	if (cImagePath != NULL && g_file_test (cImagePath, G_FILE_TEST_EXISTS))
		return cImagePath;
	else
		return NULL;
}


static GtkWidget *_build_menu (GtkWidget **pShutdownMenuItem)
{
	GtkWidget *pMenu = gldi_menu_new (myIcon);
	
	GtkWidget *pMenuItem;
	
	gboolean bCanShutdown;
	gboolean bCanReboot;
	gboolean bCanLogout;
	gboolean bCanSuspend;
	gboolean bCanHibernate;
	gboolean bCanHybridSleep;
	gboolean bCanLockScreen;
	
	cairo_dock_fm_can_shutdown_reboot_logout (
		&bCanShutdown,
		&bCanReboot,
		&bCanLogout,
		&bCanSuspend,
		&bCanHibernate,
		&bCanHybridSleep,
		&bCanLockScreen);

	gchar *cImagePath;
	cImagePath = cd_logout_check_icon ("system-shutdown", myData.iDesiredIconSize);
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Shut down"), cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/system-shutdown.svg", cd_logout_shut_down, pMenu);
	g_free (cImagePath);
	if (!bCanShutdown && ! myConfig.cUserActionShutdown)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	*pShutdownMenuItem = pMenuItem;
	
	cImagePath = cd_logout_check_icon (GLDI_ICON_NAME_REFRESH, myData.iDesiredIconSize);
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Restart"), cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/system-restart.svg", cd_logout_restart, pMenu);
	g_free (cImagePath);
	if (!bCanReboot)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	cImagePath = cd_logout_check_icon ("sleep", myData.iDesiredIconSize);
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_TOOLTIP_AND_DATA (D_("Hibernate"),
		cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/system-hibernate.svg",
		D_("Your computer will not consume any energy."),
		cd_logout_hibernate, pMenu, NULL);
	if (!bCanHibernate)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	if (bCanHybridSleep)
		CD_APPLET_ADD_IN_MENU_WITH_TOOLTIP_AND_DATA (D_("Hybrid Sleep"),
			cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/system-hibernate.svg",
			D_("Your computer will still consume a small amount of energy but after some time, the computer will suspend to disk and not consume any energy."),
			cd_logout_hybridSleep, pMenu, NULL);
	g_free (cImagePath);
	
	cImagePath = cd_logout_check_icon ("clock", myData.iDesiredIconSize);
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_TOOLTIP_AND_DATA (D_("Suspend"),
		cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/system-suspend.svg",
		D_("Your computer will still consume a small amount of energy."),
		cd_logout_suspend, pMenu, NULL);
	g_free (cImagePath);
	if (!bCanSuspend)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	cImagePath = cd_logout_check_icon ("system-log-out", myData.iDesiredIconSize);
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_TOOLTIP_AND_DATA (D_("Log out"),
		cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/system-log-out.svg",
		D_("Close your session and allow to open a new one."),
		cd_logout_close_session, pMenu, NULL);
	g_free (cImagePath);
	if (!bCanLogout && !myConfig.cUserAction)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	// display the switch account menu
	gboolean bCanSwitchToLogin = (myData.pGdmProxy != NULL); //!! TODO: chech if there are actually multiple users?
	gboolean bMultipleSession = FALSE;
	GList *it;
	CDLogoutSession *session;
	for (it = myData.pSessionList; it; it = it->next)
	{
		//!! TODO: user not added for all sessions !!
		session = (CDLogoutSession*)it->data;
		if (session->uid && session->pUser && (session->pUser->cUserName || session->pUser->cRealName) &&
			!session->bOwnSession)
		{
			// only display the menu to switch if there is at least one session with a
			// known user which is not the current session
			bMultipleSession = TRUE;
			break;
		}
	}
	
	if (bMultipleSession)
	{
		GtkWidget *pUsersSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Switch user"), pMenu, GLDI_ICON_NAME_JUMP_TO);
		
		for (it = myData.pSessionList; it; it = it->next)
		{
			session = (CDLogoutSession*)it->data;
			if (session->uid && session->pUser && (session->pUser->cUserName || session->pUser->cRealName))
			{
				gchar *cID = NULL; // copy of the session ID to stay alive with the menu item
				if (!session->bOwnSession) cID = g_strdup (session->cID);
				
				pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_DATA (
					(session->pUser->cRealName && *session->pUser->cRealName) ?
						session->pUser->cRealName : session->pUser->cUserName,
					cd_logout_switch_to_user, pUsersSubMenu, cID);
				if (cID) g_object_set_data_full (G_OBJECT (pMenuItem), "cd-session-id", cID, g_free); // this is so that cID is not leaked
				else gtk_widget_set_sensitive (pMenuItem, FALSE); // disable switching to our own session
			}
		}
		
		if (bCanSwitchToLogin)
		{
			CD_APPLET_ADD_IN_MENU_WITH_TOOLTIP_AND_DATA (D_("New session"),
				NULL,
				D_("Switch to the login screen where you can start a new session."),
				cd_logout_switch_to_greeter, pUsersSubMenu, NULL);
		}
	}
	else if (bCanSwitchToLogin)
	{
		// just display one menu item to switch to the login screen
		CD_APPLET_ADD_IN_MENU_WITH_TOOLTIP_AND_DATA (D_("Switch user"),
			GLDI_ICON_NAME_JUMP_TO,
			D_("Switch to the login screen where you can start a new session."),
			cd_logout_switch_to_greeter, pMenu, NULL);
	}
	
	CD_APPLET_ADD_SEPARATOR_IN_MENU (pMenu);
	
	cImagePath = cd_logout_check_icon ("system-lock-screen", myData.iDesiredIconSize);
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Lock screen"), cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/locked.svg", cd_logoout_lock_screen, pMenu);
	g_free (cImagePath);
	if (!bCanLockScreen && !myConfig.cUserActionLock)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	if (bCanShutdown)
	{
		cImagePath = cd_logout_check_icon ("document-open-recent", myData.iDesiredIconSize);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Program an automatic shut-down"), cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/icon-scheduling.svg", cd_logout_program_shutdown, pMenu);
		g_free (cImagePath);
	}

	if ((myDock && myDock->container.bIsHorizontal && ! myDock->container.bDirectionUp)  // on the top, we inverse the menu (mainly to be close to what others do).
		|| (myDesklet && myDesklet->container.iWindowPositionY < (g_desktopGeometry.Xscreen.height / 2)))
	{
		GList *children = gtk_container_get_children (GTK_CONTAINER (pMenu));
		GList *pMenuList;
		for (pMenuList = children; pMenuList != NULL; pMenuList = pMenuList->next)
		{
			pMenuItem = pMenuList->data;
			gtk_menu_reorder_child (GTK_MENU (pMenu), pMenuItem, 0);
		}
		g_list_free (children);
	}

	return pMenu;
}


void cd_logout_display_actions (void)
{
	// build and show the menu
	GtkWidget *pShutdownMenuItem = NULL;
	GtkWidget *pMenu = _build_menu (&pShutdownMenuItem);
	CD_APPLET_POPUP_MENU_ON_MY_ICON (pMenu);
	
	// select the first (or last) item, which corresponds to the 'shutdown' action.
	gtk_menu_shell_select_item (GTK_MENU_SHELL (pMenu), pShutdownMenuItem);  // must be done here, after the menu has been realized.
}


  ///////////////
 /// ACTIONS ///
///////////////

typedef enum {
	CD_LOGOUT_LOGOUT,
	CD_LOGOUT_SHUTDOWN,
	CD_LOGOUT_REBOOT
} CDLogoutActionType;

static void _exec_action (int iClickedButton, GtkWidget *pInteractiveWidget, void (*callback) (void), CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // 'OK' button or 'Enter', execute the action.
		callback ();
	myData.pConfirmationDialog = NULL;
}

static gboolean _on_dialog_destroyed (G_GNUC_UNUSED gpointer pUserData, CairoDialog *pDialog)
{
	if (pDialog == myData.pConfirmationDialog) myData.pConfirmationDialog = NULL;
	return GLDI_NOTIFICATION_LET_PASS;
}

static void _demand_confirmation (const gchar *cMessage, const gchar *cIconStock, const gchar *cIconImage, void (*callback) (void))
{
	gchar *cImagePath = cd_logout_check_icon (cIconStock, 32); // dialog
	myData.pConfirmationDialog = gldi_dialog_show (cMessage, myIcon, myContainer, 0, cImagePath ? cImagePath : cIconImage, NULL, (CairoDockActionOnAnswerFunc) _exec_action, callback, NULL);
	gldi_object_register_notification (myData.pConfirmationDialog, NOTIFICATION_DESTROY, (GldiNotificationFunc) _on_dialog_destroyed, GLDI_RUN_AFTER, NULL);
	g_free (cImagePath);
}

static void _confirm_action (gpointer data, CairoDockFMUserActionFunc action)
{
	CDLogoutActionType iAction = GPOINTER_TO_INT (data);
	switch (iAction)
	{
		case CD_LOGOUT_LOGOUT:
			_demand_confirmation (D_("Close the current session?"), "system-log-out", MY_APPLET_SHARE_DATA_DIR"/system-log-out.svg", action);
			break;
		case CD_LOGOUT_REBOOT:
			_demand_confirmation (D_("Restart the computer?"), GLDI_ICON_NAME_REFRESH, MY_APPLET_SHARE_DATA_DIR"/system-restart.svg", action);
			break;
		case CD_LOGOUT_SHUTDOWN:
			//!! TODO: add back the timer?
			_demand_confirmation (D_("Shut down the computer?"), "system-shutdown", MY_APPLET_SHARE_DATA_DIR"/system-shutdown.svg", action);
			break;
	}
}

static void _shutdown_custom (void)
{
	gldi_object_notify (&myModuleObjectMgr, NOTIFICATION_LOGOUT);
	cairo_dock_launch_command (myConfig.cUserActionShutdown);
}

static void _logout_custom (void)
{
	gldi_object_notify (&myModuleObjectMgr, NOTIFICATION_LOGOUT);
	cairo_dock_launch_command (myConfig.cUserAction);
}

static void cd_logout_shut_down_full (gboolean bDemandConfirmation)
{
	if (myConfig.cUserActionShutdown)
	{
		if (bDemandConfirmation) _confirm_action (GINT_TO_POINTER (CD_LOGOUT_SHUTDOWN), _shutdown_custom);
		else _shutdown_custom ();
	}
	else
	{
		if (bDemandConfirmation) cairo_dock_fm_shutdown (_confirm_action, GINT_TO_POINTER (CD_LOGOUT_SHUTDOWN));
		else cairo_dock_fm_shutdown (NULL, NULL); // might still show confirmation dialog from the system
	}
}

void cd_logout_shut_down (void)
{
	cd_logout_shut_down_full (myConfig.bConfirmAction);
}
void cd_logout_timer_shutdown (void)
{
	cd_logout_shut_down_full (FALSE);
}

static void cd_logout_restart (void)
{
	if (myConfig.bConfirmAction)
	{
		cairo_dock_fm_reboot (_confirm_action, GINT_TO_POINTER (CD_LOGOUT_REBOOT));
	}
	else
	{
		cairo_dock_fm_reboot (NULL, NULL);
	}
}

static void cd_logout_suspend (G_GNUC_UNUSED GtkMenuItem *pMenuItem, G_GNUC_UNUSED gpointer dummy)
{
	cairo_dock_fm_suspend ();
}

static void cd_logout_hibernate (G_GNUC_UNUSED GtkMenuItem *pMenuItem, G_GNUC_UNUSED gpointer dummy)
{
	cairo_dock_fm_hibernate ();
}

static void cd_logout_hybridSleep (G_GNUC_UNUSED GtkMenuItem *pMenuItem, G_GNUC_UNUSED gpointer dummy)
{
	cairo_dock_fm_hybrid_sleep ();
}

void cd_logout_close_session (G_GNUC_UNUSED GtkMenuItem *pMenuItem, G_GNUC_UNUSED gpointer dummy)
{
	if (myConfig.cUserAction != NULL)
	{
		if (myConfig.bConfirmAction) _confirm_action (GINT_TO_POINTER (CD_LOGOUT_LOGOUT), _logout_custom);
		else _logout_custom ();
	}
	else
	{
		if (myConfig.bConfirmAction) cairo_dock_fm_logout (_confirm_action, GINT_TO_POINTER (CD_LOGOUT_LOGOUT));
		else cairo_dock_fm_logout (NULL, NULL);
	}
}

static void cd_logout_switch_to_user (G_GNUC_UNUSED GtkMenuItem *pMenuItem, const gchar *cID)
{
	CD_APPLET_ENTER;
	if (!myData.pLogin1Proxy)
	{
		cd_warning ("No connection to logind, cannot switch session");
		CD_APPLET_LEAVE ();
	}
	
	g_dbus_proxy_call (myData.pLogin1Proxy, "ActivateSession",
		g_variant_new ("(s)", cID),
		G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
		-1, // timeout
		NULL, // cancellable
		NULL, // callback -- we don't care of the result
		NULL);
	
	CD_APPLET_LEAVE ();
}

static void cd_logout_switch_to_greeter (G_GNUC_UNUSED GtkMenuItem *pMenuItem, G_GNUC_UNUSED gpointer dummy)
{
	CD_APPLET_ENTER;
	if (!myData.pGdmProxy)
	{
		cd_warning ("No connection to GDM, cannot switch to login screen");
		CD_APPLET_LEAVE ();
	}
	
	g_dbus_proxy_call (myData.pGdmProxy, "CreateTransientDisplay",
		NULL,
		G_DBUS_CALL_FLAGS_ALLOW_INTERACTIVE_AUTHORIZATION,
		-1, // timeout
		NULL, // cancellable
		NULL, // callback -- we don't care of the result
		NULL);
	
	CD_APPLET_LEAVE ();
}

static void cd_logoout_lock_screen (void)
{
	if (myConfig.cUserActionLock != NULL)
		cairo_dock_launch_command (myConfig.cUserActionLock);
	else
		cairo_dock_fm_lock_screen ();
}


  /////////////////////////////
 /// USER AND SESSION LIST ///
/////////////////////////////

/* -- maybe we could use this as a fallback if org.freedesktop.Accounts is not available
static GList* _get_users_list_fallback (void)
{
	// read from /etc/passwd
	gchar *cContent=NULL;
	gsize length = 0;
	g_file_get_contents ("/etc/passwd",
		&cContent,
		&length,
		NULL);
	g_return_val_if_fail (cContent != NULL, NULL);
	
	// parse eash user
	gchar **cUsers = g_strsplit (cContent, "\n", 0);
	GList *pUserList = NULL;
	gchar **cUserProps;
	CDUser *pUser;
	char *str;
	int i;
	for (i = 0; cUsers[i] != NULL; i ++)
	{
		cUserProps = g_strsplit (cUsers[i], ":", 0);
		// add the user if it fits
		if (cUserProps && cUserProps[0] && cUserProps[1] && cUserProps[2]
			&& atoi (cUserProps[2]) >= 1000 && atoi (cUserProps[2]) < 65530)  // heuristic: first user has an udi of 1000, and other users an uid >= 1000; remove the 'nobody' user (uid=65534)
		{
			pUser = g_new0 (CDUser, 1);
			pUser->cUserName = g_strdup (cUserProps[0]);
			pUser->cIconFile = NULL;
			pUser->cRealName = g_strdup (cUserProps[4]);
			if (pUser->cRealName)
			{
				str = strchr (pUser->cRealName, ',');
				if (str)  // remove the comments, separated by ','
					*str = '\0';
			}
			pUserList = g_list_insert_sorted (pUserList, pUser, (GCompareFunc)_compare_user_name);
		}
	}
	
	free (cContent);
	g_strfreev (cUsers);
	return pUserList;
}
*/

static void _free_user (gpointer ptr)
{
	CDLogoutUser *user = (CDLogoutUser*)ptr;
	g_free (user->cUserName);
	g_free (user->cRealName);
	g_free (user->cObjectPath);
	g_cancellable_cancel (user->pCancel);
	g_object_unref (G_OBJECT (user->pCancel));
	g_free (user);
}

static void _free_session (gpointer ptr)
{
	CDLogoutSession *session = (CDLogoutSession*)ptr;
	g_free (session->cID);
	if (session->pCancel)
	{
		g_cancellable_cancel (session->pCancel);
		g_object_unref (G_OBJECT (session->pCancel));
	}
	g_free (session);
}


static void _on_got_uid (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	GVariant *res = g_dbus_connection_call_finish (G_DBUS_CONNECTION (pObj), pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("Cannot get DBus property for user: %s", err->message);
			//!! TODO: remove user object?
		}
		g_error_free (err);
		CD_APPLET_LEAVE ();
	}
	
	// res is (v) where v should be an uint32
	CDLogoutUser *user = (CDLogoutUser*)ptr;
	gboolean bValid = FALSE;
	if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(v)")))
	{
		GVariant *tmp1 = g_variant_get_child_value (res, 0);
		GVariant *tmp2 = g_variant_get_variant (tmp1);
		if (g_variant_is_of_type (tmp2, G_VARIANT_TYPE ("t")))
		{
			user->uid = g_variant_get_uint64 (tmp2);
			bValid = TRUE;
		}
		g_variant_unref (tmp2);
		g_variant_unref (tmp1);
	}
	g_variant_unref (res);
	if (!bValid)
	{
		cd_warning ("Unexpected value for user ID property");
		CD_APPLET_LEAVE ();
	}
	
	// check if uid matches any sessions without a user
	GList *it;
	CDLogoutSession *session;
	for (it = myData.pSessionList; it; it = it->next)
	{
		session = (CDLogoutSession*)it->data;
		if (!session->pUser && session->uid == user->uid)
			session->pUser = user;
	}
	
	CD_APPLET_LEAVE ();
}

static void _on_got_user_name (GObject *pObj, GAsyncResult *pRes, gpointer ptr) // ptr points to either cUserName or cRealName
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	GVariant *res = g_dbus_connection_call_finish (G_DBUS_CONNECTION (pObj), pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("Cannot get DBus property for user: %s", err->message);
			//!! TODO: remove user object?
		}
		g_error_free (err);
		CD_APPLET_LEAVE ();
	}
	
	// res is (v) where v should be a string
	gboolean bValid = FALSE;
	if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(v)")))
	{
		GVariant *tmp1 = g_variant_get_child_value (res, 0);
		GVariant *tmp2 = g_variant_get_variant (tmp1);
		if (g_variant_is_of_type (tmp2, G_VARIANT_TYPE ("s")))
		{
			gchar **name = (gchar**)ptr;
			g_free (*name); // in case we call this multiple times (we should listen to changes in user names)
			*name = g_variant_dup_string (tmp2, NULL);
			bValid = TRUE;
		}
		g_variant_unref (tmp2);
		g_variant_unref (tmp1);
	}
	g_variant_unref (res);
	if (!bValid) cd_warning ("Unexpected value for user name property");
	
	CD_APPLET_LEAVE ();
}

static void _add_one_user (const gchar *cPath, GDBusConnection *pConn)
{
	GList *it = myData.pUserList;
	CDLogoutUser *user;
	for (; it; it = it->next)
	{
		user = (CDLogoutUser*) it->data;
		if (!strcmp (user->cObjectPath, cPath)) return;
	}
	
	user = g_new0 (CDLogoutUser, 1);
	user->pCancel = g_cancellable_new ();
	user->cObjectPath = g_strdup (cPath);
	myData.pUserList = g_list_prepend (myData.pUserList, user);
	// get the properties
	g_dbus_connection_call (pConn, "org.freedesktop.Accounts", cPath, "org.freedesktop.DBus.Properties",
		"Get", g_variant_new ("(ss)", "org.freedesktop.Accounts.User", "Uid"), G_VARIANT_TYPE ("(v)"),
		G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, user->pCancel, _on_got_uid, user);
	g_dbus_connection_call (pConn, "org.freedesktop.Accounts", cPath, "org.freedesktop.DBus.Properties",
		"Get", g_variant_new ("(ss)", "org.freedesktop.Accounts.User", "UserName"), G_VARIANT_TYPE ("(v)"),
		G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, user->pCancel, _on_got_user_name, &user->cUserName);
	g_dbus_connection_call (pConn, "org.freedesktop.Accounts", cPath, "org.freedesktop.DBus.Properties",
		"Get", g_variant_new ("(ss)", "org.freedesktop.Accounts.User", "RealName"), G_VARIANT_TYPE ("(v)"),
		G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, user->pCancel, _on_got_user_name, &user->cRealName);
}

static void _on_got_users (GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GDBusProxy *pProxy = G_DBUS_PROXY (pObj);
	GError *err = NULL;
	GVariant *res = g_dbus_proxy_call_finish (pProxy, pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("Cannot get DBus property for user: %s", err->message);
			//!! TODO: remove user account proxy?
		}
		g_error_free (err);
		CD_APPLET_LEAVE ();
	}
	
	// result should be an array of object path: (ao)
	if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(ao)")))
	{
		const gchar **objs = NULL;
		g_variant_get (res, "(^a&o)", &objs);
		if (objs) // can be NULL for empty array
		{
			GDBusConnection *pConn = g_dbus_proxy_get_connection (pProxy);
			int i;
			for (i = 0; objs[i]; i++) _add_one_user (objs[i], pConn);
			g_free (objs);
		}
	}
	else cd_warning ("Unexpected return type for the list of users");
	
	CD_APPLET_LEAVE ();
}

static void _accounts_signal (GDBusProxy *pProxy, G_GNUC_UNUSED const gchar *cSender, const gchar *cSignal,
	GVariant* pPar, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	
	gboolean bAdded = !strcmp (cSignal, "UserAdded");
	gboolean bRemoved = !bAdded && !strcmp (cSignal, "UserDeleted");
	
	if (bAdded || bRemoved)
	{
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE ("(o)")))
		{
			const gchar *cPath = NULL;
			g_variant_get (pPar, "(&o)", &cPath);
			if (cPath && *cPath)
			{
				if (bAdded) _add_one_user (cPath, g_dbus_proxy_get_connection (pProxy));
				else
				{
					// find and remove this user and any session related to them
					GList *it;
					CDLogoutUser *user = NULL;
					CDLogoutSession *session;
					for (it = myData.pUserList; it; it = it->next)
					{
						user = (CDLogoutUser*)it->data;
						if (!strcmp (user->cObjectPath, cPath)) break;
					}
					
					if (it)
					{
						myData.pUserList = g_list_delete_link (myData.pUserList, it); // frees it but not user
						// remove match from any session
						for (it = myData.pSessionList; it; it = it->next)
						{
							session = (CDLogoutSession*)it->data;
							if (session->pUser == user) session->pUser = NULL;
						}
						_free_user (user);
					}
					// note: not an error if not found? (could be a user removed before we got the initial list)
				}
			}
			else cd_warning ("Empty object path");
		}
		else cd_warning ("Unexpected parameter for '%s' signal", cSignal);
	}
	
	CD_APPLET_LEAVE ();
}

static void _on_user_changed (GDBusConnection *pConn, G_GNUC_UNUSED const gchar* cSender,
	const gchar* cPath, G_GNUC_UNUSED const gchar* cInterface, G_GNUC_UNUSED const gchar* cSignal,
	G_GNUC_UNUSED GVariant* pPar, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	// find the user among our known users (it is OK if not found as we might not have received this user yet)
	GList *it;
	CDLogoutUser *user;
	for (it = myData.pUserList; it; it = it->next)
	{
		user = (CDLogoutUser*)it->data;
		if (!strcmp (user->cObjectPath, cPath))
		{
			// found, update the name properties (user IDs should not change)
			g_dbus_connection_call (pConn, "org.freedesktop.Accounts", cPath, "org.freedesktop.DBus.Properties",
				"Get", g_variant_new ("(ss)", "org.freedesktop.Accounts.User", "UserName"), G_VARIANT_TYPE ("(v)"),
				G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, user->pCancel, _on_got_user_name, &user->cUserName);
			g_dbus_connection_call (pConn, "org.freedesktop.Accounts", cPath, "org.freedesktop.DBus.Properties",
				"Get", g_variant_new ("(ss)", "org.freedesktop.Accounts.User", "RealName"), G_VARIANT_TYPE ("(v)"),
				G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, user->pCancel, _on_got_user_name, &user->cRealName);
			break;
		}
	}
	
	CD_APPLET_LEAVE ();
}


static void _add_user_to_session (CDLogoutSession *session)
{
	// check if user is already known
	GList *it;
	CDLogoutUser *user;
	for (it = myData.pUserList; it; it = it->next)
	{
		user = (CDLogoutUser*)it->data;
		if (user->uid == session->uid)
		{
			session->pUser = user;
			break;
		}
	}
}

static void _on_got_sessions (GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("Cannot get DBus property for user: %s", err->message);
			//!! TODO: remove session proxy?
		}
		g_error_free (err);
		CD_APPLET_LEAVE ();
	}
	
	// result should be an array: (a(susso))
	if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(a(susso))")))
	{
		GVariantIter *it = NULL;
		g_variant_get (res, "(a(susso))", &it);
		const gchar *cID;
		guint32 uid;
		while (g_variant_iter_loop (it, "(&susso)", &cID, &uid, NULL, NULL, NULL))
		{
			// we assume that the session does not exist
			CDLogoutSession *session = g_new0 (CDLogoutSession, 1);
			session->cID = g_strdup (cID);
			session->uid = uid;
			if (myData.cOwnSessionID && !strcmp (session->cID, myData.cOwnSessionID))
				session->bOwnSession = TRUE;
			myData.pSessionList = g_list_prepend (myData.pSessionList, session);
			
			// check if any users match
			_add_user_to_session (session);
		}
	}
	else cd_warning ("Unexpected return type for the list of sessions");
	
	CD_APPLET_LEAVE ();
}

static void _on_got_session_uid (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	GVariant *res = g_dbus_connection_call_finish (G_DBUS_CONNECTION (pObj), pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("Cannot get DBus property for user: %s", err->message);
			//!! TODO: remove session object?
		}
		g_error_free (err);
		CD_APPLET_LEAVE ();
	}
	
	// res is (v) where v should be uo
	gboolean bValid = FALSE;
	if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(v)")))
	{
		GVariant *tmp1 = g_variant_get_child_value (res, 0);
		GVariant *tmp2 = g_variant_get_variant (tmp1);
		if (g_variant_is_of_type (tmp2, G_VARIANT_TYPE ("(uo)")))
		{
			bValid = TRUE;
			guint32 uid;
			CDLogoutSession *session = (CDLogoutSession*)ptr;
			g_variant_get (tmp2, "(uo)", &uid, NULL);
			session->uid = uid; // extend to 64 bits, org.freedesktop.Accounts IDs are 64-bit
			
			// check if any users match
			_add_user_to_session (session);
		}
		g_variant_unref (tmp2);
		g_variant_unref (tmp1);
	}
	if (!bValid) cd_warning ("Unexpected value for user ID property");
	g_variant_unref (res);
	
	CD_APPLET_LEAVE ();
}

static void _login1_signal (GDBusProxy *pProxy, G_GNUC_UNUSED const gchar *cSender, const gchar *cSignal,
	GVariant* pPar, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	
	gboolean bNew = !strcmp (cSignal, "SessionNew");
	gboolean bRemoved = !bNew && !strcmp (cSignal, "SessionRemoved");
	
	if (bNew || bRemoved)
	{
		if (g_variant_is_of_type (pPar, G_VARIANT_TYPE ("(so)")))
		{
			const gchar *cID = NULL, *cObj = NULL;
			g_variant_get (pPar, "(&s&o)", &cID, &cObj);
			if (cID && *cID && (!bNew || (cObj && *cObj)))
			{
				if (bRemoved)
				{
					GList *it;
					CDLogoutSession *session;
					for (it = myData.pSessionList; it; it = it->next)
					{
						session = (CDLogoutSession*)it->data;
						if (!strcmp (session->cID, cID))
						break;
					}
					
					if (it)
					{
						myData.pSessionList = g_list_delete_link (myData.pSessionList, it);
						_free_session (session);
					}
				}
				else if (bNew)
				{
					CDLogoutSession *session = g_new0 (CDLogoutSession, 1);
					session->cID = g_strdup (cID);
					if (myData.cOwnSessionID && !strcmp (session->cID, myData.cOwnSessionID))
						session->bOwnSession = TRUE;
					// note: uid is left at 0, which corresponds to root and should be ignored
					session->pCancel = g_cancellable_new ();
					myData.pSessionList = g_list_prepend (myData.pSessionList, session);
					// get the uid -- we need to read the corresponding DBus property
					GDBusConnection *pConn = g_dbus_proxy_get_connection (pProxy);
					g_dbus_connection_call (pConn, "org.freedesktop.login1", cObj, "org.freedesktop.DBus.Properties",
						"Get", g_variant_new ("(ss)", "org.freedesktop.login1.Session", "User"), G_VARIANT_TYPE ("(v)"),
						G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, session->pCancel, _on_got_session_uid, session);
				}
			}
			else cd_warning ("Empty session name or object path");
		}
		else cd_warning ("Unexpected parameter for '%s' signal", cSignal);
	}
	
	CD_APPLET_LEAVE ();
}

static void _on_got_own_session_id (GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	GVariant *res = g_dbus_connection_call_finish (G_DBUS_CONNECTION (pObj), pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("Cannot get DBus property for session id: %s", err->message);
			//!! TODO: remove session objects?
		}
		g_error_free (err);
		CD_APPLET_LEAVE ();
	}
	
	// res is (v) where v should be s
	if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(v)")))
	{
		GVariant *tmp1 = g_variant_get_child_value (res, 0);
		GVariant *tmp2 = g_variant_get_variant (tmp1);
		if (g_variant_is_of_type (tmp2, G_VARIANT_TYPE ("s")))
		{
			g_variant_get (tmp2, "s", &myData.cOwnSessionID);
			GList *it;
			CDLogoutSession *session;
			for (it = myData.pSessionList; it; it = it->next)
			{
				session = (CDLogoutSession*)it->data;
				if (!strcmp (session->cID, myData.cOwnSessionID))
				{
					session->bOwnSession = TRUE;
					break;
				}
			}
		}
		else cd_warning ("Unexpected property type for session ID: %s", g_variant_get_type_string (tmp2));
		
		g_variant_unref (tmp2);
		g_variant_unref (tmp1);
	}
	else cd_warning ("Unexpected property type for session ID (Get): %s", g_variant_get_type_string (res));
	
	g_variant_unref (res);
	
	CD_APPLET_LEAVE ();
}

static void _on_got_own_session_path (GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GDBusProxy *pProxy = G_DBUS_PROXY (pObj);
	GError *err = NULL;
	GVariant *res = g_dbus_proxy_call_finish (pProxy, pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		{
			cd_warning ("Cannot get own session path: %s", err->message);
		}
		g_error_free (err);
		CD_APPLET_LEAVE ();
	}
	
	if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(o)")))
	{
		const char *cPath = NULL;
		g_variant_get (res, "(&o)", &cPath);
		if (cPath && *cPath)
		g_dbus_connection_call (g_dbus_proxy_get_connection (pProxy),
			"org.freedesktop.login1", cPath, "org.freedesktop.DBus.Properties",
			"Get", g_variant_new ("(ss)", "org.freedesktop.login1.Session", "Id"), G_VARIANT_TYPE ("(v)"),
			G_DBUS_CALL_FLAGS_NO_AUTO_START, -1, myData.pCancellable, _on_got_own_session_id, NULL);
	}
	else cd_warning ("Unexpected return type for GetSession: %s", g_variant_get_type_string (res));
	g_variant_unref (res);
	
	CD_APPLET_LEAVE ();
}


static void _on_accounts_proxy_created (G_GNUC_UNUSED GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	GDBusProxy *pProxy = g_dbus_proxy_new_for_bus_finish (pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Cannot create DBus proxy: %s", err->message);
		CD_APPLET_LEAVE ();
	}
	
	gchar *tmp = g_dbus_proxy_get_name_owner (pProxy);
	if (!tmp)
	{
		cd_message ("No name owner for 'org.freedesktop.Accounts', will not track users");
		g_object_unref (G_OBJECT (pProxy));
		CD_APPLET_LEAVE (); //!! TODO: cancel creating the login1 proxy (it is useless without the user list)? Or just read /etc/passwd...
	}
	g_free (tmp);
	
	myData.pAccountsProxy = pProxy;
	
	// connect to signals + list initial users
	g_signal_connect (G_OBJECT(pProxy), "g-signal", G_CALLBACK (_accounts_signal), NULL);
	myData.uRegUserChanged = g_dbus_connection_signal_subscribe (
		g_dbus_proxy_get_connection (pProxy),
		"org.freedesktop.Accounts",
		"org.freedesktop.Accounts.User",
		"Changed",
		NULL, // object path -- any object (i.e. all users)
		NULL, // arg0 -- not needed
		G_DBUS_SIGNAL_FLAGS_NONE,
		_on_user_changed,
		NULL, // user data
		NULL);
	g_dbus_proxy_call (pProxy, "ListCachedUsers", NULL, G_DBUS_CALL_FLAGS_NONE, -1, myData.pCancellable, _on_got_users, NULL);
	
	CD_APPLET_LEAVE ();
}

static void _on_login1_proxy_created (G_GNUC_UNUSED GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	GDBusProxy *pProxy = g_dbus_proxy_new_for_bus_finish (pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Cannot create DBus proxy: %s", err->message);
		CD_APPLET_LEAVE ();
	}
	
	gchar *tmp = g_dbus_proxy_get_name_owner (pProxy);
	if (!tmp)
	{
		cd_message ("No name owner for 'org.freedesktop.login1', will not track sessions");
		g_object_unref (G_OBJECT (pProxy));
		CD_APPLET_LEAVE (); //!! TODO: cancel creating the Accounts proxy as there is not use for it
	}
	g_free (tmp);
	
	myData.pLogin1Proxy = pProxy;
	
	// connect to signals + list sessions
	g_signal_connect (G_OBJECT(pProxy), "g-signal", G_CALLBACK (_login1_signal), NULL);
	g_dbus_proxy_call (pProxy, "ListSessions", NULL, G_DBUS_CALL_FLAGS_NONE, -1, myData.pCancellable, _on_got_sessions, NULL);
	
	const gchar *cID = g_getenv ("XDG_SESSION_ID");
	if (cID) myData.cOwnSessionID = g_strdup (cID);
	else g_dbus_proxy_call (pProxy, "GetSession", g_variant_new ("(s)", "auto"), G_DBUS_CALL_FLAGS_NONE,
		-1, myData.pCancellable, _on_got_own_session_path, NULL);
	
	CD_APPLET_LEAVE ();
}


static void _on_gdm_proxy_created (G_GNUC_UNUSED GObject *pObj, GAsyncResult *pRes, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	GDBusProxy *pProxy = g_dbus_proxy_new_for_bus_finish (pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Cannot create DBus proxy: %s", err->message);
		CD_APPLET_LEAVE ();
	}
	
	gchar *tmp = g_dbus_proxy_get_name_owner (pProxy);
	if (!tmp)
	{
		cd_message ("No name owner for 'org.gnome.DisplayManager', will not be able to switch to the login screen");
		g_object_unref (G_OBJECT (pProxy));
		CD_APPLET_LEAVE ();
	}
	g_free (tmp);
	
	myData.pGdmProxy = pProxy;
	// nothing more to do, we just assume that it will work
	
	CD_APPLET_LEAVE ();
}

void cd_logout_start_watch_sessions (void)
{
	// connect our proxies
	myData.pCancellable = g_cancellable_new ();
	g_dbus_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES | G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
		NULL, // InterfaceInfo
		"org.freedesktop.Accounts",
		"/org/freedesktop/Accounts",
		"org.freedesktop.Accounts",
		myData.pCancellable,
		_on_accounts_proxy_created,
		NULL);
	g_dbus_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES | G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
		NULL, // InterfaceInfo
		"org.freedesktop.login1",
		"/org/freedesktop/login1",
		"org.freedesktop.login1.Manager",
		myData.pCancellable,
		_on_login1_proxy_created,
		NULL);
	g_dbus_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES | G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
		NULL, // InterfaceInfo
		"org.gnome.DisplayManager",
		"/org/gnome/DisplayManager/LocalDisplayFactory",
		"org.gnome.DisplayManager.LocalDisplayFactory",
		myData.pCancellable,
		_on_gdm_proxy_created,
		NULL);
}

void cd_logout_stop_watch_sessions (void)
{
	if (myData.pCancellable)
	{
		g_cancellable_cancel (myData.pCancellable);
		g_object_unref (G_OBJECT (myData.pCancellable));
	}
	if (myData.pAccountsProxy)
	{
		if (myData.uRegUserChanged) g_dbus_connection_signal_unsubscribe (
			g_dbus_proxy_get_connection (myData.pAccountsProxy), myData.uRegUserChanged);
		g_object_unref (G_OBJECT (myData.pAccountsProxy));
	}
	if (myData.pLogin1Proxy) g_object_unref (G_OBJECT (myData.pLogin1Proxy));
	if (myData.pGdmProxy) g_object_unref (G_OBJECT (myData.pGdmProxy));
	
	g_list_free_full (myData.pUserList, _free_user);
	g_list_free_full (myData.pSessionList, _free_session);
	g_free (myData.cOwnSessionID);
}

