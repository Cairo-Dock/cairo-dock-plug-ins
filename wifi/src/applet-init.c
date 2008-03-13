
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
		g_string_printf (sImagePath, "%s/default.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pDefault = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	//Chargement de l'image "20"
		g_string_printf (sImagePath, "%s/link-20.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.p2Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	//Chargement de l'image "40"
		g_string_printf (sImagePath, "%s/link-40.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.p4Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	//Chargement de l'image "60"
		g_string_printf (sImagePath, "%s/link-60.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.p6Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	//Chargement de l'image "80"
		g_string_printf (sImagePath, "%s/link-80.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.p8Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
		
	//Chargement de l'image "100"
		g_string_printf (sImagePath, "%s/link-100.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.p1Surface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
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
		myDesklet->renderer = cd_get_strength;
	}
	_load_surfaces();
	cd_wifi(myIcon);
  myData.checkTimer = g_timeout_add (10000, (GSourceFunc) cd_wifi, (gpointer) myIcon);
  
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
	  cd_wifi(myIcon);
CD_APPLET_RELOAD_END
