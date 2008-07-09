/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <dbus/dbus-glib.h>

#include <glib/gi18n.h>

#include "musicplayer-draw.h"
#include "musicplayer-config.h"
#include "musicplayer-dbus.h"
#include "musicplayer-notifications.h"
#include "musicplayer-struct.h"
#include "musicplayer-init.h"


static guint iTimerID_dbus;
static guint iTimerID_player;

CD_APPLET_DEFINITION ("MusicPlayer", 1, 5, 4, CAIRO_DOCK_CATEGORY_CONTROLER)




CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (myConfig.bStealTaskBarIcon)
	{
		cairo_dock_inhibate_class ("exaile", myIcon);
	}
	cd_message("musicplayer : Lecteur utilise --> %i", myConfig.iPlayer);
	
	
	switch (myConfig.iPlayer)
	{
	
		case MY_EXAILE :
			
			iTimerID_dbus = g_timeout_add (1000, (GSourceFunc) check_dbus_connection, (gpointer) NULL);
			iTimerID_player = g_timeout_add (1000, (GSourceFunc) exaile_get_data, (gpointer) NULL);
			//cd_message("Valeur de opening --> %d, Valeur de enable --> %d", myData.opening, myData.dbus_enable);
			break;
		
		case MY_BANSHEE :
			iTimerID_dbus = g_timeout_add (1000, (GSourceFunc) check_dbus_connection, (gpointer) NULL);
			iTimerID_player = g_timeout_add (1000, (GSourceFunc) banshee_get_data, (gpointer) NULL);
			break;
			
		case MY_QUOD_LIBET :
			/*g_timeout_add (1000, (GSourceFunc) check_dbus_connection, (gpointer) NULL);
			g_timeout_add (1000, (GSourceFunc) quod_libet_get_data, (gpointer) NULL);*/
			break;

		case MY_LISTEN :
			/*g_timeout_add (1000, (GSourceFunc) check_dbus_connection, (gpointer) NULL);
			g_timeout_add (1000, (GSourceFunc) listen_get_data, (gpointer) NULL);*/ // Abandon pour le moment, trop merdique pour les infos
			break;
			
		case MY_RHYTHMBOX :
			myData.dbus_enable = musicplayer_rhythmbox_dbus_connection ();
			//musicplayer_dbus_connect_to_bus_Shell();
			if (myData.dbus_enable)
			{
				dbus_detect_rhythmbox();
				if(myData.opening)
				{
					cd_message("musicplayer : le lecteur est ouvert");
					rhythmbox_getPlaying();
					rhythmbox_getPlayingUri();
					getSongInfos();
					update_icon( FALSE );
				}
				else
				{
					cd_message("musicplayer : le lecteur n'est pas ouvert");

					musicplayer_set_surface (PLAYER_NONE);
				}
			}
			else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
			{
				musicplayer_set_surface (PLAYER_BROKEN);
			}
		
			break;
		
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
	
	musicplayer_dbus_disconnect_from_bus ();
	
	if (iTimerID_dbus != 0) g_source_remove(iTimerID_dbus);
	if (iTimerID_player != 0) g_source_remove(iTimerID_player);
	
	if (myIcon->cClass != NULL)
		cairo_dock_deinhibate_class ("exaile", myIcon);
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
			cairo_dock_deinhibate_class ("exaile", myIcon);
		}
		else if (myIcon->cClass == NULL && myConfig.bStealTaskBarIcon)
		{
			cairo_dock_inhibate_class ("exaile", myIcon);
		}
	}
	
	g_source_remove(iTimerID_dbus);
	g_source_remove(iTimerID_player);
	
	/*//\_______________ On redessine notre icone.
	if (myData.opening)
	{
		if(myData.dbus_enable)
		{
			update_icon( FALSE );
		}
		else
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL);
			musicplayer_set_surface (PLAYER_BROKEN);
		}
	}
	else  // sinon on signale par l'icone appropriee que le lecteur n'est pas ouvert.
	{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL);
			musicplayer_set_surface (PLAYER_NONE);
	}*/
CD_APPLET_RELOAD_END
