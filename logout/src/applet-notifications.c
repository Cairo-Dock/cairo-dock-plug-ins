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
#include <signal.h>
#include <math.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"


static void _logout (void)
{
	if (myConfig.cUserAction != NULL)
	{
		cairo_dock_launch_command (myConfig.cUserAction);
	}
	else
	{
		gboolean bLoggedOut = cairo_dock_fm_logout ();
		if (! bLoggedOut)
		{
			cd_warning ("couldn't guess what to do to log out, you may try to specify the command in the config.");
		}
	}
}
static void _shutdown (void)
{
	if (myConfig.cUserAction2 != NULL)
	{
		cairo_dock_launch_command (myConfig.cUserAction2);
	}
	else
	{
		gboolean bShutdowned = cairo_dock_fm_shutdown ();
		if (! bShutdowned)
		{
			cd_warning ("couldn't guess what to do to shutdown, you may try to specify the command in the config.");
		}
	}
}
static inline void _execute_action (gint iAction)
{
	switch (iAction)
	{
		case 0:
			_logout ();
		break;
		case 1:
			_shutdown ();
		break;
		case 2:
			
		break;
	}
}

CD_APPLET_ON_CLICK_BEGIN
{
	if (myIcon->Xid != 0)
	{
		if (cairo_dock_get_current_active_window () == myIcon->Xid && myTaskBar.bMinimizeOnClick)
			cairo_dock_minimize_xwindow (myIcon->Xid);
		else
			cairo_dock_show_xwindow (myIcon->Xid);
	}
	else
	{
		_execute_action (myConfig.iActionOnClick);
	}
}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
{
	_execute_action (myConfig.iActionOnMiddleClick);
}
CD_APPLET_ON_MIDDLE_CLICK_END


static void _cd_logout (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	_logout ();
	CD_APPLET_LEAVE ();
}
static void _cd_shutdown (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	_shutdown ();
	CD_APPLET_LEAVE ();
}
static void _cd_lock_screen (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	cairo_dock_fm_lock_screen ();
	CD_APPLET_LEAVE ();
}

static void _cd_logout_program_shutdown (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	int iDeltaT = (int) (cairo_dock_show_value_and_wait (D_("Choose in how many minutes your PC will stop:"), myIcon, myContainer, 30, 150) * 60);
	if (iDeltaT == -1)  // cancel
		CD_APPLET_LEAVE ();
		//return ;
	
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
	CD_APPLET_LEAVE ();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
{
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	
	gchar *cLabel;
	if (myConfig.iActionOnClick != 0)  // logout action not on click => put it in the menu
	{
		if (myConfig.iActionOnMiddleClick == 0)  // logout action on middle-click
			cLabel = g_strdup_printf ("%s (%s)", D_("Log out"), D_("middle-click"));
		else
			cLabel = g_strdup (D_("Log out"));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE, _cd_logout, CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
	if (myConfig.iActionOnClick != 1)  // shutdown action not on click => put it in the menu
	{
		if (myConfig.iActionOnMiddleClick == 1)  // logout action on middle-click
			cLabel = g_strdup_printf ("%s (%s)", D_("Shut down"), D_("middle-click"));
		else
			cLabel = g_strdup (D_("Shut down"));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE, _cd_shutdown, CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
	if (myConfig.iActionOnClick != 2)  // lockscreen action not on click => put it in the menu
	{
		if (myConfig.iActionOnMiddleClick == 2)  // lockscreen action on middle-click
			cLabel = g_strdup_printf ("%s (%s)", D_("Lock screen"), D_("middle-click"));
		else
			cLabel = g_strdup (D_("Lock screen"));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, MY_APPLET_SHARE_DATA_DIR"/icon-lock.png", _cd_lock_screen, CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Program an automatic shut-down"), MY_APPLET_SHARE_DATA_DIR"/icon-scheduling.png", _cd_logout_program_shutdown, CD_APPLET_MY_MENU);  // pas beaucoup d'entrees => on le met dans le menu global.
	
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
}
CD_APPLET_ON_BUILD_MENU_END



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
			///cairo_dock_launch_command ("dbus-send --session --type=method_call --dest=org.freedesktop.PowerManagement /org/freedesktop/PowerManagement org.freedesktop.PowerManagement.Shutdown");  // --print-reply --reply-timeout=2000
			cairo_dock_launch_command ("dbus-send --system --print-reply --dest=org.freedesktop.ConsoleKit /org/freedesktop/ConsoleKit/Manager org.freedesktop.ConsoleKit.Manager.Stop");  // Suspend est aussi possible
		
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
	//return TRUE;
	
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
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultLabel);
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
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cDefaultLabel);
			CD_APPLET_STOP_DEMANDING_ATTENTION;
		break;
		
		case CAIRO_DOCK_FILE_CREATED:  // reboot required
			myData.bRebootNeeded = TRUE;
			_set_reboot_message ();
			CD_APPLET_DEMANDS_ATTENTION ("pulse", 20);
			cairo_dock_show_temporary_dialog_with_icon (myIcon->cName, myIcon, myContainer, 5e3, "same icon");
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
