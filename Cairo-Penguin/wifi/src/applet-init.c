
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
	
  //Chargement de l'image "default"
  if (myData.pDefault != NULL) {
		cairo_surface_destroy (myData.pDefault);
	}
	if (myConfig.cDefault != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cDefault);
		myData.pDefault = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/default.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pDefault = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
  //Chargement de l'image "20"
  if (myData.p2Surface != NULL) {
		cairo_surface_destroy (myData.p2Surface);
	}
	if (myConfig.c2Surface != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.c2Surface);
		myData.p2Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/link-20.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.p2Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "40"
	if (myData.p4Surface != NULL) {
		cairo_surface_destroy (myData.p4Surface);
	}
	if (myConfig.c4Surface != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.c4Surface);
		myData.p4Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/link-40.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.p4Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "60"
	if (myData.p6Surface != NULL) {
		cairo_surface_destroy (myData.p6Surface);
	}
	if (myConfig.c6Surface != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.c6Surface);
		myData.p6Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/link-60.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.p6Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "80"
	if (myData.p8Surface != NULL) {
		cairo_surface_destroy (myData.p8Surface);
	}
	if (myConfig.c8Surface != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.c8Surface);
		myData.p8Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/link-80.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.p8Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	//Chargement de l'image "100"
	if (myData.p1Surface != NULL) {
		cairo_surface_destroy (myData.p1Surface);
	}
	if (myConfig.c1Surface != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.c1Surface);
		myData.p1Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/link-100.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.p1Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
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
  cd_wifi_wait("AppletINIT");
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
    cd_wifi("AppletReload");
CD_APPLET_RELOAD_END
