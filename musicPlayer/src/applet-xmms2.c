
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
#include "applet-xmms2.h"

/* On enregistre notre lecteur.
 */
void cd_musicplayer_register_xmms2_handler (void)
{
	MusicPlayerHandeler *pXmms2 = cd_mpris_new_handler ();
	pXmms2->cMprisService = "org.xmms.xmms2";  /// trouver le nom ...
	pXmms2->appclass = "xmms2";  /// idem...
	pXmms2->launch = "xmms2";  /// idem...
	pXmms2->name = "XMMS 2";
	pXmms2->iPlayer = MP_XMMS2;
	cd_musicplayer_register_my_handler (pXmms2, "XMMS 2");
}
