/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <stdlib.h>

#include "applet-struct.h"
#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


void cd_clock_launch_time_admin (GtkMenuItem *menu_item, gpointer *data)
{
	GError *erreur = NULL;
	if (myConfig.cSetupTimeCommand != NULL)
	{
		g_spawn_command_line_async (myConfig.cSetupTimeCommand, &erreur);
	}
	else if (g_iDesktopEnv == CAIRO_DOCK_GNOME)
	{
		g_spawn_command_line_async ("gksu time-admin", &erreur);
	}
	else if (g_iDesktopEnv == CAIRO_DOCK_KDE)
	{
		g_spawn_command_line_async ("kcmshell kde-clock.desktop", &erreur);
	}
	
	if (erreur != NULL)
	{
		cd_message ("Attention : when trying to execute '%s' : %s\n", myConfig.cSetupTimeCommand, erreur->message);
		g_error_free (erreur);
	}
}


CD_APPLET_ABOUT (_D("This is the Cairo-Dock's clock applet\n made by Fabrice Rey (fabounet_03@yahoo.fr) for Cairo-Dock.\nThe analogic representation is a port of the well-known Cairo-Clock\n from MacSlow (http://macslow.thepimp.net)."))


CD_APPLET_ON_CLICK_BEGIN
	cairo_dock_remove_dialog_if_any (myIcon);
	GtkWidget *pCalendar = gtk_calendar_new ();
	cairo_dock_show_dialog_full (_("Calendar"), myIcon, myContainer, 0, NULL, GTK_BUTTONS_NONE, pCalendar, NULL, NULL, NULL);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Clock", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU (_D("Set up time and date"), cd_clock_launch_time_admin, pSubMenu)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myData.iAlarmPID > 0)
	{
		kill (myData.iAlarmPID, 1);
		myData.iAlarmPID = 0;
	}
	cairo_dock_remove_dialog_if_any (myIcon);
CD_APPLET_ON_MIDDLE_CLICK_END
