/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include "stdlib.h"
#include "string.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-bookmarks.h"
#include "applet-load-icons.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("shortcuts", 1, 6, 2, CAIRO_DOCK_CATEGORY_DESKTOP)


CD_APPLET_INIT_BEGIN
	if (myIcon->acName == NULL || *myIcon->acName == '\0')
		myIcon->acName = g_strdup (SHORTCUTS_DEFAULT_NAME);
	
	//\_______________ On charge les icones dans un sous-dock.
	//cd_shortcuts_launch_measure ();  // asynchrone
	myData.pMeasureTimer = cairo_dock_new_measure_timer (0,
		NULL,
		(CairoDockReadTimerFunc) cd_shortcuts_get_shortcuts_data,
		(CairoDockUpdateTimerFunc) cd_shortcuts_build_shortcuts_from_data,
		myApplet);
	cairo_dock_launch_measure (myData.pMeasureTimer);
	
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On charge les icones dans un sous-dock.
		cd_shortcuts_reset_all_datas (myApplet);
		
		if (myIcon->acName == NULL || *myIcon->acName == '\0')
			myIcon->acName = g_strdup (SHORTCUTS_DEFAULT_NAME);
		
		//cd_shortcuts_launch_measure ();  // asynchrone
		myData.pMeasureTimer = cairo_dock_new_measure_timer (0,
			NULL,
			(CairoDockReadTimerFunc) cd_shortcuts_get_shortcuts_data,
	                (CairoDockUpdateTimerFunc) cd_shortcuts_build_shortcuts_from_data,
	                myApplet);
		cairo_dock_launch_measure (myData.pMeasureTimer);
	}
	else if (myDesklet)
	{
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);  // on n'a pas besoin du context sur myIcon.
	}
	else
	{
		// rien a faire, cairo-dock va recharger notre sous-dock.
	}
CD_APPLET_RELOAD_END
