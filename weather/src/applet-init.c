/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include "stdlib.h"

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"
#include "applet-init.h"

CD_APPLET_DEFINITION ("weather", 1, 5, 4, CAIRO_DOCK_CATEGORY_ACCESSORY)


CD_APPLET_INIT_BEGIN (erreur)
	if (myIcon->acName == NULL || *myIcon->acName == '\0')
		myIcon->acName = g_strdup (WEATHER_DEFAULT_NAME);
	
	myData.pMeasureTimer = cairo_dock_new_measure_timer (myConfig.iCheckInterval,
		cd_weather_acquisition,
		cd_weather_read_data,
		cd_weather_update_from_data);
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
		reset_data ();  // on bourrine.
		if (myIcon->acName == NULL || *myIcon->acName == '\0')
			myIcon->acName = g_strdup (WEATHER_DEFAULT_NAME);
		
		myData.pMeasureTimer = cairo_dock_new_measure_timer (myConfig.iCheckInterval,
			cd_weather_acquisition,
			cd_weather_read_data,
			cd_weather_update_from_data);
		cairo_dock_launch_measure (myData.pMeasureTimer);
	}
	else if (myDesklet != NULL)
	{
		gpointer pConfig[2] = {GINT_TO_POINTER (myConfig.bDesklet3D), GINT_TO_POINTER (FALSE)};
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, pConfig);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	else
	{
		// rien a faire, cairo-dock va recharger notre sous-dock.
	}
CD_APPLET_RELOAD_END
