#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-cpusage.h"


CD_APPLET_DEFINITION ("cpusage", 1, 5, 4, CAIRO_DOCK_CATEGORY_ACCESSORY);

CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet != NULL) {
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	//Initialisation de la jauge
	double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
	myData.pGauge = init_cd_Gauge(myDrawContext,myConfig.cThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
 	make_cd_Gauge(myDrawContext,myDock,myIcon,myData.pGauge,0);
	
	myData.pClock = g_timer_new ();
	myData.pMeasureTimer = cairo_dock_new_measure_timer (myConfig.iCheckInterval,
		NULL,
		cd_cpusage_read_data,
		cd_cpusage_update_from_data);
	cairo_dock_launch_measure (myData.pMeasureTimer);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet != NULL) {
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	//On recharge la jauge
	free_cd_Gauge(myData.pGauge);
	double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
	myData.pGauge = init_cd_Gauge(myDrawContext,myConfig.cThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
	
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_ICON)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL)
		}
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_LABEL)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle)
		}
		
		cairo_dock_relaunch_measure_immediately (myData.pMeasureTimer, myConfig.iCheckInterval);
	}
	else {  // on redessine juste l'icone.
		cd_cpusage_update_from_data ();
	}
CD_APPLET_RELOAD_END
