#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-wifi.h"
#include "applet-draw.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("wifi", 1, 5, 4, CAIRO_DOCK_CATEGORY_ACCESSORY);


CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet != NULL) {
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	//Initialisation de la jauge
	double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
	myData.pGauge = init_cd_Gauge(myDrawContext,myConfig.cGThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
	
	myData.iPreviousQuality = -1;  // force le dessin.
	myData.prev_prcnt = -1;
	myData.pMeasureTimer = cairo_dock_new_measure_timer (myConfig.iCheckInterval,
		cd_wifi_acquisition,
		cd_wifi_read_data,
		cd_wifi_update_from_data);
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
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	int i;
	for (i = 0; i < WIFI_NB_QUALITY; i ++) { // reset surfaces.
		if (myData.pSurfaces[i] != NULL) {
			cairo_surface_destroy (myData.pSurfaces[i]);
			myData.pSurfaces[i] = NULL;
		}
	}
	if (myData.pGauge != NULL) {
		free_cd_Gauge(myData.pGauge);
		myData.pGauge = NULL;
	}
	
	//\_______________ On relance avec la nouvelle config ou on redessine.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		cairo_dock_stop_measure_timer (myData.pMeasureTimer);
		
		if (myConfig.bUseGauge)  // on ne veut plus des jauges.
		{
			double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
			myData.pGauge = init_cd_Gauge(myDrawContext,myConfig.cGThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
		}
		
		myData.iFrequency = WIFI_FREQUENCY_NORMAL;
		myData.iPreviousQuality = -1;  // on force le redessin.
		myData.prev_prcnt = -1;
		cairo_dock_launch_measure (myData.pMeasureTimer);  // mesure immediate.
	}
	else {  // on redessine juste l'icone.
		cd_wifi_draw_icon_with_effect (myData.iQuality);
	}
CD_APPLET_RELOAD_END
