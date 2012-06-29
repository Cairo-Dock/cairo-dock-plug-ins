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
#include "applet-bounce.h"


static void init (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL)
{
	int m = (1 - myConfig.fBounceFlatten) / .1;
	pData->iBounceCount = myConfig.iBounceDuration / dt - 1 + m;
	if (pData->fResizeFactor == 0)
		pData->fResizeFactor = 1.;
	if (pData->fFlattenFactor == 0)
		pData->fFlattenFactor = 1.;
}


static gboolean update (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL, gboolean bRepeat)
{
	int m = (1 - myConfig.fBounceFlatten) / .1;  // pas de 0.1
	int n = myConfig.iBounceDuration / dt + m;  // nbre d'iteration pour 1 aplatissement+montree+descente.
	int k = n - (pData->iBounceCount % n) - m;  // m iterations pour s'aplatir.
	n -= m;   // nbre d'iteration pour 1 montree+descente.
	//g_print ("%s (%d)\n", __func__, pData->iBounceCount);
	
	double fPrevElevation = pData->fElevation;
	double fPrevDeltaY = pIcon->fDeltaYReflection;
	if (k > 0)
	{
		if (pData->iBounceCount == 1 && ! bRepeat)
			pData->fResizeFactor = 1.;
		else if (pData->fResizeFactor > myConfig.fBounceResize)
		{
			pData->fResizeFactor -= (1 - myConfig.fBounceResize) / (n/2);
		}
		
		double fPossibleDeltaY = MIN (50, (pDock->container.bDirectionUp ? pIcon->fDrawY : pDock->container.iHeight - (pIcon->fDrawY + pIcon->fHeight * pIcon->fScale)) + (1-pData->fResizeFactor)*pIcon->fHeight*pIcon->fScale);  // on borne a 50 pixels pour les rendus qui ont des fenetres grandes..
		if (pData->iBounceCount == 1 && ! bRepeat)
		{
			pData->fElevation = 0.;
			pIcon->fDeltaYReflection = 0.;
		}
		else
		{
			pData->fElevation = 1.*k / (n/2) * fPossibleDeltaY * (2 - 1.*k/(n/2)) - (pDock->container.bDirectionUp ? (1 - pData->fResizeFactor) * pIcon->fHeight*pIcon->fScale/2 : 0);
			pIcon->fDeltaYReflection = 1.40 * pData->fElevation;  // le reflet "rebondira" de 40% de la hauteur au sol.
			if (! bUseOpenGL)  // on prend en compte la translation du au fHeightFactor.
				pIcon->fDeltaYReflection -= (pDock->container.bIsHorizontal ? pIcon->fHeight * pIcon->fScale * pIcon->fHeightFactor * (1 - pData->fResizeFactor) / (pDock->container.bDirectionUp ? 2:1) : pIcon->fWidth * pIcon->fScale * (1 - pData->fResizeFactor) / 2);
			else if (! pDock->container.bDirectionUp)
			{
				pData->fElevation -= (1 - pData->fResizeFactor) * pIcon->fHeight*pIcon->fScale/2;
			}
		}
		//g_print (" ^ pData->fElevation : %.2f (x%.2f)\n", pData->fElevation, pData->fResizeFactor);
		pData->fFlattenFactor = 1.;
	}
	else  // on commence par s'aplatir.
	{
		pData->fFlattenFactor = - (1. - myConfig.fBounceFlatten) / m * k + myConfig.fBounceFlatten;  // varie de 1. exclus a f inclus.
		if (pDock->container.bDirectionUp)
			pData->fElevation = - (1. - pData->fFlattenFactor * pData->fResizeFactor) / 2 * pIcon->fHeight * pIcon->fScale;
		
		pIcon->fDeltaYReflection = pData->fElevation;
		if (! bUseOpenGL)
			pIcon->fDeltaYReflection -= (pDock->container.bIsHorizontal ? pIcon->fHeight * pIcon->fScale * (1 - pData->fResizeFactor * pData->fFlattenFactor) / (pDock->container.bDirectionUp ? 2:1) : pIcon->fWidth * pIcon->fScale * (1 - pData->fResizeFactor * pData->fFlattenFactor) / 2);
		else if (! pDock->container.bDirectionUp)
		{
			pData->fElevation = - (1 - pData->fResizeFactor * pData->fFlattenFactor) * pIcon->fHeight*pIcon->fScale/2;
		}
		//g_print (" v pData->fElevation : %.2f (x%.2f)\n", pData->fElevation, pData->fFlattenFactor);
	}
	
	pData->iBounceCount --;  // c'est une loi de type acceleration dans le champ de pesanteur. 'g' et 'v0' n'interviennent pas directement, car s'expriment en fonction de 'fPossibleDeltaY' et 'n'.
	
	if (! bUseOpenGL)
	{
		double fDamageWidthFactor = pIcon->fWidthFactor;
		double fDamageHeightFactor = pIcon->fHeightFactor;
		double fDeltaYReflection = pIcon->fDeltaYReflection;
		
		fPrevDeltaY = MAX (fPrevDeltaY, pIcon->fDeltaYReflection);
		pIcon->fDeltaYReflection = fPrevDeltaY;
		fPrevElevation = MAX (fPrevElevation, pData->fElevation);
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		pIcon->fDrawY -= (pDock->container.bDirectionUp ? 1 : 0) * fPrevElevation;
		pIcon->fHeight += fPrevElevation;
		
		cairo_dock_redraw_icon (pIcon, CAIRO_CONTAINER (pDock));
		pIcon->fDrawY += (pDock->container.bDirectionUp ? 1 : 0) * fPrevElevation;
		pIcon->fWidthFactor = fDamageWidthFactor;
		pIcon->fHeightFactor = fDamageHeightFactor;
		pIcon->fDeltaYReflection = fDeltaYReflection;
		pIcon->fHeight -= fPrevElevation;
	}
	else
		cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));
	
	gboolean bContinue = (pData->iBounceCount > 0);
	if (! bContinue && bRepeat)
		init (pIcon, pDock, pData, dt, bUseOpenGL);
	
	return bContinue;
}

static inline _translate (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext, int sens)
{
	if (pCairoContext)
	{
		if (pDock->container.bIsHorizontal)
			cairo_translate (pCairoContext,
				pIcon->fWidth * pIcon->fScale * (1 - pIcon->fWidthFactor) / 2 * sens,
				(pDock->container.bDirectionUp ? 1 : 0) * pIcon->fHeight * pIcon->fScale * (1 - pIcon->fHeightFactor) / 2 * sens);
		else
			cairo_translate (pCairoContext,
				(pDock->container.bDirectionUp ? 1 : 0) * pIcon->fHeight * pIcon->fScale * (1 - pIcon->fHeightFactor) / 2 * sens,
				pIcon->fWidth * pIcon->fScale * (1 - pIcon->fWidthFactor) / 2 * sens);

		if (pDock->container.bIsHorizontal)
			cairo_translate (pCairoContext,
				0.,
				- (pDock->container.bDirectionUp ? 1 : -1) * pData->fElevation * sens);
		else
			cairo_translate (pCairoContext,
				- (pDock->container.bDirectionUp ? 1 : -1) * pData->fElevation * sens,
				0.);
	}
	else
	{
		if (pDock->container.bIsHorizontal)
			glTranslatef (0., (pDock->container.bDirectionUp ? 1 : -1) * pData->fElevation * sens, 0.);
		else
			glTranslatef ((pDock->container.bDirectionUp ? -1 : 1) * pData->fElevation * sens, 0., 0.);
	}
}

static void render (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	pIcon->fHeightFactor *= pData->fFlattenFactor;
	pIcon->fHeightFactor *= pData->fResizeFactor;
	pIcon->fWidthFactor *= pData->fResizeFactor;
	
	_translate (pIcon, pDock, pData, pCairoContext, 1);
}

static void post_render (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	pIcon->fHeightFactor /= pData->fFlattenFactor;
	pIcon->fHeightFactor /= pData->fResizeFactor;
	pIcon->fWidthFactor /= pData->fResizeFactor;
	
	_translate (pIcon, pDock, pData, pCairoContext, -1);
}


void cd_animations_register_bounce (void)
{
	CDAnimation *pAnimation = &myData.pAnimations[CD_ANIMATIONS_BOUNCE];
	pAnimation->cName = "bounce";
	pAnimation->cDisplayedName = D_("Bounce");
	pAnimation->id = CD_ANIMATIONS_BOUNCE;
	pAnimation->bDrawIcon = FALSE;
	pAnimation->bDrawReflect = FALSE;
	pAnimation->init = init;
	pAnimation->update = update;
	pAnimation->render = render;
	pAnimation->post_render = post_render;
	cd_animations_register_animation (pAnimation);
}
