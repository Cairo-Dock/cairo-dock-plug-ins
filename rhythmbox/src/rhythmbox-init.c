#include <stdlib.h>

#include "rhythmbox-draw.h"
#include "rhythmbox-config.h"
#include "rhythmbox-dbus.h"
#include "rhythmbox-menu-functions.h"
#include "rhythmbox-struct.h"
#include "rhythmbox-init.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_DEFINITION ("Rhythmbox", 1, 4, 6)


static void _load_surfaces (void)
{
	GString *sImagePath = g_string_new ("");  // ce serait bien de pouvoir choisir ses icones, comme dans l'applet logout...
	//Chargement de l'image "default"
	g_string_printf (sImagePath, "%s/default.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	//Chargement de l'image "stop"
	g_string_printf (sImagePath, "%s/stop.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pStopSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	//Chargement de l'image "pause"
	g_string_printf (sImagePath, "%s/pause.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pPauseSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	//Chargement de l'image "play"
	g_string_printf (sImagePath, "%s/play.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pPlaySurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	//Chargement de l'image "broken"
	g_string_printf (sImagePath, "%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
	myData.pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	
	g_string_free (sImagePath, TRUE);
}

CD_APPLET_INIT_BEGIN (erreur)
	myConfig.defaultTitle = g_strdup (myIcon->acName);
	
	_load_surfaces ();
	
	//Si le bus n'a pas encore ete acquis, on le recupere.
	if (! myData.dbus_enable)
		myData.dbus_enable = rhythmbox_dbus_get_dbus();
	
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
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	rhythmbox_dbus_disconnect_from_bus ();
	
	reset_config ();
	reset_data ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
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
