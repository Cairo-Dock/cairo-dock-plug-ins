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

#ifdef CD_UPOWER_AVAILABLE
/* to access suspend/resume functionality on Upower 0.9
 * even if we use first logind, we only use logind via DBus
 */
#define UPOWER_ENABLE_DEPRECATED
#include <upower.h>
#endif

#include "applet-struct.h"
#include "applet-reboot-required.h"
#include "applet-timer.h"
#include "applet-logout.h"


typedef struct {
	gchar *cUserName;
	gchar *cIconFile;
	gchar *cRealName;
} CDUser;

static void cd_logout_shut_down_full (gboolean bDemandConfirmation);
static void cd_logout_shut_down (void);
static void cd_logout_restart (void);
static void cd_logout_suspend (void);
static void cd_logout_hibernate (void);
static void cd_logout_hybridSleep (void);
static void cd_logout_close_session (void);
static void cd_logout_switch_to_user (const gchar *cUser);
static void cd_logout_switch_to_guest (void);
static void cd_logoout_lock_screen (void);
//static gboolean cd_logout_switch_to_greeter (void);

static void _free_user (CDUser *pUser);
static GList *cd_logout_get_users_list (void);

static void _display_menu (const GdkEvent *pEvent);


  ////////////////////
 /// CAPABILITIES ///
////////////////////

/*
 * Check if the method (cMethod) exists and returns 'yes' or 'challenge':
 * http://www.freedesktop.org/wiki/Software/systemd/logind/
 *  If "yes" is returned the operation is supported and the user may execute the
 *   operation without further authentication.
 *  If "challenge" is returned the operation is available, but only after
 *   authorization.
 * If it exists, *bIsAble is modified and TRUE is returned.
 */
static gboolean _cd_logout_check_capabilities_logind (DBusGProxy *pProxy, const gchar *cMethod, gboolean *bIsAble)
{
	GError *error = NULL;
	gchar *cResult = NULL;
	dbus_g_proxy_call (pProxy, cMethod, &error,
		G_TYPE_INVALID,
		G_TYPE_STRING, &cResult,
		G_TYPE_INVALID);
	if (!error)
	{
		*bIsAble = (cResult && (strcmp (cResult, "yes") == 0
		            || strcmp (cResult, "challenge") == 0));
		g_free (cResult);
	}
	else
	{
		cd_debug ("Logind error: %s", error->message);
		g_error_free (error);
		return FALSE;
	}
	return TRUE;
}

static void _cd_logout_check_capabilities_async (CDSharedMemory *pSharedMemory)
{
	// test first with LoginD
	DBusGProxy *pProxy = cairo_dock_create_new_system_proxy (
		"org.freedesktop.login1",
		"/org/freedesktop/login1",
		"org.freedesktop.login1.Manager");

	const gchar *cLogindMethods[] = {"CanPowerOff", "CanReboot", "CanSuspend", "CanHibernate", "CanHybridSleep", NULL};
	gboolean *bCapabilities[] = {&pSharedMemory->bCanStop,
		&pSharedMemory->bCanRestart, &pSharedMemory->bCanSuspend,
		&pSharedMemory->bCanHibernate, &pSharedMemory->bCanHybridSleep};

	if (pProxy && _cd_logout_check_capabilities_logind (pProxy, cLogindMethods[0], bCapabilities[0]))
	{
		pSharedMemory->iLoginManager = CD_LOGIND;
		for (int i = 1; cLogindMethods[i] != NULL; i++)
			_cd_logout_check_capabilities_logind (pProxy, cLogindMethods[i], bCapabilities[i]);

		g_object_unref (pProxy);
	}
	else // then check with ConsoleKit and UPower
	{
		GError *error = NULL;

		// get capabilities from UPower: hibernate and suspend
		#ifdef CD_UPOWER_AVAILABLE
		UpClient *pUPowerClient = up_client_new ();
		if (pUPowerClient)
		{
			up_client_get_properties_sync (pUPowerClient, NULL, &error);  // this function always returns false ... and it crashes the dock (Debian 6) ! :-O
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
				g_object_unref (pUPowerClient); // not do that if there is an error to avoid: double free or corruption
			}
		}
		#endif
		
		// get capabilities from ConsoleKit.: reboot and poweroff
		pProxy = cairo_dock_create_new_system_proxy (
			"org.freedesktop.ConsoleKit",
			"/org/freedesktop/ConsoleKit/Manager",
			"org.freedesktop.ConsoleKit.Manager");
		
		dbus_g_proxy_call (pProxy, "CanRestart", &error,
			G_TYPE_INVALID,
			G_TYPE_BOOLEAN, &pSharedMemory->bCanRestart,
			G_TYPE_INVALID);
		if (!error)
		{
			pSharedMemory->iLoginManager = CD_CONSOLE_KIT;
			
			dbus_g_proxy_call (pProxy, "CanStop", &error,
				G_TYPE_INVALID,
				G_TYPE_BOOLEAN, &pSharedMemory->bCanStop,
				G_TYPE_INVALID);
			if (error)
			{
				cd_warning ("ConsoleKit error: %s", error->message);
				g_error_free (error);
				/*g_object_unref (pProxy); // should not happen but lets check guest account
				return;*/
			}
		}
		else
		{
			cd_debug ("ConsoleKit error: %s", error->message);
			g_error_free (error);
		}
		g_object_unref (pProxy);
	}

	// get capabilities from DisplayManager
	const gchar *seat = g_getenv ("XDG_SEAT_PATH");
	if (seat)  // else, we could possibly get it by: ck -> GetCurrentSession -> session -> GetSeatId
	{
		pProxy = cairo_dock_create_new_system_proxy (
			"org.freedesktop.DisplayManager",
			seat,
			DBUS_INTERFACE_PROPERTIES);
		pSharedMemory->bHasGuestAccount = cairo_dock_dbus_get_property_as_boolean (pProxy, "org.freedesktop.DisplayManager.Seat", "HasGuestAccount");
		g_object_unref (pProxy);
	}
	else
	{
		pSharedMemory->bHasGuestAccount = cairo_dock_dbus_detect_system_application ("org.gnome.DisplayManager");
	}
}

static gboolean _cd_logout_got_capabilities (CDSharedMemory *pSharedMemory)
{
	CD_APPLET_ENTER;
	
	// fetch the capabilities.
	myData.bCapabilitiesChecked = TRUE;
	myData.bCanHibernate = pSharedMemory->bCanHibernate;
	myData.bCanHybridSleep = pSharedMemory->bCanHybridSleep;
	myData.bCanSuspend = pSharedMemory->bCanSuspend;
	myData.bCanRestart = pSharedMemory->bCanRestart;
	myData.bCanStop = pSharedMemory->bCanStop;
	myData.bHasGuestAccount = pSharedMemory->bHasGuestAccount;
	myData.iLoginManager = pSharedMemory->iLoginManager;
	cd_debug ("capabilities: %d; %d; %d; %d; %d; %d", myData.bCanHibernate,
		myData.bCanHybridSleep, myData.bCanSuspend, myData.bCanRestart,
		myData.bCanStop, myData.bHasGuestAccount);
	
	// display the menu that has been asked beforehand.
	if (pSharedMemory->bShowMenu)
		_display_menu (pSharedMemory->pEvent);
	
	// sayonara task-san ^^
	gldi_task_discard (myData.pTask);
	myData.pTask = NULL;
	
	CD_APPLET_LEAVE (FALSE);
}

static void _free_memory (CDSharedMemory *pSharedMemory)
{
	if (pSharedMemory)
	{
		if (pSharedMemory->pEvent) gdk_event_free (pSharedMemory->pEvent);
		g_free (pSharedMemory);
	}
}

void cd_logout_check_capabilities (guint delay)
{
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	pSharedMemory->pEvent = gtk_get_current_event ();
	myData.pTask = gldi_task_new_full (0,
		(GldiGetDataAsyncFunc) _cd_logout_check_capabilities_async,
		(GldiUpdateSyncFunc) _cd_logout_got_capabilities,
		(GFreeFunc) _free_memory,
		pSharedMemory);
	if (delay) gldi_task_launch_delayed (myData.pTask, delay);
	else gldi_task_launch (myData.pTask);
}

void cd_logout_display_actions (void)
{
	if (! myData.bCapabilitiesChecked)
		cd_logout_check_capabilities (0);
	if (myData.pTask != NULL)
	{
		/**note: pSharedMemory will only be freed in the main thread after pTask
		 * has been set to NULL, so it is safe to access it. Also, bShowMenu and
		 * pEvent are only accessed in the main thread (in _cd_logout_got_capabilities ())
		 * so we can set them here without worry */
		CDSharedMemory *pSharedMemory = (CDSharedMemory*)myData.pTask->pSharedMemory;
		pSharedMemory->bShowMenu = TRUE;
		pSharedMemory->pEvent = gtk_get_current_event ();
		return;
	}
	_display_menu (NULL);
}


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

static gboolean _can_logoout ()
{
	// if the user set a custom command, assume that it will work
	if (myConfig.cUserAction != NULL) return TRUE;
	// original behavior: look for a session manager
	if (g_getenv ("SESSION_MANAGER") != NULL) return TRUE;
	// also check whether cairo_dock_fm_logout () would work
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME && (glib_major_version > 2 || glib_minor_version >= 16)) return TRUE;
	if (g_iDesktopEnv == CAIRO_DOCK_KDE) return TRUE;
	if (g_iDesktopEnv == CAIRO_DOCK_XFCE) return TRUE;
	
	const char *args[] = {"/bin/sh", "-c", NULL, NULL, NULL};
		
	// new additions in logout.sh
	// 1. Wayland specific
	if (g_getenv ("WAYLAND_DISPLAY") != NULL)
	{
		// check for wayland-logout and assume that it will work
		args[2] = "command -v wayland-logout";
		gchar *tmp = cairo_dock_launch_command_argv_sync_with_stderr (args, FALSE);
		if (tmp != NULL)
		{
			g_free (tmp);
			return TRUE;
		}
	}
	// 2. check if we are in a systemd graphical session
	args[2] = "command -v systemctl";
	gchar *tmp2 = cairo_dock_launch_command_argv_sync_with_stderr (args, FALSE);
	if (tmp2 != NULL)
	{
		g_free (tmp2);
		args[0] = "systemctl";
		args[1] = "--user";
		args[2] = "is-active";
		args[3] = "graphical-session.target";
		tmp2 = cairo_dock_launch_command_argv_sync_with_stderr (args, FALSE);
		if (tmp2 != NULL)
		{
			int r = strcmp(tmp2, "active");
			g_free (tmp2);
			if (!r) return TRUE;
		}
	}
	return FALSE;
}

static GtkWidget *_build_menu (GtkWidget **pShutdownMenuItem)
{
	GtkWidget *pMenu = gldi_menu_new (myIcon);
	
	GtkWidget *pMenuItem;

	gchar *cImagePath;
	cImagePath = cd_logout_check_icon ("system-shutdown", myData.iDesiredIconSize);
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Shut down"), cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/system-shutdown.svg", cd_logout_shut_down, pMenu);
	g_free (cImagePath);
	if (!myData.bCanStop && ! myConfig.cUserActionShutdown)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	*pShutdownMenuItem = pMenuItem;
	
	cImagePath = cd_logout_check_icon (GLDI_ICON_NAME_REFRESH, myData.iDesiredIconSize);
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Restart"), cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/system-restart.svg", cd_logout_restart, pMenu);
	g_free (cImagePath);
	if (!myData.bCanRestart)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	cImagePath = cd_logout_check_icon ("sleep", myData.iDesiredIconSize);
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Hibernate"), cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/system-hibernate.svg", cd_logout_hibernate, pMenu);
	gtk_widget_set_tooltip_text (pMenuItem, D_("Your computer will not consume any energy."));
	if (!myData.bCanHibernate)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	if (myData.bCanHybridSleep)
	{
		pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Hybrid Sleep"), cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/system-hibernate.svg", cd_logout_hybridSleep, pMenu);
		gtk_widget_set_tooltip_text (pMenuItem, D_("Your computer will still consume a small amount of energy but after some time, the computer will suspend to disk and not consume any energy."));
	}
	g_free (cImagePath);
	
	cImagePath = cd_logout_check_icon ("clock", myData.iDesiredIconSize);
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Suspend"), cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/system-suspend.svg", cd_logout_suspend, pMenu);
	g_free (cImagePath);
	gtk_widget_set_tooltip_text (pMenuItem, D_("Your computer will still consume a small amount of energy."));
	if (!myData.bCanSuspend)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	cImagePath = cd_logout_check_icon ("system-log-out", myData.iDesiredIconSize);
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Log out"), cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/system-log-out.svg", cd_logout_close_session, pMenu);
	g_free (cImagePath);
	gtk_widget_set_tooltip_text (pMenuItem, D_("Close your session and allow to open a new one."));
	if (!_can_logoout ())
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	if (myData.pUserList != NULL)  // refresh the users list (we could listen for the UserAdded,UserDeleted, UserChanged signals too).
	{
		g_list_foreach (myData.pUserList, (GFunc)_free_user, NULL);
		g_list_free (myData.pUserList);
	}
	myData.pUserList = cd_logout_get_users_list ();
	if (myData.pUserList != NULL && (myData.bHasGuestAccount || myData.pUserList->next != NULL))  // at least 2 users
	{
		GtkWidget *pUsersSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Switch user"), pMenu, GLDI_ICON_NAME_JUMP_TO);
		
		gboolean bFoundUser = FALSE;
		const gchar *cCurrentUser = g_getenv ("USER");
		CDUser *pUser;
		GList *u;
		for (u = myData.pUserList; u != NULL; u = u->next)
		{
			pUser = u->data;
			pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (
				(pUser->cRealName && *(pUser->cRealName) != '\0') ? pUser->cRealName : pUser->cUserName,
				pUser->cIconFile, _switch_to_user, pUsersSubMenu, pUser->cUserName);
			if (! bFoundUser && cCurrentUser && strcmp (cCurrentUser, pUser->cUserName) == 0)
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
	
	cImagePath = cd_logout_check_icon ("system-lock-screen", myData.iDesiredIconSize);
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Lock screen"), cImagePath ? cImagePath : MY_APPLET_SHARE_DATA_DIR"/locked.svg", cd_logoout_lock_screen, pMenu);  /// TODO: same question...
	g_free (cImagePath);
	if (myData.bCanStop)
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

static void _display_menu (const GdkEvent *pEvent)
{
	// build and show the menu
	GtkWidget *pShutdownMenuItem = NULL;
	GtkWidget *pMenu = _build_menu (&pShutdownMenuItem);
	CD_APPLET_POPUP_MENU_ON_MY_ICON_WITH_EVENT (pMenu, pEvent);
	
	// select the first (or last) item, which corresponds to the 'shutdown' action.
	gtk_menu_shell_select_item (GTK_MENU_SHELL (pMenu), pShutdownMenuItem);  // must be done here, after the menu has been realized.
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

static void _logind_action (const gchar *cAction)
{
	GError *error = NULL;
	DBusGProxy *pProxy = cairo_dock_create_new_system_proxy (
		"org.freedesktop.login1",
		"/org/freedesktop/login1",
		"org.freedesktop.login1.Manager");
	
	dbus_g_proxy_call (pProxy, cAction, &error,
		G_TYPE_BOOLEAN, FALSE,  // non-interactive
		G_TYPE_INVALID,
		G_TYPE_INVALID);
	if (error)
	{
		cd_warning ("Logind error: %s", error->message);
		gchar *cMessage = g_strdup_printf ("%s %s\n%s",
			D_("Logind has returned this error:"),
			error->message,
			D_("Please check that you can do this action\n"
				"(e.g. you can't power the computer off if another session is active)"));
		gldi_dialog_show_temporary_with_icon (cMessage, myIcon, myContainer,
			15e3, "same icon");
		g_free (cMessage);
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
static void _demand_confirmation (const gchar *cMessage, const gchar *cIconStock, const gchar *cIconImage, void (*callback) (void))
{
	gchar *cImagePath = cd_logout_check_icon (cIconStock, 32); // dialog
	myData.pConfirmationDialog = gldi_dialog_show (cMessage, myIcon, myContainer, 0, cImagePath ? cImagePath : cIconImage, NULL, (CairoDockActionOnAnswerFunc) _exec_action, callback, NULL);
	g_free (cImagePath);
}

static void _shut_down (void)
{
	gldi_object_notify (&myModuleObjectMgr, NOTIFICATION_LOGOUT);
	if (myData.bCanStop)
	{
		switch (myData.iLoginManager)
		{
			case CD_CONSOLE_KIT:
				_console_kit_action ("Stop");  // could use org.gnome.SessionManager.RequestShutdown, but it's not standard.
			break;
			case CD_LOGIND:
				_logind_action ("PowerOff");
			break;
			default:
			break;
		}
	}
	else if (myConfig.cUserActionShutdown)
	{
		cairo_dock_launch_command (myConfig.cUserActionShutdown);
	}
}
static inline gchar *_info_msg (void)
{
	gchar *cInfo = g_strdup_printf (D_("It will automatically shut-down in %ds"), myData.iCountDown);
	gchar *cMessage = g_strdup_printf ("%s\n\n (%s)", D_("Shut down the computer?"), cInfo);
	g_free (cInfo);
	if (! cd_logout_can_safety_shutdown ())
	{
		cInfo = g_strdup_printf ("%s\n\n%s", cMessage,
			D_("It seems your system is being updated!\n"
			   "Please be sure that you can shut down your computer right now."));
		g_free (cMessage);
		return cInfo;
	}
	return cMessage;
}
static gboolean _auto_shot_down (gpointer data)
{
	myData.iCountDown --;
	if (myData.iCountDown <= 0)
	{
		myData.iSidShutDown = 0;
		gldi_object_unref (GLDI_OBJECT(myData.pConfirmationDialog));
		myData.pConfirmationDialog = NULL;
		_shut_down ();
		return FALSE;
	}
	else
	{
		if (myData.pConfirmationDialog)  // paranoia
		{
			gchar *cMessage = _info_msg ();
			gldi_dialog_set_message (myData.pConfirmationDialog, cMessage);
			g_free (cMessage);
		}
		return TRUE;
	}
}
void cd_logout_shut_down_full (gboolean bDemandConfirmation)
{
	if (bDemandConfirmation)
	{
		myData.iCountDown = 60;
		gchar *cMessage = _info_msg ();
		_demand_confirmation (cMessage, "system-shutdown", MY_APPLET_SHARE_DATA_DIR"/system-shutdown.svg", _shut_down);
		g_free (cMessage);
		if (myData.iSidShutDown == 0)
		{
			myData.iSidShutDown = g_timeout_add_seconds (1, _auto_shot_down, NULL);
		}
	}
	else
	{
		_shut_down ();
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

static void _restart (void)
{
	gldi_object_notify (&myModuleObjectMgr, NOTIFICATION_LOGOUT);
	if (myData.bCanRestart)
	{
		switch (myData.iLoginManager)
		{
			case CD_CONSOLE_KIT:
				_console_kit_action ("Restart");  // could use org.gnome.SessionManager.RequestShutdown, but it's not standard.
			break;
			case CD_LOGIND:
				_logind_action ("Reboot");
				break;
			default:
				break;
		}
	}
	else if (myConfig.cUserActionShutdown)
	{
		cairo_dock_launch_command (myConfig.cUserActionShutdown);
	}
}

static void cd_logout_restart (void)
{
	if (myConfig.bConfirmAction)
	{
		_demand_confirmation (D_("Restart the computer?"), GLDI_ICON_NAME_REFRESH, MY_APPLET_SHARE_DATA_DIR"/system-restart.svg", _restart);
	}
	else
	{
		_restart ();
	}
}

static void cd_logout_suspend (void)
{
	if (myData.iLoginManager == CD_LOGIND)
		_logind_action ("Suspend");
	else
		_upower_action (TRUE);
}

static void cd_logout_hibernate (void)
{
	if (myData.iLoginManager == CD_LOGIND)
		_logind_action ("Hibernate");
	else
		_upower_action (FALSE);
}

static void cd_logout_hybridSleep (void)
{
	if (myData.iLoginManager == CD_LOGIND)
		_logind_action ("HybridSleep");
	else
		_upower_action (FALSE);
}

static void _logout (void)
{
	gldi_object_notify (&myModuleObjectMgr, NOTIFICATION_LOGOUT);
	if (myConfig.cUserAction != NULL)
		cairo_dock_launch_command (myConfig.cUserAction);
	else  // SwitchToGreeter will only show the greeter, we want to close the session
		cairo_dock_launch_command_single (MY_APPLET_SHARE_DATA_DIR"/logout.sh");
}

static void cd_logout_close_session (void)  // could use org.gnome.SessionManager.Logout
{
	/* Currently, cairo_dock_fm_logout displays to us a window from the DE
	 * to confirm if we want to close the session or not. So there is a
	 * confirmation box if cairo_dock_fm_logout returns TRUE.
	 * Now, if it returns FALSE, we will use logout.sh script and display
	 * a confirmation box if the user wants to have this dialogue.
	 */
	if (! cairo_dock_fm_logout ()) // it seems it's better to use tools from the DE for the logout action
	{
		if (myConfig.bConfirmAction)
		{
			_demand_confirmation (D_("Close the current session?"), "system-log-out", MY_APPLET_SHARE_DATA_DIR"/system-log-out.svg", _logout);  /// same question, see above...
		}
		else
		{
			_logout ();
		}
	}
	else
		gldi_object_notify (&myModuleObjectMgr, NOTIFICATION_LOGOUT);
}

static void cd_logout_switch_to_user (const gchar *cUser)
{
	const gchar *seat = g_getenv ("XDG_SEAT_PATH");
	GError *error = NULL;
	if (seat)  // else, we could possibly get it by: ck -> GetCurrentSession -> session -> GetSeatId
	{
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
			cd_warning ("DisplayManager (LocalDisplay) error: %s", error->message);
			g_error_free (error);
			if (myConfig.cUserActionSwitchUser != NULL)
				cairo_dock_launch_command_printf ("%s %s", NULL,
					myConfig.cUserActionSwitchUser, cUser);
		}
		g_object_unref (pProxy);
	}
	else // try with gdm
	{
		DBusGProxy *pProxy = cairo_dock_create_new_system_proxy (
			"org.gnome.DisplayManager",
			"/org/gnome/DisplayManager/LocalDisplayFactory",
			"org.gnome.DisplayManager.LocalDisplayFactory");
		dbus_g_proxy_call (pProxy, "SwitchToUser", &error,
			G_TYPE_STRING, cUser,
			G_TYPE_INVALID,
			G_TYPE_INVALID);  // we don't care the 'id' object path returned
		if (error)
		{
			cd_warning ("DisplayManager error: %s", error->message);
			g_error_free (error);
			if (myConfig.cUserActionSwitchUser != NULL)
				cairo_dock_launch_command_printf ("%s %s", NULL,
					myConfig.cUserActionSwitchUser, cUser);
		}
		g_object_unref (pProxy);
	}
}

static void cd_logout_switch_to_guest (void)
{
	const gchar *seat = g_getenv ("XDG_SEAT_PATH");
	GError *error = NULL;
	if (seat)  // else, we could possibly get it by: ck -> GetCurrentSession -> session -> GetSeatId
	{
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
			if (myConfig.cUserActionSwitchUser != NULL)
				cairo_dock_launch_command (myConfig.cUserActionSwitchUser);
		}
		g_object_unref (pProxy);
	}
	else // try with gdm
	{
		DBusGProxy *pProxy = cairo_dock_create_new_system_proxy (
			"org.gnome.DisplayManager",
			"/org/gnome/DisplayManager/LocalDisplayFactory",
			"org.gnome.DisplayManager.LocalDisplayFactory");
		dbus_g_proxy_call (pProxy, "StartGuestSession", &error,
			G_TYPE_STRING, "",  // current user session, but actually it can be NULL so why bother?
			G_TYPE_INVALID,
			G_TYPE_INVALID);  // we don't care the 'id' object path returned
		if (error)
		{
			cd_warning ("DisplayManager (LocalDisplay) error: %s", error->message);
			g_error_free (error);
			if (myConfig.cUserActionSwitchUser != NULL)
				cairo_dock_launch_command (myConfig.cUserActionSwitchUser);
		}
		g_object_unref (pProxy);
	}
}

static void cd_logoout_lock_screen (void)
{
	if (myConfig.cUserActionLock != NULL)
		cairo_dock_launch_command (myConfig.cUserActionLock);
	else
		cairo_dock_fm_lock_screen ();
}

/*
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
*/

  //////////////////
 /// USERS LIST ///
//////////////////

static void _free_user (CDUser *pUser)
{
	g_free (pUser->cUserName);
	g_free (pUser->cIconFile);
	g_free (pUser->cRealName);
	g_free (pUser);
}

static int _compare_user_name (CDUser *pUser1, CDUser *pUser2)
{
	return strcmp (pUser1->cUserName, pUser2->cUserName);
}

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

static GList* _get_users_list_gdm (void)
{
	GError *error = NULL;
	DBusGProxy *pProxy = cairo_dock_create_new_system_proxy ("org.gnome.DisplayManager",
		"/org/gnome/DisplayManager/UserManager",
		"org.gnome.DisplayManager.UserManager");
	GArray *users =  NULL;
	dbus_g_proxy_call (pProxy, "GetUserList", &error,
		G_TYPE_INVALID,
		dbus_g_type_get_collection ("GArray", G_TYPE_INT64), &users,
		G_TYPE_INVALID);
	if (error)
	{
		cd_warning ("Couldn't get users on the bus from org.gnome.DisplayManager (%s)\n-> Using a fallback method.", error->message);
		g_error_free (error);
		users =  NULL;
	}
	if (users == NULL)
		return _get_users_list_fallback ();
	
	GType g_type_ptrarray = dbus_g_type_get_collection ("GPtrArray",
		dbus_g_type_get_struct("GValueArray",
			G_TYPE_INT64,  // uid
			G_TYPE_STRING,  // user name
			G_TYPE_STRING,  // real name
			G_TYPE_STRING,  // shell
			G_TYPE_INT,  // login frequency
			G_TYPE_STRING,  // icon URL
			G_TYPE_INVALID));
	GPtrArray *info =  NULL;
	dbus_g_proxy_call (pProxy, "GetUsersInfo", &error,
		dbus_g_type_get_collection ("GArray", G_TYPE_INT64), users,
		G_TYPE_INVALID,
		g_type_ptrarray, &info,
		G_TYPE_INVALID);
	if (error)
	{
		cd_warning ("Couldn't get info on the bus from org.gnome.DisplayManager (%s)\n-> Using a fallback method.", error->message);
		g_error_free (error);
		info =  NULL;
	}
	if (info == NULL)
		return _get_users_list_fallback ();
	
	CDUser *pUser;
	GList *pUserList = NULL;
	GValueArray *va;
	GValue *v;
	guint i;
	for (i = 0; i < info->len; i ++)
	{
		va = info->pdata[i];
		if (! va)
			continue;
		
		pUser = g_new0 (CDUser, 1);
		v = g_value_array_get_nth (va, 1);  // GValueArray is deprecated from 2.32, yet it's so convenient to map the g_type_ptrarray type ...
		if (v && G_VALUE_HOLDS_STRING (v))
			pUser->cUserName = g_strdup (g_value_get_string (v));
		if (pUser->cUserName == NULL) // shouldn't happen
			continue;
		
		v = g_value_array_get_nth (va, 2);
		if (v && G_VALUE_HOLDS_STRING (v))
			pUser->cRealName = g_strdup (g_value_get_string (v));
		
		v = g_value_array_get_nth (va, 5);
		if (v && G_VALUE_HOLDS_STRING (v))
			pUser->cIconFile = g_strdup (g_value_get_string (v));
		
		pUserList = g_list_insert_sorted (pUserList, pUser, (GCompareFunc)_compare_user_name);
	}
	g_ptr_array_free (info, TRUE);
	g_array_free (users, TRUE);
	
	g_object_unref (pProxy);
	return pUserList;
}

static GList *cd_logout_get_users_list (void)
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
		cd_warning ("Couldn't get info on the bus from org.freedesktop.Accounts (%s)\n-> Trying from GnomeDisplayManager.", error->message);
		g_error_free (error);
		users =  NULL;
	}
	if (users == NULL)
		return _get_users_list_gdm ();
	
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
		pUser->cRealName = cairo_dock_dbus_get_property_as_string (pProxy, "org.freedesktop.Accounts.User", "RealName");
		pUserList = g_list_insert_sorted (pUserList, pUser, (GCompareFunc)_compare_user_name);
		
		g_object_unref (pProxy);
	}
	return pUserList;
}
