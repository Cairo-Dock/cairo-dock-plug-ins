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

gboolean banshee_get_data (void)
{
	if ((myData.dbus_enable) & (myData.opening))
	{
		cd_message("musicplayer : Bus ouvert et lecteur accessible pour Banshee");
		musicplayer_getStatus_integer(); // On récupère l'état de la lecture (play/pause/stop)

		if ((myData.playing) || (myData.paused))
		{
			cd_message("musicplayer : Lecture en cours");
			musicplayer_check_for_changes(); //On vérifie si les données ont changé
			//cd_message("exaile : dans la fonction init et dbus déjà activé avec animation --> %d", myData.data_have_changed);
			musicplayer_getSongInfos(); // On récupère toutes les infos de la piste en cours
			if (myData.data_have_changed) update_icon(TRUE); // On redessine l'icone avec l'animation
			else update_icon(FALSE); // Sinon on redessine l'icone mais sans animation
		}
		else // La lecture est stoppé, on met l'icone appropriée
		{
			musicplayer_set_surface (PLAYER_STOPPED); 
		}
	}	

	return TRUE; 	// On continue avec le timeout en attendant l'ouverture (soit par un clic soit par un lanceur)

}
