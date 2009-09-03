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


#ifndef __CD_DUSTBIN_DRAW__
#define  __CD_DUSTBIN_DRAW__


#include <cairo-dock.h>
#include "applet-struct.h"


CdDustbin *cd_dustbin_find_dustbin_from_uri (const gchar *cDustbinPath);

void cd_dustbin_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, CdDustbin *pDustbin);


gboolean cd_dustbin_check_trashes (Icon *icon);


void cd_dustbin_draw_quick_info (gboolean bRedraw);

void cd_dustbin_signal_full_dustbin (void);

#endif
