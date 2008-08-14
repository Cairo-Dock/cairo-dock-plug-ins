#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-netspeed.h"


CD_APPLET_DEFINITION ("netspeed", 1, 6, 2, CAIRO_DOCK_CATEGORY_ACCESSORY);


CD_APPLET_INIT_BEGIN
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	//Initialisation de la jauge
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	myData.pGauge = cairo_dock_load_gauge(myDrawContext,myConfig.cGThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
	if (myConfig.cWatermarkImagePath != NULL)
		cairo_dock_add_watermark_on_gauge (myDrawContext, myData.pGauge, myConfig.cWatermarkImagePath, myConfig.fAlpha);
	
	//Initialisation du timer de mesure.
	myData.pClock = g_timer_new ();
	myData.pMeasureTimer = cairo_dock_new_measure_timer (myConfig.iCheckInterval,
		NULL,
		(CairoDockReadTimerFunc) cd_netspeed_read_data,
		(CairoDockUpdateTimerFunc) cd_netspeed_update_from_data,
		myApplet);
	myData.bAcquisitionOK = TRUE;
	cairo_dock_launch_measure (myData.pMeasureTimer);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		cairo_dock_free_gauge(myData.pGauge);
		myData.pGauge = cairo_dock_load_gauge(myDrawContext,myConfig.cGThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
		if (myConfig.cWatermarkImagePath != NULL)
			cairo_dock_add_watermark_on_gauge (myDrawContext, myData.pGauge, myConfig.cWatermarkImagePath, myConfig.fAlpha);
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
		cairo_dock_reload_gauge (myDrawContext, myData.pGauge, myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
		if (myConfig.cWatermarkImagePath != NULL)
			cairo_dock_add_watermark_on_gauge (myDrawContext, myData.pGauge, myConfig.cWatermarkImagePath, myConfig.fAlpha);
		
		cd_netspeed_update_from_data (myApplet);
	}
CD_APPLET_RELOAD_END
