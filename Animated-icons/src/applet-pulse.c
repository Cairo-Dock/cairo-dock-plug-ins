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
#include "applet-notifications.h"
#include "applet-pulse.h"


static void init (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL)
{
	if (myConfig.iPulseDuration == 0)
		return ;
	pData->fPulseAlpha = 1.;
	pData->fPulseSpeed = dt / myConfig.iPulseDuration;
}


static gboolean update (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL, gboolean bRepeat)
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
	else
		cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));
	
	gboolean bContinue = (pData->fPulseAlpha != 0);
	if (! bContinue && bRepeat)
		init (pIcon, pDock, pData, dt, bUseOpenGL);
	return bContinue;
}


static void render (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	if (pData->fPulseAlpha == 0 || pData->fPulseAlpha == 1)
		return ;
	if (pData->bHasBeenPulsed)
		return ;
	if (pCairoContext)
	{
		cairo_save (pCairoContext);
		double fScaleFactor = (1 - myConfig.fPulseZoom) * pData->fPulseAlpha + myConfig.fPulseZoom;
		if (pDock->container.bIsHorizontal)
			cairo_translate (pCairoContext, pIcon->fWidth * pIcon->fScale * (1 - fScaleFactor) / 2, pIcon->fHeight * pIcon->fScale * (1 - fScaleFactor) / 2);
		else
			cairo_translate (pCairoContext, pIcon->fHeight * pIcon->fScale * (1 - fScaleFactor) / 2, pIcon->fWidth * pIcon->fScale * (1 - fScaleFactor) / 2);
		
		cairo_dock_set_icon_scale_on_context (pCairoContext, pIcon, pDock->container.bIsHorizontal, 1., pDock->container.bDirectionUp);
		cairo_scale (pCairoContext, fScaleFactor, fScaleFactor);
		
		cairo_set_source_surface (pCairoContext, pIcon->pIconBuffer, 0.0, 0.0);
		cairo_paint_with_alpha (pCairoContext, pData->fPulseAlpha * pIcon->fAlpha);
		cairo_restore (pCairoContext);
	}
	else
	{
		glPushMatrix ();
		double fScaleFactor = (1 - myConfig.fPulseZoom) * pData->fPulseAlpha + myConfig.fPulseZoom;
		cairo_dock_set_icon_scale (pIcon, CAIRO_CONTAINER (pDock), fScaleFactor);
		_cairo_dock_enable_texture ();
		_cairo_dock_set_blend_alpha ();
		_cairo_dock_set_alpha (pData->fPulseAlpha * pIcon->fAlpha);
		_cairo_dock_apply_texture (pIcon->iIconTexture);
		_cairo_dock_disable_texture ();
		glPopMatrix ();
	}
	///pIcon->fAlpha = 1. - .5 * pData->fPulseAlpha;
}


void cd_animations_register_pulse (void)
{
	CDAnimation *pAnimation = &myData.pAnimations[CD_ANIMATIONS_PULSE];
	pAnimation->cName = "pulse";
	pAnimation->cDisplayedName = D_("Pulse");
	pAnimation->id = CD_ANIMATIONS_PULSE;
	pAnimation->bDrawIcon = FALSE;
	pAnimation->bDrawReflect = FALSE;
	pAnimation->init = init;
	pAnimation->update = update;
	pAnimation->render = render;
	pAnimation->post_render = NULL;
	cd_animations_register_animation (pAnimation);
}
