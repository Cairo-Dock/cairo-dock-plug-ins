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
			if (g_iDesktopEnv == CAIRO_DOCK_KDE)
			{
				int answer = cairo_dock_ask_question_and_wait ("Log out ?", myIcon, myContainer);
				if (answer == GTK_RESPONSE_YES)
				{
					int r = system ("dcop ksmserver default logout 0 0 0");  // kdmctl shutdown reboot forcenow  // kdeinit_shutdown
					r = system ("qdbus org.kde.ksmserver /KSMServer logout 0 3 0");
				}
			}
			else
			{
				cd_warning ("couldn't guess what to do to log out.");
			}
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
			if (g_iDesktopEnv == CAIRO_DOCK_KDE)
			{
				int answer = cairo_dock_ask_question_and_wait ("Shutdown ?", myIcon, myContainer);
				if (answer == GTK_RESPONSE_YES)
				{
					int r = system ("dcop ksmserver default logout 0 0 0");  // kdmctl shutdown reboot forcenow  // kdeinit_shutdown
					r = system ("qdbus org.kde.ksmserver /KSMServer logout 0 2 0");
				}
			}
			else
			{
				cd_warning ("couldn't guess what to do to shutdown.");
			}
		}
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
		if (myConfig.bInvertButtons)
			_shutdown ();
		else
			_logout ();
	}
}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
{
	if (myConfig.bInvertButtons)
		_logout ();
	else
		_shutdown ();
}
CD_APPLET_ON_MIDDLE_CLICK_END



static void _cd_logout_program_shutdown (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	int iDeltaT = (int) (cairo_dock_show_value_and_wait (D_("Choose in how many minutes your PC will stop :"), myIcon, myContainer, 30, 150) * 60);
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
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Program an automatic shutdown"), MY_APPLET_SHARE_DATA_DIR"/icon-scheduling.png", _cd_logout_program_shutdown, pSubMenu);
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
}
CD_APPLET_ON_BUILD_MENU_END



static gboolean _timer (gpointer data)
{
	CD_APPLET_ENTER;
	time_t t_cur = (time_t) time (NULL);
	if (t_cur >= myConfig.iShutdownTime)
	{
		g_print ("shutdown !\n");
		
		if (g_iDesktopEnv == CAIRO_DOCK_KDE)
			cairo_dock_launch_command ("dbus-send --session --type=method_call --dest=org.kde.ksmserver /KSMServer org.kde.KSMServerInterface.logout int32:0 int32:2 int32:2");
		else
			cairo_dock_launch_command ("dbus-send --session --type=method_call --dest=org.freedesktop.PowerManagement /org/freedesktop/PowerManagement org.freedesktop.PowerManagement.Shutdown");  // --print-reply --reply-timeout=2000
		
		myData.iSidTimer = 0;
		CD_APPLET_LEAVE (FALSE);  // inutile de faire quoique ce soit d'autre, puisque l'ordi s'eteint.
		//return FALSE;  // inutile de faire quoique ce soit d'autre, puisque l'ordi s'eteint.
	}
	else
	{
		g_print ("shutdown in %d minutes\n", (int) (myConfig.iShutdownTime - t_cur) / 60);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%dmn", (int) ceil ((double)(myConfig.iShutdownTime - t_cur) / 60.));
		CD_APPLET_REDRAW_MY_ICON;
		if (t_cur >= myConfig.iShutdownTime - 60)
			cairo_dock_show_temporary_dialog_with_icon (D_("Your computer will shutdown in 1 minute."), myIcon, myContainer, 8000, "same icon");
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
