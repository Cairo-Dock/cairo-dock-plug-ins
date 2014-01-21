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

#include <math.h>

#include "applet-struct.h"
#include "applet-timer.h"

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
		cd_debug ("shutdown in %d minutes", (int) (myConfig.iShutdownTime - t_cur) / 60);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%dmn", (int) ceil ((double)(myConfig.iShutdownTime - t_cur) / 60.));
		CD_APPLET_REDRAW_MY_ICON;
		if (t_cur >= myConfig.iShutdownTime - 60)
			gldi_dialog_show_temporary_with_icon (D_("Your computer will shut-down in 1 minute."), myIcon, myContainer, 8000, "same icon");
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

static void _on_program_shutdown (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	CD_APPLET_ENTER;
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		int iDeltaT = 60 * gtk_range_get_value (GTK_RANGE (pInteractiveWidget));
		if (iDeltaT > 0)  // set the new time
		{
			//g_print ("iShutdownTime <- %ld + %d\n", t_cur, iDeltaT);
			time_t t_cur = (time_t) time (NULL);
			myConfig.iShutdownTime = (int) (t_cur + iDeltaT);
		}
		else if (iDeltaT == 0)  // cancel any previous shutdown 
		{
			myConfig.iShutdownTime = 0;
		}
		cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
			G_TYPE_INT, "Configuration", "shutdown time", myConfig.iShutdownTime,
			G_TYPE_INVALID);
		cd_logout_set_timer ();
	}
	CD_APPLET_LEAVE ();
}
void cd_logout_program_shutdown (void)
{
	gldi_dialog_show_with_value (D_("Choose in how many minutes your PC will stop:"),
		myIcon, myContainer,
		"same icon",
		30, 150,
		(CairoDockActionOnAnswerFunc) _on_program_shutdown, NULL, (GFreeFunc)NULL);
}