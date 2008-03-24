
#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-netspeed.h"

AppletConfig myConfig;
AppletData myData;


CD_APPLET_DEFINITION ("netspeed", 1, 4, 7, CAIRO_DOCK_CATEGORY_ACCESSORY);

static void _load_surfaces (void) {
	gchar *cUserImagePath;
	GString *sImagePath = g_string_new ("");
	
	//Chargement des images
	if (myData.pDefault != NULL) {
		cairo_surface_destroy (myData.pDefault);
	}
	g_string_printf (sImagePath, "%s/default.png", MY_APPLET_SHARE_DATA_DIR);
	myData.pDefault = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	if (myData.pUnknown != NULL) {
		cairo_surface_destroy (myData.pUnknown);
	}
	g_string_printf (sImagePath, "%s/unknown.png", MY_APPLET_SHARE_DATA_DIR);
	myData.pUnknown = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	if (myData.pOk != NULL) {
		cairo_surface_destroy (myData.pOk);
	}
	g_string_printf (sImagePath, "%s/ok.png", MY_APPLET_SHARE_DATA_DIR);
	myData.pOk = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	if (myData.pBad != NULL) {
		cairo_surface_destroy (myData.pBad);
	}
	g_string_printf (sImagePath, "%s/bad.png", MY_APPLET_SHARE_DATA_DIR);
	myData.pBad = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
}

CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet != NULL) {
		myIcon->fWidth = MAX (1, myDesklet->iWidth - g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius/2;
		myIcon->fDrawY = g_iDockRadius/2;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;
	}
	_load_surfaces();
	cd_netspeed_wait("AppletINIT");
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	
	//\_________________ On libere toutes nos ressources.
	reset_config ();
	reset_data ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet != NULL) {
		myIcon->fWidth = MAX (1, myDesklet->iWidth - g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius/2;
		myIcon->fDrawY = g_iDockRadius/2;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;
		cd_message("WH : %d %d", myDesklet->iWidth, myDesklet->iHeight);
	}
	  
	_load_surfaces();
	cd_netspeed("AppletReload");
CD_APPLET_RELOAD_END
