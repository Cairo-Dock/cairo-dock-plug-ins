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
		
		cairo_dock_redraw_my_icon (pIcon, CAIRO_CONTAINER (pDock));
		
		pIcon->fWidthFactor /= fScaleFactor;
		pIcon->fHeightFactor /= fScaleFactor;
	}
	
	return pData->fPulseAlpha != 0;
}


void cd_animations_draw_pulse_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData)
{
	if (pData->fPulseAlpha == 0 || pData->fPulseAlpha == 1 || pIcon->iIconTexture == 0)
		return ;
	
	glPushMatrix ();
	double fScaleFactor = (1 - myConfig.fPulseZoom) * pData->fPulseAlpha + myConfig.fPulseZoom;
	cairo_dock_set_icon_scale (pIcon, pDock, fScaleFactor/2);
	glColor4f(1., 1., 1., pData->fPulseAlpha * pIcon->fAlpha);
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture (GL_TEXTURE_2D, pIcon->iIconTexture);
	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.); glVertex3f(-1., 1,  0.);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1., 0.); glVertex3f( 1., 1,  0.);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1., 1.); glVertex3f( 1., -1,  0.);  // Top Right Of The Texture and Quad
	glTexCoord2f(0., 1.); glVertex3f(-1.,  -1,  0.);  // Top Left Of The Texture and Quad
	glEnd();
	glDisable (GL_BLEND);
	glDisable (GL_TEXTURE_2D);
	glPopMatrix ();
	
	pIcon->fAlpha = 1. - .3 * pData->fPulseAlpha;
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
