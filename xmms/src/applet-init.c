#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-infopipe.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("xmms", 1, 5, 4, CAIRO_DOCK_CATEGORY_CONTROLER)

static void _load_surfaces (void) {
	gchar *cUserImagePath;
	GString *sImagePath = g_string_new ("");
	//Chargement de l'image "default"
	if (myData.pSurface != NULL) {
		cairo_surface_destroy (myData.pSurface);
	}
	if (myConfig.cDefaultIcon != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cDefaultIcon);
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/xmms.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "stop"
	if (myData.pStopSurface != NULL) {
		cairo_surface_destroy (myData.pStopSurface);
	}
	if (myConfig.cStopIcon != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cStopIcon);
		myData.pStopSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/stop.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pStopSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "pause"
	if (myData.pPauseSurface != NULL) {
		cairo_surface_destroy (myData.pPauseSurface);
	}
	if (myConfig.cPauseIcon != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cPauseIcon);
		myData.pPauseSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/pause.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pPauseSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "play"
	if (myData.pPlaySurface != NULL) {
		cairo_surface_destroy (myData.pPlaySurface);
	}
	if (myConfig.cPlayIcon != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cPlayIcon);
		myData.pPlaySurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/play.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pPlaySurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "broken"
	if (myData.pBrokenSurface != NULL) {
		cairo_surface_destroy (myData.pBrokenSurface);
	}
	if (myConfig.cBrokenIcon != NULL) {
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cBrokenIcon);
		myData.pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else {
		g_string_printf (sImagePath, "%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	g_string_free (sImagePath, TRUE);
}

CD_APPLET_INIT_BEGIN (erreur)
  if (myDesklet != NULL) {
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	cd_remove_pipes();
	_load_surfaces();
	
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
	if (myDesklet != NULL) {
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	_load_surfaces();
	
	//\_______________ On relance avec la nouvelle config ou on redessine.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		myData.playingStatus = PLAYER_NONE;
		myData.previousPlayingStatus = -1;
		myData.previousPlayingTitle = NULL;
		myData.iPreviousTrackNumber = -1;
		myData.iPreviousCurrentTime = -1;
		// on ne fait rien, les modifs seront prises en compte au prochain coup de timer, dans au plus 1s.
	}
	else {  // on redessine juste l'icone.
		cd_xmms_draw_icon ();
	}
CD_APPLET_RELOAD_END
