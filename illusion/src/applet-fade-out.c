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
#include <math.h>

#include "applet-struct.h"
#include "applet-fade-out.h"

#define _update_alpha(pData) (pData)->fFadeOutAlpha = 1. - (pData)->fTime / myConfig.iFadeOutDuration

gboolean cd_illusion_init_fade_out (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	_update_alpha (pData);
	return TRUE;
}

void cd_illusion_update_fade_out (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	_update_alpha (pData);
	if (pData->fFadeOutAlpha < 0)
		pData->fFadeOutAlpha = 0;
	
	cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));
}

void cd_illusion_draw_fade_out_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	pIcon->fAlpha = pData->fFadeOutAlpha;  // on laisse l'icone se faire redessiner par quelqu'un d'autre.
}
