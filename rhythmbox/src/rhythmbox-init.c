#include <stdlib.h>
#include <dbus/dbus-glib.h>

#include "rhythmbox-draw.h"
#include "rhythmbox-config.h"
#include "rhythmbox-dbus.h"
#include "rhythmbox-menu-functions.h"
#include "rhythmbox-struct.h"
#include "rhythmbox-init.h"


CD_APPLET_DEFINITION ("Rhythmbox", 1, 5, 4, CAIRO_DOCK_CATEGORY_CONTROLER)



static void _rhythmbox_set_desklet_renderer (void)
{
	const gchar *cConfigName = NULL;
	switch (myConfig.iDecoration)
	{
		case MY_APPLET_PERSONNAL :
		break ;
		case MY_APPLET_CD_BOX:
			cConfigName = "CD box";
		break ;
		case MY_APPLET_FRAME_REFLECTS :
			cConfigName = "frame&reflects";
		break ;
		case MY_APPLET_SCOTCH :
				cConfigName = "scotch";
		break ;
		case MY_APPLET_FRAME_SCOTCH :
				cConfigName = "frame with scotch";
		break ;
		default :
			return ;
	}
	if (cConfigName != NULL)
	{
		CairoDeskletRendererConfigPtr pConfig = cairo_dock_get_desklet_renderer_predefined_config ("Simple", cConfigName);
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Simple", pConfig);
	}
	else if (myConfig.cFrameImage != NULL || myConfig.cReflectImage != NULL)
	{
		gpointer pManualConfig[9] = {myConfig.cFrameImage, myConfig.cReflectImage, GINT_TO_POINTER (CAIRO_DOCK_FILL_SPACE), &myConfig.fFrameAlpha, &myConfig.fReflectAlpha, GINT_TO_POINTER (myConfig.iLeftOffset), GINT_TO_POINTER (myConfig.iTopOffset), GINT_TO_POINTER (myConfig.iRightOffset), GINT_TO_POINTER (myConfig.iBottomOffset)};
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Simple", pManualConfig);
	}
	else
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
}

CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet)
	{
		if (myConfig.extendedDesklet)
		{
			rhythmbox_add_buttons_to_desklet ();
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Controler", data);
		}
		else
		{
			_rhythmbox_set_desklet_renderer ();
		}
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
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT
	
	rhythmbox_dbus_disconnect_from_bus ();
	
	if (myIcon->cClass != NULL)
		cairo_dock_deinhibate_class ("rhythmbox", myIcon);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED && myDesklet)
	{
		if ( ! myConfig.extendedDesklet && myDesklet->icons != NULL)
		{
			g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myDesklet->icons);
			myDesklet->icons = NULL;
		}
		else if (myConfig.extendedDesklet && myDesklet->icons == NULL)
		{
			rhythmbox_add_buttons_to_desklet ();
		}
	}
	
	if (myDesklet)
	{
		if (myConfig.extendedDesklet)
		{
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Controler", data);
		}
		else
		{
			_rhythmbox_set_desklet_renderer ();
		}
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
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL);
			rhythmbox_set_surface (PLAYER_NONE);
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL);
		rhythmbox_set_surface (PLAYER_BROKEN);
	}
CD_APPLET_RELOAD_END
