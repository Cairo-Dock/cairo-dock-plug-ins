/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

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
