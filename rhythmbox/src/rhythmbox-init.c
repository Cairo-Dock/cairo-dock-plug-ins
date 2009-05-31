#include <stdlib.h>
#include <dbus/dbus-glib.h>

#include "rhythmbox-draw.h"
#include "rhythmbox-config.h"
#include "rhythmbox-dbus.h"
#include "rhythmbox-menu-functions.h"
#include "rhythmbox-struct.h"
#include "rhythmbox-init.h"

#include "3dcover-draw.h"


CD_APPLET_DEFINITION ("Rhythmbox",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_CONTROLER,
	N_("Control your Rhythmbox player directly in the dock !\n"
	"Left-click to launch it, and to play/pause."
	"Middle-click and Scroll-down for next song, Scroll-up for previous song.\n"
	"You can drag and drop covers (jpg) on the icon to use them,\n"
	" and songs to put them in the queue."),
	"Adrien Pilleboue (Necropotame) & Nochka85 (Opengl display)")

CD_APPLET_INIT_BEGIN
	if (myDesklet)  // on definit un mode de rendu pour notre desklet.
	{
		/*if (myConfig.extendedDesklet)
		{
			rhythmbox_add_buttons_to_desklet ();
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Controler", data);
		}
		else*/
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
	}
	
	//\_______________ on initialise DBus et on recupere l'etat courant si RB est deja lance.
	myData.dbus_enable = rhythmbox_dbus_connect_to_bus ();
	if (myData.dbus_enable)
	{
		dbus_detect_rhythmbox();
		if(myData.bIsRunning)  // player en cours d'execution, on recupere son etat.
		{
			rhythmbox_getPlaying();
			rhythmbox_getPlayingUri();
			getSongInfos();
			update_icon( FALSE );
		}
		else  // player eteint.
		{
			rhythmbox_set_surface (PLAYER_NONE);
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		rhythmbox_set_surface (PLAYER_BROKEN);
	}
	
	//\_______________ On prend en charge l'icone de l'appli rhythmbox.
	CD_APPLET_MANAGE_APPLICATION ("rhythmbox", myConfig.bStealTaskBarIcon);
	
	//\_______________ Enregistrement des notifications
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOpenglThemes)
	{
		myConfig.bOpenglThemes = cd_opengl_load_3D_theme (myApplet, myConfig.cThemePath);
		//CD_APPLET_REGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
		if (myDesklet)  // On ne teste le survol des boutons que si l'applet est détachée
			cairo_dock_register_notification (CAIRO_DOCK_MOUSE_MOVED,
				(CairoDockNotificationFunc) cd_opengl_test_mouse_over_buttons,
				CAIRO_DOCK_RUN_AFTER,
				myApplet);
		
	}
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
	cairo_dock_remove_notification_func (CAIRO_DOCK_MOUSE_MOVED,
		(CairoDockNotificationFunc) cd_opengl_test_mouse_over_buttons,
		myApplet);
	
	if (myData.iSidCheckCover != 0)
		g_source_remove (myData.iSidCheckCover);
	
	rhythmbox_dbus_disconnect_from_bus ();
	
	CD_APPLET_MANAGE_APPLICATION ("rhythmbox", FALSE);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	/*if (CD_APPLET_MY_CONFIG_CHANGED && myDesklet)
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
	}*/
	
	if (myDesklet)
	{
		/*if (myConfig.extendedDesklet)
		{
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Controler", data);
		}
		else*/
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
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
		CD_APPLET_MANAGE_APPLICATION ("rhythmbox", myConfig.bStealTaskBarIcon);
		
		CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
		cairo_dock_remove_notification_func (CAIRO_DOCK_MOUSE_MOVED,
			(CairoDockNotificationFunc) cd_opengl_test_mouse_over_buttons,
			myApplet);
		cd_opengl_reset_opengl_datas (myApplet);  // si le theme a change.
		
		if (myConfig.bOpenglThemes)
		{
			myConfig.bOpenglThemes = cd_opengl_load_3D_theme (myApplet, myConfig.cThemePath);			
			//CD_APPLET_REGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
			if (myDesklet)  // On ne teste le survol des boutons que si l'applet est détachée
				cairo_dock_register_notification (CAIRO_DOCK_MOUSE_MOVED,
					(CairoDockNotificationFunc) cd_opengl_test_mouse_over_buttons,
					CAIRO_DOCK_RUN_AFTER,
					myApplet);
		}
	}
	
	//\_______________ On redessine notre icone.
	if (myData.dbus_enable)
	{
		if(myData.bIsRunning)
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
	myData.CoverWasDistant = FALSE;
	
CD_APPLET_RELOAD_END
