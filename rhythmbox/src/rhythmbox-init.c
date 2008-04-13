#include <stdlib.h>

#include "rhythmbox-draw.h"
#include "rhythmbox-config.h"
#include "rhythmbox-dbus.h"
#include "rhythmbox-menu-functions.h"
#include "rhythmbox-struct.h"
#include "rhythmbox-init.h"


CD_APPLET_DEFINITION ("Rhythmbox", 1, 5, 4, CAIRO_DOCK_CATEGORY_CONTROLER)


CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	myData.dbus_enable = rhythmbox_dbus_connect_to_bus ();
	if (myData.dbus_enable)
	{
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
			rhythmbox_set_surface (PLAYER_NONE);
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		rhythmbox_set_surface (PLAYER_BROKEN);
	}
	
	if (myConfig.bStealTaskBarIcon)
	{
		cairo_dock_inhibate_class ("rhythmbox", myIcon);
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
	
	if (myIcon->cClass != NULL)
		cairo_dock_deinhibate_class ("rhythmbox", myIcon);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) { // reset surfaces.
		if (myData.pSurfaces[i] != NULL) {
			cairo_surface_destroy (myData.pSurfaces[i]);
			myData.pSurfaces[i] = NULL;
		}
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myIcon->cClass != NULL && ! myConfig.bStealTaskBarIcon)
		{
			cairo_dock_deinhibate_class ("rhythmbox", myIcon);
		}
		else if (myIcon->cClass == NULL && myConfig.bStealTaskBarIcon)
		{
			cairo_dock_inhibate_class ("rhythmbox", myIcon);
		}
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
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			rhythmbox_set_surface (PLAYER_NONE);
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		rhythmbox_set_surface (PLAYER_BROKEN);
	}
CD_APPLET_RELOAD_END
