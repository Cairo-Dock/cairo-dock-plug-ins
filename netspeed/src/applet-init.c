#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-netspeed.h"

extern int inDebug;

CD_APPLET_DEFINITION ("netspeed", 1, 5, 4, CAIRO_DOCK_CATEGORY_ACCESSORY);

/*static void _load_surfaces (void) {
	//Chargement des images
	if (myData.pDefault != NULL) {
		cairo_surface_destroy (myData.pDefault);
	}
	myData.pDefault = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (myConfig.cDefault);
	
	if (myData.pUnknown != NULL) {
		cairo_surface_destroy (myData.pUnknown);
	}
	myData.pUnknown = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (myConfig.cUnknown);
	
	if (myData.pOk != NULL) {
		cairo_surface_destroy (myData.pOk);
	}
	myData.pOk = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (myConfig.cOk);
	
	if (myData.pBad != NULL) {
		cairo_surface_destroy (myData.pBad);
	}
	myData.pBad = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (myConfig.cBad);
}*/

CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet != NULL) {
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	inDebug = 0;
							/*_load_surfaces();*/
	//Initialisation de la jauge
	double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
	myData.pGauge = init_cd_Gauge(myDrawContext,myConfig.cThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
 
	cd_netspeed_launch_analyse();
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	//Adieu la jauge...
	free_cd_Gauge(myData.pGauge);
	
/*	cairo_surface_destroy (myData.pDefault);
	cairo_surface_destroy (myData.pUnknown);
	cairo_surface_destroy (myData.pOk);
	cairo_surface_destroy (myData.pBad);
	myData.pDefault = NULL;
	myData.pUnknown = NULL;
	myData.pOk = NULL;
	myData.pBad = NULL; */
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
						/*_load_surfaces();*/
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		if (myData.iSidTimer != 0) { // la frequence a pu changer.
			g_source_remove (myData.iSidTimer);
			myData.iSidTimer = 0;
		}
	}
	else {
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
		/*CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pDefault);*/
	}	
		//On recharge la jauge
		free_cd_Gauge(myData.pGauge);
		double fMaxScale = (myDock != NULL ? 1 + g_fAmplitude : 1);
		myData.pGauge = init_cd_Gauge(myDrawContext,myConfig.cThemePath,myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
		// on relance l'analyse
		cd_netspeed_launch_analyse ();
CD_APPLET_RELOAD_END
