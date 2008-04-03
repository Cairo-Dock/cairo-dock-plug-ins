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
	///double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
	///myData.pGauge = init_cd_Gauge(myDrawContext,myConfig.cGThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
	
	cd_wifi_launch_measure();
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	
	//On libère la mémoire de la jauge
	///free_cd_Gauge(myData.pGauge);
	
	if (myData.iSidTimer != 0) {
		g_source_remove (myData.iSidTimer);
		myData.iSidTimer = 0;
	}
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
	
	//\_______________ On relance avec la nouvelle config ou on redessine.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		if (myData.iSidTimer != 0) { // la frequence a pu changer.
			g_source_remove (myData.iSidTimer);
			myData.iSidTimer = 0;
		}
		
		//On recharge la jauge
		cd_wifi_launch_measure ();  // asynchrone
	}
	else {  // on redessine juste l'icone.
		cd_wifi_set_surface (myData.iQuality);
	}
	
	/*free_cd_Gauge(myData.pGauge);
	double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
  myData.pGauge = init_cd_Gauge(myDrawContext,myConfig.cGThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);*/
CD_APPLET_RELOAD_END
