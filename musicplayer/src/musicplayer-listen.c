
/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"
#include "string.h"
#include <dbus/dbus-glib.h>
#include <glib/gi18n.h>
#include "musicplayer-dbus.h"
#include "musicplayer-struct.h"
#include "musicplayer-init.h"
#include "musicplayer-config.h"
#include "musicplayer-draw.h"

CD_APPLET_INCLUDE_MY_VARS

void musicplayer_getStatus_for_listen (void)
{
	myData.paused = FALSE;
	myData.playing = FALSE;
	
	myData.full_data = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_full_data);
	cd_message("musicplayer : data --> %s", myData.full_data);
	if (myData.full_data == NULL) 
	{
		myData.status = 0;
		myData.paused = TRUE;
	}
	else
	{
		myData.status=1;
		myData.playing = TRUE;	
	}
	g_free(myData.full_data);
}

void musicplayer_getSongInfos_for_listen(void)
{
	cd_message ("musicplayer : On récupère les infos de la musique jouée");
	
	if (myData.full_data != NULL) g_free (myData.full_data);
	myData.full_data = cairo_dock_dbus_get_string (dbus_proxy_player, myData.DBus_commands.get_full_data);
	
	cd_message("musicplayer : %s", myData.full_data);
}


gboolean listen_get_data (void)
{
	gchar *data_temp;
	if ((myData.dbus_enable) & (myData.opening))
	{
		cd_message("musicplayer : Bus ouvert et lecteur accessible pour Listen");
		//musicplayer_getStatus_integer(); // On récupère l'état de la lecture (play/pause/stop)
		musicplayer_getStatus_for_listen();
		
		if (myData.playing)
		{
			cd_message("musicplayer : Lecture en cours");
			//musicplayer_getSongInfos_for_listen();
			data_temp = musicplayer_dbus_getValue(myData.DBus_commands.get_full_data);
			cd_message("musicplayer : data_temp -> %s", data_temp);
			
			if (g_ascii_strcasecmp(data_temp,myData.full_data)) myData.data_have_changed = TRUE;
			else  myData.data_have_changed = FALSE;
			musicplayer_getSongInfos_for_listen();
			cd_message("musicplayer : valeur de have_changed : %d", myData.data_have_changed);
			/*musicplayer_getSongInfos(); // On récupère toutes les infos de la piste en cours
			if (myData.data_have_changed) update_icon(TRUE); // On redessine l'icone avec l'animation
			else update_icon(FALSE); // Sinon on redessine l'icone mais sans animation*/
		}
		else // La lecture est stoppé, on met l'icone appropriée
		{
			cd_message("musicplayer : Pause");
			musicplayer_set_surface (PLAYER_PAUSED); 
		}
	}	

	return TRUE; 	// On continue avec le timeout en attendant l'ouverture (soit par un clic soit par un lanceur)

}
