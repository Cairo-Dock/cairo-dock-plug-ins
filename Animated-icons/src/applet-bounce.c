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


void cd_animations_init_bounce (CairoDock *pDock, CDAnimationData *pData, double dt)
{
	int m = (1 - myConfig.fBounceFlatten) / .1;
	pData->iBounceCount = myConfig.iBounceDuration / dt - 1 + m;
	if (pData->fResizeFactor == 0)
		pData->fResizeFactor = 1.;
	if (pData->fFlattenFactor == 0)
		pData->fFlattenFactor = 1.;
	pData->bIsBouncing = TRUE;
}


gboolean cd_animations_update_bounce (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL, gboolean bWillContinue)
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
		if (pData->iBounceCount == 1 && ! bWillContinue)
			pData->fResizeFactor = 1.;
		else if (pData->fResizeFactor > myConfig.fBounceResize)
		{
			pData->fResizeFactor -= (1 - myConfig.fBounceResize) / (n/2);
		}
		
		double fPossibleDeltaY = MIN (50, (pDock->bDirectionUp ? pIcon->fDrawY : pDock->iCurrentHeight - (pIcon->fDrawY + pIcon->fHeight * pIcon->fScale)) + (1-pData->fResizeFactor)*pIcon->fHeight*pIcon->fScale);  // on borne a 50 pixels pour les rendus qui ont des fenetres grandes..
		if (pData->iBounceCount == 1 && ! bWillContinue)
		{
			pData->fElevation = 0.;
			pIcon->fDeltaYReflection = 0.;
		}
		else
		{
			pData->fElevation = 1.*k / (n/2) * fPossibleDeltaY * (2 - 1.*k/(n/2)) - (pDock->bDirectionUp ? (1 - pData->fResizeFactor) * pIcon->fHeight*pIcon->fScale/2 : 0);
			pIcon->fDeltaYReflection = 1.40 * pData->fElevation;  // le reflet "rebondira" de 40% de la hauteur au sol.
			if (! bUseOpenGL)  // on prend en compte la translation du au fHeightFactor.
				pIcon->fDeltaYReflection -= (pDock->bHorizontalDock ? pIcon->fHeight * pIcon->fScale * pIcon->fHeightFactor * (1 - pData->fResizeFactor) / (pDock->bDirectionUp ? 2:1) : pIcon->fWidth * pIcon->fScale * (1 - pData->fResizeFactor) / 2);
			else if (! pDock->bDirectionUp)
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
		if (pDock->bDirectionUp)
			pData->fElevation = - (1. - pData->fFlattenFactor * pData->fResizeFactor) / 2 * pIcon->fHeight * pIcon->fScale;
		
		pIcon->fDeltaYReflection = pData->fElevation;
		if (! bUseOpenGL)
			pIcon->fDeltaYReflection -= (pDock->bHorizontalDock ? pIcon->fHeight * pIcon->fScale * (1 - pData->fResizeFactor * pData->fFlattenFactor) / (pDock->bDirectionUp ? 2:1) : pIcon->fWidth * pIcon->fScale * (1 - pData->fResizeFactor * pData->fFlattenFactor) / 2);
		else if (! pDock->bDirectionUp)
		{
			pData->fElevation = - (1 - pData->fResizeFactor * pData->fFlattenFactor) * pIcon->fHeight*pIcon->fScale/2;
		}
		//g_print (" v pData->fElevation : %.2f (x%.2f)\n", pData->fElevation, pData->fFlattenFactor);
	}
	
	pData->iBounceCount --;  // c'est une loi de type acceleration dans le champ de pesanteur. 'g' et 'v0' n'interviennent pas directement, car s'expriment en fonction de 'fPossibleDeltaY' et 'n'.
	
	if (! bUseOpenGL && ! pDock->bIsShrinkingDown && ! pDock->bIsGrowingUp)
	{
		double fDamageWidthFactor = pIcon->fWidthFactor;
		double fDamageHeightFactor = pIcon->fHeightFactor;
		double fDeltaYReflection = pIcon->fDeltaYReflection;
		
		fPrevDeltaY = MAX (fPrevDeltaY, pIcon->fDeltaYReflection);
		pIcon->fDeltaYReflection = fPrevDeltaY;
		fPrevElevation = MAX (fPrevElevation, pData->fElevation);
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		pIcon->fDrawY -= (pDock->bDirectionUp ? 1 : 0) * fPrevElevation;
		pIcon->fHeight += fPrevElevation;
		
		cairo_dock_redraw_icon (pIcon, CAIRO_CONTAINER (pDock));
		//cairo_dock_redraw_container (pDock);
		pIcon->fDrawY += (pDock->bDirectionUp ? 1 : 0) * fPrevElevation;
		pIcon->fWidthFactor = fDamageWidthFactor;
		pIcon->fHeightFactor = fDamageHeightFactor;
		pIcon->fDeltaYReflection = fDeltaYReflection;
		pIcon->fHeight -= fPrevElevation;
	}
	
	return (pData->iBounceCount > 0);
}


void cd_animations_draw_bounce_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, int sens)
{
	if (sens == 1)
		pIcon->fHeightFactor *= pData->fFlattenFactor;
	else
		pIcon->fHeightFactor /= pData->fFlattenFactor;
	
	if (sens == 1)
	{
		pIcon->fHeightFactor *= pData->fResizeFactor;
		pIcon->fWidthFactor *= pData->fResizeFactor;
	}
	else
	{
		pIcon->fHeightFactor /= pData->fResizeFactor;
		pIcon->fWidthFactor /= pData->fResizeFactor;
	}
	
	if (pDock->bHorizontalDock)
		glTranslatef (0., (pDock->bDirectionUp ? 1 : -1) * pData->fElevation * sens, 0.);
	else
		glTranslatef ((pDock->bDirectionUp ? -1 : 1) * pData->fElevation * sens, 0., 0.);
}


void cd_animations_draw_bounce_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext, int sens)
{
	if (sens == 1)
		pIcon->fHeightFactor *= pData->fFlattenFactor;
	else
		pIcon->fHeightFactor /= pData->fFlattenFactor;
	
	if (sens == 1)
	{
		pIcon->fHeightFactor *= pData->fResizeFactor;
		pIcon->fWidthFactor *= pData->fResizeFactor;
	}
	else
	{
		pIcon->fHeightFactor /= pData->fResizeFactor;
		pIcon->fWidthFactor /= pData->fResizeFactor;
	}
	
	if (pDock->bHorizontalDock)
		cairo_translate (pCairoContext,
			pIcon->fWidth * pIcon->fScale * (1 - pIcon->fWidthFactor) / 2 * sens,
			(pDock->bDirectionUp ? 1 : 0) * pIcon->fHeight * pIcon->fScale * (1 - pIcon->fHeightFactor) / 2 * sens);
	else
		cairo_translate (pCairoContext,
			(pDock->bDirectionUp ? 1 : 0) * pIcon->fHeight * pIcon->fScale * (1 - pIcon->fHeightFactor) / 2 * sens,
			pIcon->fWidth * pIcon->fScale * (1 - pIcon->fWidthFactor) / 2 * sens);
	
	if (pDock->bHorizontalDock)
		cairo_translate (pCairoContext,
			0.,
			- (pDock->bDirectionUp ? 1 : -1) * pData->fElevation * sens);
	else
		cairo_translate (pCairoContext,
			- (pDock->bDirectionUp ? 1 : -1) * pData->fElevation * sens,
			0.);
}

