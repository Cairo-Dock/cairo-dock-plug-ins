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



gboolean cd_animations_update_bounce (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, gboolean bUseOpenGL)
{
	int n = 2*7+3;  // nbre d'iteration pour 1 aplatissement+montree+descente.
	int k = n - (pData->iBounceCount % n) - 3;  // 3 iterations pour s'aplatir.
	n -= 3;   // nbre d'iteration pour 1 montree+descente.

	if (k > 0)
	{
		double fPossibleDeltaY = MIN (100, (pDock->bDirectionUp ? icon->fDrawY : pDock->iCurrentHeight - (icon->fDrawY + icon->fHeight * icon->fScale)));  // on borne a 100 pixels pour les rendus qui ont des fenetres grandes..
		
		pData->fElevation = (pDock->bDirectionUp ? -1. : 1.) * k / (n/2) * fPossibleDeltaY * (2 - 1.*k/(n/2));
		icon->fDeltaYReflection = 1.25 * pData->fElevation;  // le reflet "rebondira" de 25% de la hauteur au sol.
		//g_print ("%d) + %.2f (%d)\n", icon->iCount, (pDock->bDirectionUp ? -1. : 1.) * k / (n/2) * fPossibleDeltaY * (2 - 1.*k/(n/2)), k);
	}
	else  // on commence par s'aplatir.
	{
		pData->fFlattenFactor *= 1.*(2 - 1.5*k) / 10;
		pData->fElevation = (pDock->bDirectionUp ? 1 : -1) * (1 - icon->fHeightFactor) / 2 * icon->fHeight * icon->fScale;
		icon->fDeltaYReflection = pData->fElevation;
		//g_print ("%d) * %.2f (%d)\n", icon->iCount, icon->fHeightFactor, k);
	}
	pData->iBounceCount --;  // c'est une loi de type acceleration dans le champ de pesanteur. 'g' et 'v0' n'interviennent pas directement, car s'expriment en fonction de 'fPossibleDeltaY' et 'n'.
	return (pData->iBounceCount > 0);
}


void cd_animations_draw_bounce_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData)
{
	
}


void cd_animations_draw_bounce_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	
}
