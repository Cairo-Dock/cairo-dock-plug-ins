#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-infopipe.h"

#define CD_XMMS_TIME_INTERVAL 1000

AppletConfig myConfig;
AppletData myData;

CD_APPLET_DEFINITION ("xmms", 1, 5, 3, CAIRO_DOCK_CATEGORY_CONTROLER)

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
		/*myIcon->fWidth = MAX (1, myDesklet->iWidth - g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius/2;
		myIcon->fDrawY = g_iDockRadius/2;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL; */
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	cd_remove_pipes();
	_load_surfaces();
	myData.lastQuickInfo = " ";
	myData.playingStatus = PLAYER_NONE;
	cd_xmms_update_title();
	myData.pipeTimer = g_timeout_add (CD_XMMS_TIME_INTERVAL, (GSourceFunc) cd_xmms_get_pipe, (gpointer) NULL);
	
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
	
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		cd_remove_pipes();
		cd_xmms_update_title();
	}
	else {
		cd_xmms_update_title();
	}
CD_APPLET_RELOAD_END
