/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

**********************************************************************************/
#include <stdlib.h>
#include <math.h>
#define __USE_POSIX
#include <signal.h>

#include "applet-struct.h"
#include "applet-draw.h"
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
	if (myData.pCalendarDialog != NULL)
	{
		cairo_dock_dialog_unreference (myData.pCalendarDialog);
		myData.pCalendarDialog = NULL;
	}
	else
	{
		GtkWidget *pCalendar = gtk_calendar_new ();
		myData.pCalendarDialog = cairo_dock_show_dialog_full (_("Calendar"),
			myIcon, myContainer,
			0,
			MY_APPLET_SHARE_DATA_DIR"/dates.svg",
			pCalendar,
			NULL, NULL, NULL);
	}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_IN_MENU (D_("Set up time and date"), _cd_clock_launch_time_admin, pSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myData.iAlarmPID > 0)
	{
		kill (myData.iAlarmPID, 1);
		myData.iAlarmPID = 0;
	}
	cairo_dock_remove_dialog_if_any (myIcon);
	myData.pCalendarDialog = NULL;
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
