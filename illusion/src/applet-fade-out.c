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
#define CD_ILLUSION_FADE_OUT_LIMIT .2

gboolean cd_illusion_init_fade_out (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData, double dt)
{
	pData->fFadeOutSpeed = dt / myConfig.iFadeOutDuration;
	pData->fFadeOutAlpha = 1.;
	
	return TRUE;
}


gboolean cd_illusion_update_fade_out (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	pData->fFadeOutAlpha -= pData->fFadeOutSpeed;
	if (pData->fFadeOutAlpha < 0)
		pData->fFadeOutAlpha = 0;
	
	if (pData->fFadeOutAlpha < CD_ILLUSION_FADE_OUT_LIMIT)
		cairo_dock_update_removing_inserting_icon_size_default (pIcon);
	
	cairo_dock_redraw_icon (pIcon, pDock);
	return (pData->fFadeOutAlpha > 0 || pIcon->fPersonnalScale > .05);
}

void cd_illusion_draw_fade_out_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	pIcon->fAlpha = pData->fFadeOutAlpha;
}
