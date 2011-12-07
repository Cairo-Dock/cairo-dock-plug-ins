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
#include <math.h>

#ifdef CD_UPOWER_AVAILABLE
#include <upower.h>
#endif

#include "applet-struct.h"
#include "applet-logout.h"


typedef struct {
	gchar *cUserName;
	gchar *cIconFile;
} CDUser;

static void cd_logout_shut_down (void);
static void cd_logout_restart (void);
static void cd_logout_suspend (void);
static void cd_logout_hibernate (void);
static void cd_logout_close_session (void);
static void cd_logout_switch_to_user (const gchar *cUser);
static void cd_logout_switch_to_guest (void);
static gboolean cd_logout_switch_to_greeter (void);

static void _free_user (CDUser *pUser);
static GList *cd_logout_get_users_list (void);

static void _display_menu (void);


  ////////////////////
 /// CAPABILITIES ///
////////////////////

static void _cd_logout_check_capabilities_async (CDSharedMemory *pSharedMemory)
{
	GError *error = NULL;
	
	// get capabilities from UPower.
	#ifdef CD_UPOWER_AVAILABLE
	UpClient *pUPowerClient = up_client_new ();
	up_client_get_properties_sync (pUPowerClient, NULL, &error);  // this function always returns false ...
	if (error)
	{
		cd_warning ("UPower error: %s", error->message);
		g_error_free (error);
		error = NULL;
	}
	else
	{
		pSharedMemory->bCanHibernate = up_client_get_can_hibernate (pUPowerClient);
		pSharedMemory->bCanSuspend = up_client_get_can_suspend (pUPowerClient);
	}
	g_object_unref (pUPowerClient);
	#endif
	
	// get capabilities from ConsoleKit.
	DBusGProxy *pProxy = cairo_dock_create_new_system_proxy (
		"org.freedesktop.ConsoleKit",
		"/org/freedesktop/ConsoleKit/Manager",
		"org.freedesktop.ConsoleKit.Manager");
	
	dbus_g_proxy_call (pProxy, "CanRestart", &error,
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN, &pSharedMemory->bCanRestart,
		G_TYPE_INVALID);
	if (error)
	{
		cd_warning ("ConsoleKit error: %s", error->message);
		g_error_free (error);
		g_object_unref (pProxy);
		return;
	}
	
	dbus_g_proxy_call (pProxy, "CanStop", &error,
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN, &pSharedMemory->bCanStop,
		G_TYPE_INVALID);
	if (error)
	{
		cd_warning ("ConsoleKit error: %s", error->message);
		g_error_free (error);
		g_object_unref (pProxy);
		return;
	}
	g_object_unref (pProxy);
	
	// get capabilities from DisplayManager
	const gchar *seat = g_getenv ("XDG_SEAT_PATH");
	if (seat)  // else, we could possibly get it by: ck -> GetCurrentSession -> session -> GetSeatId
	{
		pProxy = cairo_dock_create_new_system_proxy (
			"org.freedesktop.DisplayManager",
			seat,
			DBUS_INTERFACE_PROPERTIES);
		pSharedMemory->bHasGuestAccount = cairo_dock_dbus_get_property_as_boolean (pProxy, "org.freedesktop.DisplayManager.Seat", "HasGuestAccount");
	}
}

static gboolean _cd_logout_got_capabilities (CDSharedMemory *pSharedMemory)
{
	CD_APPLET_ENTER;
	
	// fetch the capabilities.
	myData.bCapabilitiesChecked = TRUE;
	myData.bCanHibernate = pSharedMemory->bCanHibernate;
	myData.bCanSuspend = pSharedMemory->bCanSuspend;
	myData.bCanRestart = pSharedMemory->bCanRestart;
	myData.bCanStop = pSharedMemory->bCanStop;
	myData.bHasGuestAccount = pSharedMemory->bHasGuestAccount;
	cd_debug ("capabilities: %d; %d; %d; %d; %d", myData.bCanHibernate, myData.bCanSuspend, myData.bCanRestart, myData.bCanStop, myData.bHasGuestAccount);
	
	// display the menu that has been asked beforehand.
	_display_menu ();
	
	// sayonara task-san ^^
	cairo_dock_discard_task (myData.pTask);
	myData.pTask = NULL;
	
	CD_APPLET_LEAVE (FALSE);
}


void cd_logout_display_actions (void)
{
	if (myData.pTask != NULL)
		return;
	if (! myData.bCapabilitiesChecked)
	{
		CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
		myData.pTask = cairo_dock_new_task_full (0,
			(CairoDockGetDataAsyncFunc) _cd_logout_check_capabilities_async,
			(CairoDockUpdateSyncFunc) _cd_logout_got_capabilities,
			(GFreeFunc) g_free,
			pSharedMemory);
		cairo_dock_launch_task (myData.pTask);
	}
	else
	{
		_display_menu ();
	}
}


  ////////////
 /// MENU ///
////////////

static void _switch_to_user (GtkMenuItem *menu_item, gchar *cUserName)
{
	if (cUserName != NULL)
	{
		cd_logout_switch_to_user (cUserName);
	}
	else  // guest
	{
		cd_logout_switch_to_guest ();
	}
}
static GtkWidget *_build_menu (void)
{
	GtkWidget *pMenu = gtk_menu_new ();
	
	GtkWidget *pMenuItem;
	
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Shut down"), MY_APPLET_SHARE_DATA_DIR"/system-shutdown.svg", cd_logout_shut_down, pMenu);
	if (!myData.bCanStop && ! myConfig.cUserAction2)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Restart"), MY_APPLET_SHARE_DATA_DIR"/system-restart.svg", cd_logout_restart, pMenu);
	if (!myData.bCanRestart)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Hibernate"), MY_APPLET_SHARE_DATA_DIR"/system-hibernate.svg", cd_logout_hibernate, pMenu);
	gtk_widget_set_tooltip_text (pMenuItem, D_("Your computer will not consume any energy."));
	if (!myData.bCanHibernate)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Suspend"), MY_APPLET_SHARE_DATA_DIR"/system-suspend.svg", cd_logout_suspend, pMenu);
	gtk_widget_set_tooltip_text (pMenuItem, D_("Your computer will still consume a small amount of energy."));
	if (!myData.bCanSuspend)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	if (g_getenv ("SESSION_MANAGER") != NULL)  // needs a session manager for this.
	{
		pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Log out"), MY_APPLET_SHARE_DATA_DIR"/system-log-out.svg", cd_logout_close_session, pMenu);
		gtk_widget_set_tooltip_text (pMenuItem, D_("Close your session and allow to open a new one."));
	}
	
	if (myData.pUserList != NULL)  // refresh the users list (we could listen for the UserAdded,UserDeleted, UserChanged signals too).
	{
		g_list_foreach (myData.pUserList, (GFunc)_free_user, NULL);
		g_list_free (myData.pUserList);
	}
	myData.pUserList = cd_logout_get_users_list ();
	if (myData.pUserList != NULL && (myData.bHasGuestAccount || myData.pUserList->next != NULL))  // at least 2 users
	{
		GtkWidget *pUsersSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Switch user"), pMenu, GTK_STOCK_JUMP_TO);
		
		gboolean bFoundUser = FALSE;
		const gchar *cCurrentUser = g_getenv ("USER");
		CDUser *pUser;
		GList *u;
		for (u = myData.pUserList; u != NULL; u = u->next)
		{
			pUser = u->data;
			pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (pUser->cUserName, pUser->cIconFile, _switch_to_user, pUsersSubMenu, pUser->cUserName);
			if (cCurrentUser && strcmp (cCurrentUser, pUser->cUserName) == 0)
			{
				bFoundUser = TRUE;
				gtk_widget_set_sensitive (pMenuItem, FALSE);
			}
		}
		
		if (myData.bHasGuestAccount && bFoundUser)  // if we didn't find the user yet, it means we are the guest, so don't show this entry.
		{
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Guest session"), NULL, _switch_to_user, pUsersSubMenu, NULL);  // NULL will mean "guest"
		}
	}
	
	CD_APPLET_ADD_SEPARATOR_IN_MENU (pMenu);
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Lock screen"), MY_APPLET_SHARE_DATA_DIR"/locked.svg", cairo_dock_fm_lock_screen, pMenu);  /// TODO: same question...
	
	if (myData.bCanStop)
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Program an automatic shut-down"), MY_APPLET_SHARE_DATA_DIR"/icon-scheduling.svg", cd_logout_program_shutdown, pMenu);

	if ((myDock && myDock->container.bIsHorizontal && ! myDock->container.bDirectionUp) // on the top, we inverse the menu
		|| (myDesklet && myDesklet->container.iWindowPositionY < (g_desktopGeometry.iXScreenHeight[CAIRO_DOCK_HORIZONTAL] / 2)))
	{
		GList *pMenuList;
		for (pMenuList = gtk_container_get_children (GTK_CONTAINER (pMenu)); pMenuList != NULL; pMenuList = pMenuList->next)
		{
			pMenuItem = pMenuList->data;
			gtk_menu_reorder_child (GTK_MENU (pMenu), pMenuItem, 0);
		}
	}

	return pMenu;
}

static void _display_menu (void)
{
	GtkWidget *pMenu = _build_menu ();
	CD_APPLET_POPUP_MENU_ON_MY_ICON (pMenu);
	gtk_menu_shell_select_first (GTK_MENU_SHELL (pMenu), FALSE);  // must be done here, after the menu has been realized.
}


  ////////////////////
 /// REBOOT TIMER ///
////////////////////

static gboolean _timer (gpointer data)
{
	CD_APPLET_ENTER;
	time_t t_cur = (time_t) time (NULL);
	if (t_cur >= myConfig.iShutdownTime)
	{
		cd_debug ("shutdown !\n");
		if (g_iDesktopEnv == CAIRO_DOCK_KDE)
			cairo_dock_launch_command ("dbus-send --session --type=method_call --dest=org.kde.ksmserver /KSMServer org.kde.KSMServerInterface.logout int32:0 int32:2 int32:2");
		else
			cairo_dock_launch_command ("dbus-send --system --print-reply --dest=org.freedesktop.ConsoleKit /org/freedesktop/ConsoleKit/Manager org.freedesktop.ConsoleKit.Manager.Stop");
		
		myData.iSidTimer = 0;
		CD_APPLET_LEAVE (FALSE);  // inutile de faire quoique ce soit d'autre, puisque l'ordi s'eteint.
	}
	else
	{
		cd_debug ("shutdown in %d minutes\n", (int) (myConfig.iShutdownTime - t_cur) / 60);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%dmn", (int) ceil ((double)(myConfig.iShutdownTime - t_cur) / 60.));
		CD_APPLET_REDRAW_MY_ICON;
		if (t_cur >= myConfig.iShutdownTime - 60)
			cairo_dock_show_temporary_dialog_with_icon (D_("Your computer will shut-down in 1 minute."), myIcon, myContainer, 8000, "same icon");
	}
	CD_APPLET_LEAVE (TRUE);
	
}
void cd_logout_set_timer (void)
{
	time_t t_cur = (time_t) time (NULL);
	if (myConfig.iShutdownTime > t_cur)
	{
		if (myData.iSidTimer == 0)
			myData.iSidTimer = g_timeout_add_seconds (60, _timer, NULL);
		_timer (NULL);
	}
	else if (myData.iSidTimer != 0)
	{
		g_source_remove (myData.iSidTimer);
		myData.iSidTimer = 0;
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
	}
}

void cd_logout_program_shutdown (void)
{
	int iDeltaT = (int) (cairo_dock_show_value_and_wait (D_("Choose in how many minutes your PC will stop:"), myIcon, myContainer, 30, 150) * 60);
	if (iDeltaT == -1)  // cancel
		CD_APPLET_LEAVE ();
	
	time_t t_cur = (time_t) time (NULL);
	if (iDeltaT > 0)
	{
		//g_print ("iShutdownTime <- %ld + %d\n", t_cur, iDeltaT);
		myConfig.iShutdownTime = (int) (t_cur + iDeltaT);
	}
	else if (iDeltaT == 0)  // on annule l'eventuel precedent.
	{
		myConfig.iShutdownTime = 0;
	}
	cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
		G_TYPE_INT, "Configuration", "shutdown time", myConfig.iShutdownTime,
		G_TYPE_INVALID);
	cd_logout_set_timer ();
}


  /////////////////////
 /// REBOOT NEEDED ///
/////////////////////

static void _set_reboot_message (void)
{
	gchar *cMessage = NULL;
	gsize length = 0;
	g_file_get_contents (CD_REBOOT_NEEDED_FILE,
		&cMessage,
		&length,
		NULL);
	if (cMessage != NULL)
	{
		int len = strlen (cMessage);
		if (cMessage[len-1] == '\n')
			cMessage[len-1] = '\0';
		CD_APPLET_SET_NAME_FOR_MY_ICON (cMessage);
	}
	else
	{
		if (myConfig.cDefaultLabel) // has another default name
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultLabel);
		else
			CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
	}
	g_free (cMessage);
}
void cd_logout_check_reboot_required (CairoDockFMEventType iEventType, const gchar *cURI, gpointer data)
{
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_MODIFIED:  // new message
			_set_reboot_message ();
		break;
		
		case CAIRO_DOCK_FILE_DELETED:  // reboot no more required (shouldn't happen)
			myData.bRebootNeeded = FALSE;
			if (myConfig.cDefaultLabel) // has another default name
				CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultLabel);
			else
				CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
			CD_APPLET_STOP_DEMANDING_ATTENTION;
		break;
		
		case CAIRO_DOCK_FILE_CREATED:  // reboot required
			myData.bRebootNeeded = TRUE;
			_set_reboot_message ();
			CD_APPLET_DEMANDS_ATTENTION ("pulse", 20);
			cairo_dock_show_temporary_dialog_with_icon (myIcon->cName, myIcon, myContainer, 5e3, "same icon");

			gchar *cImagePath = cairo_dock_search_icon_s_path (myConfig.cEmblemPath);
			if (! (cImagePath != NULL && g_file_test (cImagePath, G_FILE_TEST_EXISTS)))
				cImagePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/system-restart.svg");

			if (myConfig.iRebootNeededImage == CD_DISPLAY_EMBLEM)
				CD_APPLET_PRINT_OVERLAY_ON_MY_ICON (cImagePath, CAIRO_OVERLAY_UPPER_RIGHT);
			else
				CD_APPLET_SET_IMAGE_ON_MY_ICON (cImagePath);
			g_free (cImagePath);
		break;
		default:
		break;
	}
}

void cd_logout_check_reboot_required_init (void)
{
	if (g_file_test (CD_REBOOT_NEEDED_FILE, G_FILE_TEST_EXISTS))
	{
		cd_logout_check_reboot_required (CAIRO_DOCK_FILE_CREATED, CD_REBOOT_NEEDED_FILE, NULL);
	}
}


  ///////////////
 /// ACTIONS ///
///////////////

static void _console_kit_action (const gchar *cAction)
{
	GError *error = NULL;
	DBusGProxy *pProxy = cairo_dock_create_new_system_proxy (
		"org.freedesktop.ConsoleKit",
		"/org/freedesktop/ConsoleKit/Manager",
		"org.freedesktop.ConsoleKit.Manager");
	
	dbus_g_proxy_call (pProxy, cAction, &error,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
	if (error)
	{
		cd_warning ("ConsoleKit error: %s", error->message);
		g_error_free (error);
	}
	g_object_unref (pProxy);
}

static void _upower_action (gboolean bSuspend)
{
	#ifdef CD_UPOWER_AVAILABLE
	UpClient *pUPowerClient = up_client_new ();
	if (bSuspend)
		up_client_suspend_sync (pUPowerClient, NULL, NULL);
	else
		up_client_hibernate_sync (pUPowerClient, NULL, NULL);
	g_object_unref (pUPowerClient);
	#endif
}


static void _exec_action (int iClickedButton, GtkWidget *pInteractiveWidget, void (*callback) (void), CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // 'OK' button or 'Enter', execute the action.
		callback ();
	else if (myData.iSidShutDown != 0)  // 'Cancel' or 'Escap', if a countdown was scheduled, remove it.
	{
		g_source_remove (myData.iSidShutDown);
		myData.iSidShutDown = 0;
	}
	myData.pConfirmationDialog = NULL;
}
static void _demand_confirmation (const gchar *cMessage, const gchar *cIconImage, void (*callback) (void))
{
	myData.pConfirmationDialog = cairo_dock_show_dialog_full (cMessage, myIcon, myContainer, 0, cIconImage, NULL, (CairoDockActionOnAnswerFunc) _exec_action, callback, NULL);
}

static void _shut_down (void)
{
	if (myData.bCanStop)
	{
		_console_kit_action ("Stop");  // could use org.gnome.SessionManager.RequestShutdown, but it's not standard.
	}
	else if (myConfig.cUserAction2)
	{
		cairo_dock_launch_command (myConfig.cUserAction2);
	}
}
static inline gchar *_info_msg (void)
{
	gchar *cInfo = g_strdup_printf (D_("It will automatically shut-down in %ds"), myData.iCountDown);
	gchar *cMessage = g_strdup_printf ("%s\n\n (%s)", D_("Shut down the computer?"), cInfo);
	g_free (cInfo);
	return cMessage;
}
static gboolean _auto_shot_down (gpointer data)
{
	myData.iCountDown --;
	if (myData.iCountDown <= 0)
	{
		myData.iSidShutDown = 0;
		cairo_dock_dialog_unreference (myData.pConfirmationDialog);
		myData.pConfirmationDialog = NULL;
		_shut_down ();
		return FALSE;
	}
	else
	{
		if (myData.pConfirmationDialog)  // paranoia
		{
			gchar *cMessage = _info_msg ();
			cairo_dock_set_dialog_message (myData.pConfirmationDialog, cMessage);
			g_free (cMessage);
		}
		return TRUE;
	}
}
void cd_logout_shut_down (void)
{
	if (myConfig.bConfirmAction)
	{
		gchar *cMessage = _info_msg ();
		_demand_confirmation (cMessage, "system-shutdown.svg", _shut_down);
		g_free (cMessage);
		if (myData.iSidShutDown == 0)
		{
			myData.iCountDown = 60;
			myData.iSidShutDown = g_timeout_add_seconds (1, _auto_shot_down, NULL);
		}
	}
	else
	{
		_shut_down ();
	}
}

static void _restart (void)
{
	if (myData.bCanRestart)
	{
		_console_kit_action ("Restart");  // could use org.gnome.SessionManager.RequestReboot
	}
	else if (myConfig.cUserAction2)
	{
		cairo_dock_launch_command (myConfig.cUserAction2);
	}
}
void cd_logout_restart (void)
{
	if (myConfig.bConfirmAction)
	{
		_demand_confirmation (D_("Restart the computer?"), "system-restart.svg", _restart);
	}
	else
	{
		_restart ();
	}
}

void cd_logout_suspend (void)
{
	_upower_action (TRUE);
}

void cd_logout_hibernate (void)
{
	_upower_action (FALSE);
}

static void _logout (void)
{
	if (! cairo_dock_fm_logout ())
		cairo_dock_launch_command (MY_APPLET_SHARE_DATA_DIR"/logout.sh");  // SwitchToGreeter will only show the greeter, we want to close the session
}
void cd_logout_close_session (void)  // could use org.gnome.SessionManager.Logout
{
	if (myConfig.bConfirmAction)
	{
		_demand_confirmation (D_("Close the current session?"), MY_APPLET_SHARE_DATA_DIR"/system-log-out.svg", _logout);  /// same question, see above...
	}
	else
	{
		_logout ();
	}
}

static void cd_logout_switch_to_user (const gchar *cUser)
{
	const gchar *seat = g_getenv ("XDG_SEAT_PATH");
	if (seat)  // else, we could possibly get it by: ck -> GetCurrentSession -> session -> GetSeatId
	{
		GError *error = NULL;
		DBusGProxy *pProxy = cairo_dock_create_new_system_proxy (
			"org.freedesktop.DisplayManager",
			seat,
			"org.freedesktop.DisplayManager.Seat");
		dbus_g_proxy_call (pProxy, "SwitchToUser", &error,
			G_TYPE_STRING, cUser,
			G_TYPE_STRING, "",  // session, but actually it can be NULL so why bother?
			G_TYPE_INVALID,
			G_TYPE_INVALID);
		if (error)
		{
			cd_warning ("DisplayManager error: %s", error->message);
			g_error_free (error);
		}
		g_object_unref (pProxy);
	}
}

static void cd_logout_switch_to_guest (void)
{
	const gchar *seat = g_getenv ("XDG_SEAT_PATH");
	if (seat)  // else, we could possibly get it by: ck -> GetCurrentSession -> session -> GetSeatId
	{
		GError *error = NULL;
		DBusGProxy *pProxy = cairo_dock_create_new_system_proxy (
			"org.freedesktop.DisplayManager",
			seat,
			"org.freedesktop.DisplayManager.Seat");
		dbus_g_proxy_call (pProxy, "SwitchToGuest", &error,
			G_TYPE_STRING, "",  // session, but actually it can be NULL so why bother?
			G_TYPE_INVALID,
			G_TYPE_INVALID);
		if (error)
		{
			cd_warning ("DisplayManager error: %s", error->message);
			g_error_free (error);
		}
		g_object_unref (pProxy);
	}
}

static gboolean cd_logout_switch_to_greeter (void)  // not really sure how to use it.
{
	const gchar *seat = g_getenv ("XDG_SEAT_PATH");
	if (!seat)
		return FALSE;
	
	GError *error = NULL;
	DBusGProxy *pProxy = cairo_dock_create_new_system_proxy (
		"org.freedesktop.DisplayManager",
		seat,
		"org.freedesktop.DisplayManager.Seat");
	dbus_g_proxy_call (pProxy, "SwitchToGreeter", &error,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
	if (error)
	{
		cd_warning ("DisplayManager error: %s", error->message);
		g_error_free (error);
		g_object_unref (pProxy);
		return FALSE;
	}
	g_object_unref (pProxy);
	return TRUE;
}


  //////////////////
 /// USERS LIST ///
//////////////////

static void _free_user (CDUser *pUser)
{
	g_free (pUser->cUserName);
	g_free (pUser->cIconFile);
	g_free (pUser);
}

static int _compare_user_name (CDUser *pUser1, CDUser *pUser2)
{
	return strcmp (pUser1->cUserName, pUser2->cUserName);
}
GList *cd_logout_get_users_list (void)
{
	// get the list of users
	GError *error = NULL;
	DBusGProxy *pProxy = cairo_dock_create_new_system_proxy ("org.freedesktop.Accounts",
		"/org/freedesktop/Accounts",
		"org.freedesktop.Accounts");
	
	GPtrArray *users =  NULL;
	dbus_g_proxy_call (pProxy, "ListCachedUsers", &error,
		G_TYPE_INVALID,
		dbus_g_type_get_collection ("GPtrArray", DBUS_TYPE_G_OBJECT_PATH), &users,
		G_TYPE_INVALID);
	g_object_unref (pProxy);
	
	if (error)
	{
		cd_warning ("Accounts error: %s", error->message);
		g_error_free (error);
		return NULL;
	}
	if (users == NULL)
		return NULL;
	
	// foreach user, get its properties (name & icon).
	CDUser *pUser;
	GList *pUserList = NULL;
	gchar *cUserObjectPath;
	guint i;
	for (i = 0; i < users->len; i++)
	{
		cUserObjectPath = g_ptr_array_index (users, i);
		pProxy = cairo_dock_create_new_system_proxy ("org.freedesktop.Accounts",
			cUserObjectPath,
			DBUS_INTERFACE_PROPERTIES);
		
		pUser = g_new0 (CDUser, 1);
		pUser->cUserName = cairo_dock_dbus_get_property_as_string (pProxy, "org.freedesktop.Accounts.User", "UserName");  // used to identify the user in SwitchToUser()
		if (pUser->cUserName == NULL) // shouldn't happen
			continue;
		pUser->cIconFile = cairo_dock_dbus_get_property_as_string (pProxy, "org.freedesktop.Accounts.User", "IconFile");
		pUserList = g_list_insert_sorted (pUserList, pUser, (GCompareFunc)_compare_user_name);
		
		g_object_unref (pProxy);
	}
	return pUserList;
}
