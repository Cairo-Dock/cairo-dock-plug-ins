#include <stdlib.h>

#include "rhythmbox-draw.h"
#include "rhythmbox-config.h"
#include "rhythmbox-dbus.h"
#include "rhythmbox-menu-functions.h"
#include "rhythmbox-init.h"

cairo_surface_t *rhythmbox_pSurface = NULL;
cairo_surface_t *rhythmbox_pPlaySurface = NULL;
cairo_surface_t *rhythmbox_pPauseSurface = NULL;
cairo_surface_t *rhythmbox_pBrokenSurface = NULL;
cairo_surface_t *rhythmbox_pCover = NULL;

gchar *conf_defaultTitle = NULL;
gboolean rhythmbox_dbus_enable = FALSE;
gboolean rhythmbox_opening = FALSE;
gboolean rhythmbox_playing = FALSE;
gboolean cover_exist = FALSE;
int playing_duration = 0;
int playing_track = 0;
gchar *playing_uri = NULL;
const gchar *playing_artist = NULL;
const gchar *playing_album = NULL;
const gchar *playing_title = NULL;


CD_APPLET_PRE_INIT_BEGIN ("Rhythmbox", 1, 4, 6)
	rhythmbox_dbus_enable = rhythmbox_dbus_pre_init();
CD_APPLET_PRE_INIT_END


CD_APPLET_INIT_BEGIN (erreur)
	conf_defaultTitle = g_strdup (myIcon->acName);
	
	GString *sImagePath = g_string_new ("");  // ce serait bien de pouvoir choisir ses icones, comme dans l'applet logout...
	//Chargement de l'image "default"
	g_string_printf (sImagePath, "%s/stop.svg", MY_APPLET_SHARE_DATA_DIR);
	rhythmbox_pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	//Chargement de l'image "pause"
	g_string_printf (sImagePath, "%s/pause.svg", MY_APPLET_SHARE_DATA_DIR);
	rhythmbox_pPauseSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	//Chargement de l'image "play"
	g_string_printf (sImagePath, "%s/play.svg", MY_APPLET_SHARE_DATA_DIR);
	rhythmbox_pPlaySurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	//Chargement de l'image "broken"
	g_string_printf (sImagePath, "%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
	rhythmbox_pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	g_string_free (sImagePath, TRUE);
	
	if (rhythmbox_dbus_enable)
	{
		rhythmbox_dbus_init ();
		
		dbus_detect_rhythmbox();
		if(rhythmbox_opening)
		{
			rhythmbox_getPlaying();
			rhythmbox_getPlayingUri();
			getSongInfos();
			update_icon( FALSE );
		}
		else
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (rhythmbox_pSurface)
		}
	}
	else
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (rhythmbox_pBrokenSurface)
	}
	
	//Enregistrement des notifications	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	rhythmbox_dbus_stop ();
	
	g_free (conf_defaultTitle);
	conf_defaultTitle = NULL;
	
	cairo_surface_destroy (rhythmbox_pSurface);
	rhythmbox_pSurface = NULL;
	cairo_surface_destroy (rhythmbox_pPlaySurface);
	rhythmbox_pPlaySurface = NULL;
	cairo_surface_destroy (rhythmbox_pPauseSurface);
	rhythmbox_pPauseSurface = NULL;
	cairo_surface_destroy (rhythmbox_pCover);
	rhythmbox_pCover = NULL;
	cairo_surface_destroy (rhythmbox_pBrokenSurface);
	rhythmbox_pBrokenSurface = NULL;
	
	g_free (playing_uri);
	playing_uri = NULL;
	playing_artist = NULL;
	playing_album = NULL;
	playing_title = NULL;
	playing_duration = 0;
	playing_track = 0;
CD_APPLET_STOP_END
