
#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-wifi.h"

AppletConfig myConfig;
AppletData myData;


CD_APPLET_DEFINITION ("wifi", 1, 4, 7, CAIRO_DOCK_CATEGORY_ACCESSORY);

static void _load_surfaces (void) {
	gchar *cUserImagePath;
	GString *sImagePath = g_string_new ("");
	int i;
	
  //Chargement de l'image "default"
  i = WIFI_QUALITY_NO_SIGNAL;
  if (myData.pSurfaces[i] != NULL) {
		cairo_surface_destroy (myData.pSurfaces[i]);
	}
	if (myConfig.cDefault != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cDefault);
		myData.pSurfaces[i] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/default.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurfaces[i] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
  //Chargement de l'image "20"
  i = WIFI_QUALITY_VERY_LOW;
  if (myData.pSurfaces[i] != NULL) {
		cairo_surface_destroy (myData.pSurfaces[1]);
	}
	if (myConfig.c20Surface != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.c20Surface);
		myData.pSurfaces[i] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/link-20.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurfaces[i] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "40"
	i = WIFI_QUALITY_LOW;
	if (myData.pSurfaces[i] != NULL) {
		cairo_surface_destroy (myData.pSurfaces[i]);
	}
	if (myConfig.c40Surface != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.c40Surface);
		myData.pSurfaces[i] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/link-40.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurfaces[i] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "60"
	i = WIFI_QUALITY_MIDDLE;
	if (myData.pSurfaces[i] != NULL) {
		cairo_surface_destroy (myData.pSurfaces[i]);
	}
	if (myConfig.c60Surface != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.c60Surface);
		myData.pSurfaces[i] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/link-60.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurfaces[i] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "80"
	i = WIFI_QUALITY_GOOD;
	if (myData.pSurfaces[i] != NULL) {
		cairo_surface_destroy (myData.pSurfaces[i]);
	}
	if (myConfig.c80Surface != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.c80Surface);
		myData.pSurfaces[i] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/link-80.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurfaces[i] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	//Chargement de l'image "100"
	i = WIFI_QUALITY_EXCELLENT;
	if (myData.pSurfaces[i] != NULL) {
		cairo_surface_destroy (myData.pSurfaces[i]);
	}
	if (myConfig.c100Surface != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.c100Surface);
		myData.pSurfaces[i] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/link-100.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurfaces[i] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	g_string_free (sImagePath, TRUE);
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
	cd_wifi_launch_measure();
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	if (myData.iSidTimer != 0) {
		g_source_remove (myData.iSidTimer);
		myData.iSidTimer = 0;
	}
	
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
	
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		if (myData.iSidTimer != 0) {
			g_source_remove (myData.iSidTimer);
			myData.iSidTimer = 0;
		}
		
		cd_wifi_launch_measure ();  // asynchrone
	}
	else if (myDesklet != NULL) {
		cairo_surface_t *pSurface = myData.pSurfaces[myData.iPreviousQuality];
		if (pSurface != NULL) {
			CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
		}
	}
	else {
		// rien a faire, cairo-dock va recharger notre sous-dock.
	}
	
	//cd_wifi("AppletReload");
CD_APPLET_RELOAD_END
