/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib/gi18n.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-musicplayer.h"
#include "applet-mpris.h"
#include "applet-guayadeque.h"

/* On enregistre notre lecteur.
 * Par matttbe => pour déresponsabiliser fab ça ^^
 */
void cd_musicplayer_register_guayadeque_handler (void)
{
	MusicPlayerHandeler *pGuayadeque = cd_mpris_new_handler ();
	pGuayadeque->cMprisService = "org.mpris.guayadeque";
	pGuayadeque->appclass = "guayadeque";
	pGuayadeque->launch = "guayadeque";
	pGuayadeque->name = "Guayadeque";
	pGuayadeque->iPlayer = MP_GUAYADEQUE;
	cd_musicplayer_register_my_handler (pGuayadeque, "Guayadeque");
}
