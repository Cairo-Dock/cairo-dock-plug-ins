/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-silder.h"


CD_APPLET_DEFINITION ("slider", 1, 5, 4, CAIRO_DOCK_CATEGORY_ACCESSORY)


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	//myConfig.pAnimation = SLIDER_DEFAULT;
	//myConfig.pAnimation = SLIDER_FADE;
	cd_slider_get_files_from_dir();  /// suggestion : le threader car ca prend du temps de parcourir le disque.
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	//Il faut couper le dernier affichage?
	g_source_remove(myData.iTimerID);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	
	//On arrete tout!
	myData.bPause = TRUE;
	g_source_remove(myData.iTimerID);
	myData.iTimerID = 0;
	cd_slider_get_files_from_dir(); //on recharge les images
	
	if (myDesklet) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED) {

	}
	else {
		//Rien a faire ^^
	}
	
	myData.bPause = FALSE;
	cd_slider_draw_images(); //on relance le diapo
CD_APPLET_RELOAD_END
