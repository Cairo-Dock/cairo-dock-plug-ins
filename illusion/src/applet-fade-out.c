/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

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
	
	cairo_dock_redraw_icon (pIcon, pDock);
}

void cd_illusion_draw_fade_out_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	pIcon->fAlpha = pData->fFadeOutAlpha;  // on laisse l'icone se faire redessiner par quelqu'un d'autre.
}
