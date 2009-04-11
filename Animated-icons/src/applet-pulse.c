/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-pulse.h"


void cd_animations_init_pulse (CDAnimationData *pData, double dt)
{
	if (myConfig.iPulseDuration == 0)
		return ;
	pData->fPulseAlpha = 1.;
	pData->fPulseSpeed = dt / myConfig.iPulseDuration;
}


gboolean cd_animations_update_pulse (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, gboolean bUseOpenGL)
{
	pData->fPulseAlpha -= pData->fPulseSpeed;
	if (pData->fPulseAlpha < 0)
		pData->fPulseAlpha = 0;
	
	if (! bUseOpenGL)
	{
		double fScaleFactor = 1 + (1 - pData->fPulseAlpha);
		pIcon->fWidthFactor *= fScaleFactor;
		pIcon->fHeightFactor *= fScaleFactor;
		
		cairo_dock_redraw_icon (pIcon, CAIRO_CONTAINER (pDock));
		
		pIcon->fWidthFactor /= fScaleFactor;
		pIcon->fHeightFactor /= fScaleFactor;
	}
	
	return pData->fPulseAlpha != 0;
}


void cd_animations_draw_pulse_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData)
{
	if (pData->fPulseAlpha == 0 || pData->fPulseAlpha == 1 || pIcon->iIconTexture == 0)
		return ;
	
	pIcon->fAlpha = 1. - .3 * pData->fPulseAlpha;
	glPushMatrix ();
	double fScaleFactor = (1 - myConfig.fPulseZoom) * pData->fPulseAlpha + myConfig.fPulseZoom;
	cairo_dock_set_icon_scale (pIcon, CAIRO_CONTAINER (pDock), fScaleFactor);
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_over ();
	_cairo_dock_set_alpha (pData->fPulseAlpha * pIcon->fAlpha);
	_cairo_dock_apply_texture (pIcon->iIconTexture);
	_cairo_dock_disable_texture ();
	glPopMatrix ();
}

void cd_animations_draw_pulse_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	if (pData->fPulseAlpha == 0 || pData->fPulseAlpha == 1 || pIcon->pIconBuffer == NULL)
		return ;
	cairo_save (pCairoContext);
	double fScaleFactor = (1 - myConfig.fPulseZoom) * pData->fPulseAlpha + myConfig.fPulseZoom;
	if (pDock->bHorizontalDock)
		cairo_translate (pCairoContext, pIcon->fWidth * pIcon->fScale * (1 - fScaleFactor) / 2, pIcon->fHeight * pIcon->fScale * (1 - fScaleFactor) / 2);
	else
		cairo_translate (pCairoContext, pIcon->fHeight * pIcon->fScale * (1 - fScaleFactor) / 2, pIcon->fWidth * pIcon->fScale * (1 - fScaleFactor) / 2);
	
	cairo_dock_set_icon_scale_on_context (pCairoContext, pIcon, pDock->bHorizontalDock, pDock->fRatio * fScaleFactor, pDock->bDirectionUp);
	
	cairo_set_source_surface (pCairoContext, pIcon->pIconBuffer, 0.0, 0.0);
	cairo_paint_with_alpha (pCairoContext, pData->fPulseAlpha * pIcon->fAlpha);
	cairo_restore (pCairoContext);
	
	pIcon->fAlpha = 1. - .3 * pData->fPulseAlpha;
}
