#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-wifi.h"
#include "applet-draw.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("wifi", 1, 6, 2, CAIRO_DOCK_CATEGORY_ACCESSORY);


CD_APPLET_INIT_BEGIN
	if (myDesklet != NULL)
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	
	//Initialisation de la jauge
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	if (myConfig.iDisplay == WIFI_GRAPHIC) {
		myData.pGraph = cairo_dock_create_graph (myDrawContext,
			20, myConfig.iGraphType,
			myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale,
			myConfig.fLowColor, myConfig.fHigholor, myConfig.fBgColor, NULL, NULL);
		if (myConfig.cWatermarkImagePath != NULL)
			cairo_dock_add_watermark_on_graph (myDrawContext, myData.pGraph, myConfig.cWatermarkImagePath, myConfig.fAlpha);
		CD_APPLET_RENDER_GRAPH (myData.pGraph);
	}
	else if (myConfig.iDisplay == WIFI_GAUGE) {
		myData.pGauge = cairo_dock_load_gauge (myDrawContext, myConfig.cGThemePath, myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
		if (myConfig.cWatermarkImagePath != NULL)
			cairo_dock_add_watermark_on_gauge (myDrawContext, myData.pGauge, myConfig.cWatermarkImagePath, myConfig.fAlpha);
		CD_APPLET_RENDER_GAUGE (myData.pGauge, 0.);
	}
	
	myData.iPreviousQuality = -1;  // force le dessin.
	myData.prev_prcnt = -1;
	myData.pMeasureTimer = cairo_dock_new_measure_timer (myConfig.iCheckInterval,
		cd_wifi_acquisition,
		cd_wifi_read_data,
		cd_wifi_update_from_data,
		myApplet);
	cairo_dock_launch_measure (myData.pMeasureTimer);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet != NULL)
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	
	
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	//\_______________ On relance avec la nouvelle config ou on redessine.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		int i; // reset surfaces utilisateurs.
		for (i = 0; i < WIFI_NB_QUALITY; i ++) {
			if (myData.pSurfaces[i] != NULL) {
				cairo_surface_destroy (myData.pSurfaces[i]);
				myData.pSurfaces[i] = NULL;
			}
		}

    if (myData.pGauge != NULL) {
		  cairo_dock_free_gauge (myData.pGauge);
		  myData.pGauge = NULL;
		}
		if (myData.pGraph != NULL) {
      myData.pGraph = NULL;
		  cairo_dock_free_graph (myData.pGraph);
		}
		
		if (myConfig.iDisplay == WIFI_GRAPHIC) {
			myData.pGauge = NULL;
			myData.pGraph = cairo_dock_create_graph (myDrawContext,
				20, myConfig.iGraphType,
				myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale,
				myConfig.fLowColor, myConfig.fHigholor, myConfig.fBgColor, NULL, NULL);
			if (myConfig.cWatermarkImagePath != NULL)
				cairo_dock_add_watermark_on_graph (myDrawContext, myData.pGraph, myConfig.cWatermarkImagePath, myConfig.fAlpha);
		}
		else if (myConfig.iDisplay == WIFI_GAUGE) {
			myData.pGraph = NULL;
			myData.pGauge = cairo_dock_load_gauge (myDrawContext,
				myConfig.cGThemePath,
				myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
			if (myConfig.cWatermarkImagePath != NULL)
				cairo_dock_add_watermark_on_gauge (myDrawContext, myData.pGauge, myConfig.cWatermarkImagePath, myConfig.fAlpha);
		}
		
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		myData.iPreviousQuality = -1;  // on force le redessin.
		myData.prev_prcnt = -1;
		cairo_dock_stop_measure_timer (myData.pMeasureTimer);  // on stoppe avant car  on ne veut pas attendre la prochaine iteration.
		cairo_dock_change_measure_frequency (myData.pMeasureTimer, myConfig.iCheckInterval);
		cairo_dock_launch_measure (myData.pMeasureTimer);  // mesure immediate.
	}
	else {  // on redessine juste l'icone.
		myData.iPreviousQuality = -1;  // force le redessin.
		if (myData.bAcquisitionOK) {
			cd_wifi_draw_icon ();
		}
		else {
			cd_wifi_draw_no_wireless_extension ();
		}
	}
CD_APPLET_RELOAD_END
