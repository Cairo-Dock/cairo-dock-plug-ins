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
#include "applet-bounce.h"


void cd_animations_init_blink (CDAnimationData *pData, double dt)
{
	pData->iBlinkCount = myConfig.iBlinkDuration / dt - 1;
	pData->bIsBlinking = TRUE;
}

gboolean cd_animations_update_blink (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL)
{
	int c = pData->iBlinkCount;
	int n = (int) floor (myConfig.iBlinkDuration / dt) / 2;  // nbre d'iteration pour une inversion d'alpha.
	if ( (c/n) & 1)
		pData->fBlinkAlpha = 1. * (c%n) / n;
	else
		pData->fBlinkAlpha = 1. * (n - 1 - (c%n)) / n;
	pData->fBlinkAlpha *= pData->fBlinkAlpha;  // pour accentuer.
	if (pData->fBlinkAlpha < .01)
		pData->fBlinkAlpha = .01;
	
	pData->iBlinkCount --;
	
	cairo_dock_redraw_icon (pIcon, CAIRO_CONTAINER (pDock));
	
	return (pData->iBlinkCount > 0);
}


void cd_animations_draw_blink_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, int sens)
{
	if (sens == 1)
		pIcon->fAlpha *= pData->fBlinkAlpha;
	else
		pIcon->fAlpha /= pData->fBlinkAlpha;
}
