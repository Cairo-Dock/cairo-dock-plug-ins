/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-rays.h"
#include "applet-spot.h"

void cd_animations_init_spot (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt)
{
	if (myData.iSpotTexture == 0)
		myData.iSpotTexture = cd_animation_load_spot_texture ();
	if (myData.iHaloTexture == 0)
		myData.iHaloTexture = cd_animation_load_halo_texture ();
	if (myData.iSpotFrontTexture == 0)
		myData.iSpotFrontTexture = cd_animation_load_spot_front_texture ();
	if (myData.iRaysTexture == 0)
		myData.iRaysTexture = cd_animations_load_rays_texture ();
	if (pData->pRaysSystem == NULL && myConfig.iNbRaysParticles != 0)
		pData->pRaysSystem = cd_animations_init_rays (pIcon, pDock, dt);
	pData->fRadiusFactor = .001;
	pData->fHaloRotationAngle = 0;
}


void cd_animation_render_spot (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor)
{
	glPushMatrix ();
	//\___________________On se place au bas de l'icone.
	if (! pDock->bHorizontalDock)
		glRotatef (90, 0., 0., 1.);
	double fY = (- pIcon->fHeight + CD_ANIMATIONS_SPOT_HEIGHT * fRadiusFactor) * pIcon->fScale/2;
	if (pDock->bUseReflect)
		fY -= MIN (myIcons.fReflectSize, CD_ANIMATIONS_SPOT_HEIGHT/2);
	if (! pDock->bDirectionUp)
		fY = -fY;
	glTranslatef (0., fY, 0.);
	if (! pDock->bDirectionUp)
		glScalef (1., -1., 1.);
	
	glColor4f (myConfig.pSpotColor[0], myConfig.pSpotColor[1], myConfig.pSpotColor[2], pIcon->fAlpha);
	cairo_dock_draw_texture (myData.iSpotTexture, fRadiusFactor * pIcon->fWidth * pIcon->fScale, fRadiusFactor * CD_ANIMATIONS_SPOT_HEIGHT * pIcon->fScale);
	
	glPopMatrix ();
}

void cd_animation_render_halo (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor, int fHaloRotationAngle)
{
	glPushMatrix ();
	
	//\___________________On se place au bas de l'icone.
	if (! pDock->bHorizontalDock)
		glRotatef (90, 0., 0., 1.);
	double fY = fRadiusFactor * CD_ANIMATIONS_SPOT_HEIGHT * (1 + cos (G_PI * fHaloRotationAngle / 180.))/2 - pIcon->fHeight * pIcon->fScale/2;
	if (pDock->bUseReflect)
		fY -= MIN (myIcons.fReflectSize, CD_ANIMATIONS_SPOT_HEIGHT/2);
	if (! pDock->bDirectionUp)
		fY = -fY;
	double fX = .9 * fRadiusFactor * pIcon->fWidth * pIcon->fScale/2;
	
	glRotatef (fHaloRotationAngle, 0., 1., 0.);
	glTranslatef (0., fY, fX);
	if (! pDock->bDirectionUp)
		glScalef (1., -1., 1.);
	
	glColor4f (myConfig.pHaloColor[0], myConfig.pHaloColor[1], myConfig.pHaloColor[2], pIcon->fAlpha);
	cairo_dock_draw_texture (myData.iHaloTexture, pIcon->fWidth * pIcon->fScale*.25, 6);  // taille qui rend pas trop mal.
	
	glPopMatrix ();
}

void cd_animation_render_spot_front (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor)
{
	glPushMatrix ();
	//\___________________On se place au-dessus du spot.
	if (! pDock->bHorizontalDock)
		glRotatef (90, 0., 0., 1.);
	double fY = (- pIcon->fHeight + CD_ANIMATIONS_SPOT_HEIGHT/2 * fRadiusFactor + pIcon->fHeight * fRadiusFactor) * pIcon->fScale/2;
	if (pDock->bUseReflect)
		fY -= MIN (myIcons.fReflectSize, CD_ANIMATIONS_SPOT_HEIGHT/2);
	if (! pDock->bDirectionUp)
		fY = -fY;
	glTranslatef (0., fY, 0.);
	if (! pDock->bDirectionUp)
		glScalef (1., -1., 1.);
	
	glColor4f (myConfig.pSpotColor[0], myConfig.pSpotColor[1], myConfig.pSpotColor[2], pIcon->fAlpha);
	cairo_dock_draw_texture (myData.iSpotFrontTexture, fRadiusFactor * pIcon->fWidth * pIcon->fScale, fRadiusFactor * pIcon->fHeight * pIcon->fScale);
	
	glPopMatrix ();
}
