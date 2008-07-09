/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <glib/gi18n.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib-bindings.h>

#include "musicplayer-dbus.h"
#include "musicplayer-draw.h"
#include "musicplayer-dbus.h"
#include "musicplayer-struct.h"
#include "musicplayer-notifications.h"
#include "musicplayer-init.h"
#include "musicplayer-config.h"

CD_APPLET_INCLUDE_MY_VARS

static void musicplayer_menu_previous (void)
{
	musicplayer_dbus_command(myData.DBus_commands.previous);
}

static void musicplayer_menu_next (void)
{
	musicplayer_dbus_command(myData.DBus_commands.next);
}

static void musicplayer_menu_toggle (void)
{
	musicplayer_dbus_command(myData.DBus_commands.toggle);
}


static void musicplayer_music (GtkMenuItem *menu_item, gpointer *data)
{
	music_dialog();
}

//*********************************************************************************
// Informations sur l'applet et l'auteur.
//*********************************************************************************
CD_APPLET_ABOUT (_D("Applet by Mav (Yann SLADEK)"))


//*********************************************************************************
// Fonction appelée a la construction du menu.
// Cette fonction remplit le menu principal avec les actions previous, next, et information.
//*********************************************************************************
CD_APPLET_ON_BUILD_MENU_BEGIN
	if (myData.dbus_enable)
	{
		CD_APPLET_ADD_IN_MENU (_D("Previous"), musicplayer_menu_previous, CD_APPLET_MY_MENU)
		
		CD_APPLET_ADD_IN_MENU (_D("Next"), musicplayer_menu_next, CD_APPLET_MY_MENU)
		
		CD_APPLET_ADD_IN_MENU (_D("Information"), musicplayer_music, CD_APPLET_MY_MENU)
		
		CD_APPLET_ADD_IN_MENU (_D("Toggle"), musicplayer_menu_toggle, CD_APPLET_MY_MENU)
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (CD_APPLET_MY_MENU)
CD_APPLET_ON_BUILD_MENU_END


//*********************************************************************************
// Fonction appelée au clique sur l'icone.
// Cette fonction met le lecteur en pause ou en lecture selon son état.
//*********************************************************************************
CD_APPLET_ON_CLICK_BEGIN
	
	//cd_message("exaile : On a cliqué");
	
	if(myData.opening)
	{
		if(myData.playing)
		{
			cd_message("exaile : On met pause");
			musicplayer_dbus_command(myData.DBus_commands.pause);
			cd_message("exaile : On a mis pause");
		}
		else if (myData.paused)
		{
			cd_message("exaile : On enlève pause");
			musicplayer_dbus_command(myData.DBus_commands.play);
		}
		else if (myData.stopped)
		{
			cd_message("exaile : On met play");
			musicplayer_dbus_command(myData.DBus_commands.play); 	
		}
	}
	else
	{
		cd_message("exaile n'est pas ouvert : on l'ouvre");
		//g_spawn_command_line_async ("exaile", NULL);
	}
CD_APPLET_ON_CLICK_END


//*********************************************************************************
// Fonction appelée au clique du milieu sur l'icone.
// Cette fonction passe a la chanson suivante.
//*********************************************************************************
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	cd_message ("");
	
	if ((myData.playing) || (myData.paused))
	{
		musicplayer_dbus_command(myData.DBus_commands.next);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_DROP_DATA_BEGIN

CD_APPLET_ON_DROP_DATA_END

