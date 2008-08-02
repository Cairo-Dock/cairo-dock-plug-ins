/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"
#include "applet-init.h"

CD_APPLET_DEFINITION ("weather", 1, 6, 2, CAIRO_DOCK_CATEGORY_ACCESSORY)


CD_APPLET_INIT_BEGIN
	myData.pMeasureTimer = cairo_dock_new_measure_timer (myConfig.iCheckInterval,
		(CairoDockAquisitionTimerFunc) cd_weather_acquisition,
		(CairoDockReadTimerFunc) cd_weather_read_data,
		(CairoDockUpdateTimerFunc) cd_weather_update_from_data,
		myApplet);
	cairo_dock_launch_measure (myData.pMeasureTimer);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	g_return_val_if_fail (myConfig.cLocationCode != NULL, FALSE);
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		reset_data (myApplet);  // on bourrine.
		if (myIcon->acName == NULL || *myIcon->acName == '\0')
			myIcon->acName = g_strdup (WEATHER_DEFAULT_NAME);
		
		myData.pMeasureTimer = cairo_dock_new_measure_timer (myConfig.iCheckInterval,
			(CairoDockAquisitionTimerFunc) cd_weather_acquisition,
			(CairoDockReadTimerFunc) cd_weather_read_data,
			(CairoDockUpdateTimerFunc) cd_weather_update_from_data,
			myApplet);
		cairo_dock_launch_measure (myData.pMeasureTimer);
		g_print ("myDrawContext:%x\n", myDrawContext);
	}
	else if (myDesklet != NULL)
	{
		gpointer pConfig[2] = {GINT_TO_POINTER (myConfig.bDesklet3D), GINT_TO_POINTER (FALSE)};
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", pConfig);
	}
	else
	{
		// rien a faire, cairo-dock va recharger notre sous-dock.
	}
CD_APPLET_RELOAD_END
