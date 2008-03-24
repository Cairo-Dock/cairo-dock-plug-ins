#include <stdlib.h>

#include "rhythmbox-draw.h"
#include "rhythmbox-config.h"
#include "rhythmbox-dbus.h"
#include "rhythmbox-menu-functions.h"
#include "rhythmbox-struct.h"
#include "rhythmbox-init.h"

extern AppletConfig myConfig;
extern AppletData myData;

static gboolean dbus_enable = FALSE;

CD_APPLET_DEFINITION ("Rhythmbox", 1, 4, 6, CAIRO_DOCK_CATEGORY_CONTROLER)


static void _load_surfaces (void)
{
	gchar *cUserImagePath;
	GString *sImagePath = g_string_new ("");
	//Chargement de l'image "default"
	if (myData.pSurface != NULL)
		cairo_surface_destroy (myData.pSurface);
	if (myConfig.cDefaultIcon != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cDefaultIcon);
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else
	{
		g_string_printf (sImagePath, "%s/default.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "stop"
	if (myData.pStopSurface != NULL)
		cairo_surface_destroy (myData.pStopSurface);
	if (myConfig.cStopIcon != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cStopIcon);
		myData.pStopSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else
	{
		g_string_printf (sImagePath, "%s/stop.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pStopSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "pause"
	if (myData.pPauseSurface != NULL)
		cairo_surface_destroy (myData.pPauseSurface);
	if (myConfig.cPauseIcon != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cPauseIcon);
		myData.pPauseSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else
	{
		g_string_printf (sImagePath, "%s/pause.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pPauseSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "play"
	if (myData.pPlaySurface != NULL)
		cairo_surface_destroy (myData.pPlaySurface);
	if (myConfig.cPlayIcon != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cPlayIcon);
		myData.pPlaySurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else
	{
		g_string_printf (sImagePath, "%s/play.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pPlaySurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	//Chargement de l'image "broken"
	if (myData.pBrokenSurface != NULL)
		cairo_surface_destroy (myData.pBrokenSurface);
	if (myConfig.cBrokenIcon != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cBrokenIcon);
		myData.pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else
	{
		g_string_printf (sImagePath, "%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	g_string_free (sImagePath, TRUE);
}

CD_APPLET_INIT_BEGIN (erreur)
	myConfig.defaultTitle = g_strdup (myIcon->acName);
	
	if (myDesklet != NULL)
	{
		/*myIcon->fWidth = MAX (1, myDesklet->iWidth - g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius/2;
		myIcon->fDrawY = g_iDockRadius/2;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;*/
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	_load_surfaces ();
	
	//Si le bus n'a pas encore ete acquis, on le recupere.
	if (! dbus_enable)
		dbus_enable = rhythmbox_dbus_get_dbus();
	myData.dbus_enable = dbus_enable;
	
	//Si le bus a ete acquis, on y connecte nos signaux.
	if (myData.dbus_enable)
	{
		rhythmbox_dbus_connect_to_bus ();
		
		dbus_detect_rhythmbox();
		if(myData.opening)
		{
			rhythmbox_getPlaying();
			rhythmbox_getPlayingUri();
			getSongInfos();
			update_icon( FALSE );
		}
		else
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurface)
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pBrokenSurface)
	}
	
	//Enregistrement des notifications
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT
	
	rhythmbox_dbus_disconnect_from_bus ();
	
	reset_config ();
	reset_data ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myDesklet != NULL)
	{
		/*myIcon->fWidth = MAX (1, myDesklet->iWidth - g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius/2;
		myIcon->fDrawY = g_iDockRadius/2;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;*/
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	//\_______________ On recharge les donnees qui ont pu changer.
	_load_surfaces ();
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		myConfig.defaultTitle = g_strdup (myIcon->acName);  // libere dans le reset_config() precedemment appele.
	}
	
	//\_______________ On redessine notre icone.
	if (myData.dbus_enable)
	{
		if(myData.opening)
		{
			update_icon( FALSE );
		}
		else
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurface)
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pBrokenSurface)
	}
CD_APPLET_RELOAD_END
