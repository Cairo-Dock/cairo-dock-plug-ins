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

#define GUEST_SESSION_LAUNCHER "/usr/share/gdm/guest-session/guest-session-launch"

  ////////////////////////////
 /// SESSION-LESS ACTIONS ///
////////////////////////////

static void _display_dialog (void);
static void _display_menu (void);

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
	cd_debug ("capabilities: %d; %d; %d; %d", myData.bCanHibernate, myData.bCanSuspend, myData.bCanRestart, myData.bCanStop);
	
	// display the menu that has been asked beforehand.
	//_display_dialog ();
	_display_menu ();
	
	// sayonara the task.
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
		//_display_dialog ();
		_display_menu ();
	}
}


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
static void on_select_action (GtkButton *button, gpointer data)
{
	CDCommandsEnum iAction = GPOINTER_TO_INT (data);
	switch (iAction)
	{
		case CD_RESTART:
			if (myData.bCanRestart)
				_console_kit_action ("Restart");
			else if (myConfig.cUserAction)
				cairo_dock_launch_command (myConfig.cUserAction);
		break;
		case CD_STOP:
			if (myData.bCanStop)
				_console_kit_action ("Stop");
			else if (myConfig.cUserAction2)
				cairo_dock_launch_command (myConfig.cUserAction2);
		break;
		case CD_SUSPEND:
			_upower_action (TRUE);
		break;
		case CD_HIBERNATE:
			_upower_action (FALSE);
		break;
		case CD_LOG_OUT:
			cairo_dock_launch_command (MY_APPLET_SHARE_DATA_DIR"/logout.sh");
		break;
		default:  // can't happen
		break;
	}
}

/**static GtkWidget *_make_image (const gchar *cImage, int iSize)
{
	GtkWidget *pImage = NULL;
	if (strncmp (cImage, "gtk-", 4) == 0)
	{
		if (iSize >= 48)
			iSize = GTK_ICON_SIZE_DIALOG;
		else if (iSize >= 32)
			iSize = GTK_ICON_SIZE_LARGE_TOOLBAR;
		else
			iSize = GTK_ICON_SIZE_BUTTON;
		pImage = gtk_image_new_from_stock (cImage, iSize);
	}
	else
	{
		gchar *cIconPath = NULL;
		if (*cImage != '/')
		{
			cIconPath = cairo_dock_search_icon_s_path (cImage);
			if (cIconPath == NULL)
			{
				cIconPath = g_strconcat (MY_APPLET_SHARE_DATA_DIR"/", cImage, NULL);
			}
		}
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cIconPath ? cIconPath : cImage, iSize, iSize, NULL);
		g_free (cIconPath);
		if (pixbuf != NULL)
		{
			pImage = gtk_image_new_from_pixbuf (pixbuf);
			gdk_pixbuf_unref (pixbuf);
		}
	}
	return pImage;
}

static GtkWidget *_make_button (const gchar *cLabel, const gchar *cImage, CDCommandsEnum iAction)
{
	GtkWidget *pButton = gtk_button_new_with_label (cLabel);
	
	GtkWidget *pImage = _make_image (cImage, 32);
	if (pImage != NULL)
	{
		gtk_button_set_image (GTK_BUTTON (pButton), pImage);
	}
	
	g_signal_connect (G_OBJECT (pButton), "clicked", G_CALLBACK(on_select_action), GINT_TO_POINTER (iAction));
	
	cairo_dock_set_dialog_widget_bg_color (GTK_WIDGET (pButton));
	cairo_dock_set_dialog_widget_text_color (GTK_WIDGET (pButton));
	
	GdkColor color;
	color.red = myDialogsParam.dialogTextDescription.fColorStart[0] * 65535;
	color.green = myDialogsParam.dialogTextDescription.fColorStart[1] * 65535;
	color.blue = myDialogsParam.dialogTextDescription.fColorStart[2] * 65535;
	gtk_widget_modify_text (pButton, GTK_STATE_NORMAL, &color);
	
	return pButton;
}

static void _display_dialog (void)
{
	GtkWidget *pInteractiveWidget = gtk_table_new (2,
		3,
		TRUE);  // 2 lines, 3 columns
	GtkWidget *pButton;
	
	pButton = _make_button (D_("Shut down"), GTK_STOCK_QUIT, CD_STOP);  // system-shutdown
	gtk_table_attach_defaults (GTK_TABLE (pInteractiveWidget),
		pButton,
		0, 0+1,
		0, 0+1);
	gtk_widget_grab_focus (pButton);
	if (!myData.bCanStop)
		gtk_widget_set_sensitive (pButton, FALSE);
	
	pButton = _make_button (D_("Restart"), GTK_STOCK_REFRESH, CD_RESTART);  // system-restart
	gtk_table_attach_defaults (GTK_TABLE (pInteractiveWidget),
		pButton,
		1, 1+1,
		0, 0+1);
	if (!myData.bCanRestart)
		gtk_widget_set_sensitive (pButton, FALSE);
	
	pButton = _make_button (D_("Hibernate"), "/usr/share/icons/Humanity/actions/48/sleep.svg", CD_HIBERNATE);  // system-hibernate
	gtk_table_attach_defaults (GTK_TABLE (pInteractiveWidget),
		pButton,	
		0, 0+1,
		1, 1+1);
	if (!myData.bCanHibernate)
		gtk_widget_set_sensitive (pButton, FALSE);
	
	pButton = _make_button (D_("Suspend"), "/usr/share/icons/Humanity/actions/48/stock_media-pause.svg", CD_SUSPEND);  // system-suspend
	gtk_table_attach_defaults (GTK_TABLE (pInteractiveWidget),
		pButton,
		1, 1+1,
		1, 1+1);
	if (!myData.bCanSuspend)
		gtk_widget_set_sensitive (pButton, FALSE);

	pButton = _make_button (D_("Log out"), "/usr/share/icons/Humanity/actions/48/system-log-out.svg", 4);  // system-switch-user
	gtk_table_attach_defaults (GTK_TABLE (pInteractiveWidget),
		pButton,
		2, 2+1,
		1, 1+1);
	
	cairo_dock_show_dialog_full (D_("Log out"),
		myIcon,
		myContainer,
		0,
		"same icon",
		pInteractiveWidget,
		NULL,
		NULL,
		NULL);
}
*/

static GtkWidget *_build_menu (void)
{
	GtkWidget *pMenu = gtk_menu_new ();
	
	GtkWidget *pMenuItem;
	
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Shut down"), MY_APPLET_SHARE_DATA_DIR"/system-shutdown.svg", on_select_action, pMenu, GINT_TO_POINTER (CD_STOP));
	if (!myData.bCanStop && ! myConfig.cUserAction && ! myConfig.cUserAction2)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Restart"), MY_APPLET_SHARE_DATA_DIR"/system-restart.svg", on_select_action, pMenu, GINT_TO_POINTER (CD_RESTART));
	if (!myData.bCanRestart && ! myConfig.cUserAction && ! myConfig.cUserAction2)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Hibernate"), MY_APPLET_SHARE_DATA_DIR"/system-hibernate.svg", on_select_action, pMenu, GINT_TO_POINTER (CD_HIBERNATE));
	gtk_widget_set_tooltip_text (pMenuItem, D_("Your computer will not consume any energy."));
	if (!myData.bCanHibernate)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Suspend"), MY_APPLET_SHARE_DATA_DIR"/system-suspend.svg", on_select_action, pMenu, GINT_TO_POINTER (CD_SUSPEND));
	gtk_widget_set_tooltip_text (pMenuItem, D_("Your computer will still consume a small amount of energy."));
	if (!myData.bCanSuspend)
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	
	if (g_getenv ("SESSION_MANAGER") != NULL)  // needs a session manager for this.
	{
		pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Log out"), MY_APPLET_SHARE_DATA_DIR"/system-log-out.svg", on_select_action, pMenu, GINT_TO_POINTER (CD_LOG_OUT));
		gtk_widget_set_tooltip_text (pMenuItem, D_("Close your session and allow to open a new one."));
	}
	
	CD_APPLET_ADD_SEPARATOR_IN_MENU (pMenu);
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Lock screen"), MY_APPLET_SHARE_DATA_DIR"/locked.svg", cairo_dock_fm_lock_screen, pMenu);
	
	if (cd_logout_have_guest_session ())  // seems not very common yet, so we only add it if it exists.
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Guest session"), MY_APPLET_SHARE_DATA_DIR"/system-guest.svg", cd_logout_launch_guest_session, pMenu);
	}
	
	if (myData.bCanStop)
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Program an automatic shut-down"), MY_APPLET_SHARE_DATA_DIR"/icon-scheduling.svg", cd_logout_program_shutdown, pMenu);
	
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
			if (myConfig.cEmblemPath != NULL && *myConfig.cEmblemPath != '\0' && g_file_test (myConfig.cEmblemPath, G_FILE_TEST_EXISTS))
				CD_APPLET_SET_EMBLEM_ON_MY_ICON (myConfig.cEmblemPath, CAIRO_DOCK_EMBLEM_UPPER_RIGHT);
			else
				CD_APPLET_SET_EMBLEM_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/system-restart.svg", CAIRO_DOCK_EMBLEM_UPPER_RIGHT);
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


  /////////////////////
 /// GUEST SESSION ///
/////////////////////

gboolean cd_logout_have_guest_session (void)
{
	gboolean has = FALSE;
	if (g_getenv ("SESSION_MANAGER") == NULL)  // needs a session manager for this.
		return FALSE;
	if (g_file_test (GUEST_SESSION_LAUNCHER, G_FILE_TEST_EXISTS))
	{
		has = TRUE;
	}
	else
	{
		gchar *cResult = cairo_dock_launch_command_sync ("which guest-session");
		has = (cResult != NULL && *cResult == '/');
		g_free (cResult);
	}
	return has;
}

void cd_logout_launch_guest_session (void)
{
	gchar *cResult = cairo_dock_launch_command_sync ("which guest-session");
	if (cResult != NULL && *cResult == '/')
		cairo_dock_launch_command ("guest-session");
	else if (g_file_test (GUEST_SESSION_LAUNCHER, G_FILE_TEST_EXISTS))
		cairo_dock_launch_command (GUEST_SESSION_LAUNCHER);
	g_free (cResult);
}
