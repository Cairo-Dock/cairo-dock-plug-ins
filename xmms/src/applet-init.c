#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-infopipe.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("xmms", 1, 5, 4, CAIRO_DOCK_CATEGORY_CONTROLER)


CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet) {
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	cd_remove_pipes();
	
	myData.playingStatus = PLAYER_NONE;
	myData.previousPlayingStatus = -1;
	myData.previousPlayingTitle = NULL;
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	cd_xmms_launch_measure ();
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	//\_________________ On libere toutes nos ressources.
	reset_data();
	reset_config();
	cd_remove_pipes();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet) {
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) { // reset surfaces.
		if (myData.pSurfaces[i] != NULL) {
			cairo_surface_destroy (myData.pSurfaces[i]);
			myData.pSurfaces[i] = NULL;
		}
	}
	
	//\_______________ On relance avec la nouvelle config ou on redessine.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		myData.playingStatus = PLAYER_NONE;
		myData.previousPlayingStatus = -1;
		myData.previousPlayingTitle = NULL;
		myData.iPreviousTrackNumber = -1;
		myData.iPreviousCurrentTime = -1;
		// inutile de relancer le timer, sa frequence ne change pas. Inutile aussi de faire 1 iteration ici, les modifs seront prises en compte a la prochaine iteration, dans au plus 1s.
	}
	else {  // on redessine juste l'icone.
		cd_xmms_draw_icon ();
	}
CD_APPLET_RELOAD_END
