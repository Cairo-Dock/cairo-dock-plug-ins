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
#include <math.h>
#define __USE_POSIX
#include <signal.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-calendar.h"
#include "applet-notifications.h"


static void _cd_clock_launch_time_admin (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	if (myConfig.cSetupTimeCommand != NULL)
	{
		cairo_dock_launch_command (myConfig.cSetupTimeCommand);
	}
	else
	{
		if (! cairo_dock_fm_setup_time ())
		{
			if (g_iDesktopEnv == CAIRO_DOCK_KDE)
			{
				cairo_dock_launch_command ("kcmshell kde-clock.desktop");
			}
			else
			{
				cd_warning ("couldn't guess what to do to set up time.");
			}
		}
	}
}


CD_APPLET_ON_CLICK_BEGIN
	cd_clock_show_hide_calendar (myApplet);
CD_APPLET_ON_CLICK_END

static void _cd_clock_show_tasks_today (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	gchar *cTasks = cd_clock_get_tasks_for_today (myApplet);
	if (cTasks == NULL)
		cTasks = g_strdup (D_("No task is sheduled for today.\n\nYou can add tasks by clicking on the applet to open the calendar, and then double-clicking on a day."));
	
	cd_clock_hide_dialogs (myApplet);
	myDialogsParam.dialogTextDescription.bUseMarkup = TRUE;
	cairo_dock_show_temporary_dialog_with_icon (cTasks, myIcon, myContainer, 30e3, MY_APPLET_SHARE_DATA_DIR"/icon-task.png");
	myDialogsParam.dialogTextDescription.bUseMarkup = TRUE;
	
	g_free (cTasks);
}
static void _cd_clock_show_tasks_week (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	gchar *cTasks = cd_clock_get_tasks_for_this_week (myApplet);
	double fDelay = 30e3;
	if (cTasks == NULL)
	{
		cTasks = g_strdup (D_("No task is sheduled for the next 7 days.\n\nYou can add tasks by clicking on the applet to open the calendar, and then double-clicking on a day."));
		fDelay = 4e3;
	}
	
	cd_clock_hide_dialogs (myApplet);
	myDialogsParam.dialogTextDescription.bUseMarkup = TRUE;
	cairo_dock_show_temporary_dialog_with_icon (cTasks, myIcon, myContainer, fDelay, MY_APPLET_SHARE_DATA_DIR"/icon-task.png");
	myDialogsParam.dialogTextDescription.bUseMarkup = TRUE;
	
	g_free (cTasks);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	
	// Main Menu
	CD_APPLET_ADD_IN_MENU (D_("Show today's tasks"), _cd_clock_show_tasks_today, CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU (D_("Show this week's tasks"), _cd_clock_show_tasks_week, CD_APPLET_MY_MENU);
	
	// Sub-Menu
	if (cairo_dock_fm_can_setup_time ())
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Set up time and date"), GTK_STOCK_PREFERENCES, _cd_clock_launch_time_admin, pSubMenu);
		CD_APPLET_ADD_SEPARATOR_IN_MENU (pSubMenu);
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myData.iAlarmPID > 0)
	{
		kill (myData.iAlarmPID, 1);
		myData.iAlarmPID = 0;
	}
	cd_clock_hide_dialogs (myApplet);
	CD_APPLET_STOP_DEMANDING_ATTENTION;
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_UPDATE_ICON_BEGIN
	myData.iSmoothAnimationStep ++;
	int iDetlaT = cairo_dock_get_slow_animation_delta_t (myContainer);
	int iNbSteps = 1.*myConfig.iSmoothAnimationDuration / iDetlaT;  // on anime l'aiguille sur 500ms.
	if (myData.iSmoothAnimationStep > iNbSteps)
		CD_APPLET_SKIP_UPDATE_ICON;
	
	// taille de la texture.
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	// render to texture
	cd_clock_render_analogic_to_texture (myApplet, iWidth, iHeight, &myData.currentTime, 1.*myData.iSmoothAnimationStep / iNbSteps);
CD_APPLET_ON_UPDATE_ICON_END
