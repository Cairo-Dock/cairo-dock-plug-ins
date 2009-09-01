
/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Yann SLADEK (for any bug report, please mail me to mav@cairo-dock.org)
Fabrice Rey (fabounet@users.berlios.de)
Rémy Robertson (changfu@cairo-dock.org)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib/gi18n.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-musicplayer.h"
#include "applet-mpris.h"
#include "applet-amarok2.h"

/* On enregistre notre lecteur.
 */
void cd_musicplayer_register_amarok2_handler (void)
{
	MusicPlayerHandeler *pAmarok2 = cd_mpris_new_handler ();
	pAmarok2->cMprisService = "org.kde.amarok";
	pAmarok2->appclass = "amarok";
	pAmarok2->launch = "amarok";
	pAmarok2->name = "Amarok 2";
	pAmarok2->iPlayer = MP_AMAROK2;
	cd_musicplayer_register_my_handler (pAmarok2, "Amarok 2");
}
