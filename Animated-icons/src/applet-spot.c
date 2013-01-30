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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-rays.h"
#include "applet-notifications.h"
#include "applet-spot.h"


static void init (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL)
{
	if (myData.iSpotTexture == 0)
		myData.iSpotTexture = cd_animation_load_spot_texture ();
	if (myData.iHaloTexture == 0)
		myData.iHaloTexture = cd_animation_load_halo_texture ();
	if (myData.iSpotFrontTexture == 0 && myConfig.cSpotFrontImage != NULL)
		myData.iSpotFrontTexture = cd_animation_load_spot_front_texture ();
	if (myData.iRaysTexture == 0)
		myData.iRaysTexture = cd_animations_load_rays_texture ();
	if (pData->pRaysSystem == NULL && myConfig.iNbRaysParticles != 0)
		pData->pRaysSystem = cd_animations_init_rays (pIcon, pDock, dt);
	pData->fRadiusFactor = .001;
	pData->fHaloRotationAngle = 0;
	pData->bGrowingSpot = TRUE;
}

static void _cd_animations_render_rays (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, int iDepth)
{
	glPushMatrix ();
	if (pDock->container.bIsHorizontal)
		glTranslatef (0., - pIcon->fHeight * pIcon->fScale/2, 0.);
	else
		glTranslatef (- pIcon->fHeight * pIcon->fScale/2, 0., 0.);
	
	if (! pDock->container.bIsHorizontal)
		glRotatef (-90, 0., 0., 1.);
	
	if (pData->pRaysSystem != NULL)
	{
		cairo_dock_render_particles_full (pData->pRaysSystem, iDepth);
	}

	glPopMatrix ();
}

static void cd_animation_render_spot (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor)
{
	glPushMatrix ();
	//\___________________On se place au bas de l'icone.
	if (! pDock->container.bIsHorizontal)
		glRotatef (90, 0., 0., 1.);
	double fY = (- pIcon->fHeight + CD_ANIMATIONS_SPOT_HEIGHT) * pIcon->fScale/2;  // * fRadiusFactor
	if (pDock->container.bUseReflect)
		fY -= MIN (pDock->iIconSize * myIconsParam.fReflectHeightRatio, CD_ANIMATIONS_SPOT_HEIGHT/2);
	if (! pDock->container.bDirectionUp)
		fY = -fY;
	glTranslatef (0., fY, 0.);
	if (! pDock->container.bDirectionUp)
		glScalef (1., -1., 1.);
	
	glColor4f (myConfig.pSpotColor[0], myConfig.pSpotColor[1], myConfig.pSpotColor[2], .9 * fRadiusFactor * pIcon->fAlpha);
	//cairo_dock_draw_texture (myData.iSpotTexture, fRadiusFactor * pIcon->fWidth * pIcon->fScale, fRadiusFactor * CD_ANIMATIONS_SPOT_HEIGHT * pIcon->fScale);
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	
	_cairo_dock_apply_texture_at_size (myData.iSpotTexture,
		pIcon->fWidth * pIcon->fScale,
		CD_ANIMATIONS_SPOT_HEIGHT * pIcon->fScale);
	
	_cairo_dock_disable_texture ();
	
	glPopMatrix ();
}

static void cd_animation_render_halo (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor, int fHaloRotationAngle)
{
	glPushMatrix ();
	
	//\___________________On se place au bas de l'icone.
	if (! pDock->container.bIsHorizontal)
		glRotatef (90, 0., 0., 1.);
	double fY = CD_ANIMATIONS_SPOT_HEIGHT * (1 + cos (G_PI * fHaloRotationAngle / 180.))/2 - pIcon->fHeight * pIcon->fScale/2;  // * fRadiusFactor
	if (pDock->container.bUseReflect)
		fY -= MIN (pDock->iIconSize * myIconsParam.fReflectHeightRatio, CD_ANIMATIONS_SPOT_HEIGHT/2);
	if (! pDock->container.bDirectionUp)
		fY = -fY;
	double fX = .9 * pIcon->fWidth * pIcon->fScale/2;  // * fRadiusFactor
	
	glRotatef (fHaloRotationAngle, 0., 1., 0.);
	glTranslatef (0., fY, fX);
	if (! pDock->container.bDirectionUp)
		glScalef (1., -1., 1.);
	
	glColor4f (myConfig.pHaloColor[0], myConfig.pHaloColor[1], myConfig.pHaloColor[2], pIcon->fAlpha * fRadiusFactor);
	//cairo_dock_draw_texture (myData.iHaloTexture, pIcon->fWidth * pIcon->fScale*.25, 6);  // taille qui rend pas trop mal.
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_over ();
	
	_cairo_dock_apply_texture_at_size (myData.iHaloTexture,
		pIcon->fWidth * pIcon->fScale * .25,
		6.);  // taille qui rend pas trop mal.
	
	_cairo_dock_disable_texture ();
	
	glPopMatrix ();
}

static void cd_animation_render_spot_front (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor)
{
	if (myData.iSpotFrontTexture == 0)
		return ;
	glPushMatrix ();
	//\___________________On se place au-dessus du spot.
	if (! pDock->container.bIsHorizontal)
		glRotatef (90, 0., 0., 1.);
	double fY = (- pIcon->fHeight + CD_ANIMATIONS_SPOT_HEIGHT/2 + pIcon->fHeight * fRadiusFactor) * pIcon->fScale/2;  // CD_ANIMATIONS_SPOT_HEIGHT/2 * fRadiusFactor
	if (pDock->container.bUseReflect)
		fY -= MIN (pDock->iIconSize * myIconsParam.fReflectHeightRatio, CD_ANIMATIONS_SPOT_HEIGHT/2);
	if (! pDock->container.bDirectionUp)
		fY = -fY;
	glTranslatef (0., fY, 0.);
	if (! pDock->container.bDirectionUp)
		glScalef (1., -1., 1.);
	
	glColor4f (myConfig.pSpotColor[0], myConfig.pSpotColor[1], myConfig.pSpotColor[2], pIcon->fAlpha);
	//cairo_dock_draw_texture (myData.iSpotFrontTexture, fRadiusFactor * pIcon->fWidth * pIcon->fScale, fRadiusFactor * pIcon->fHeight * pIcon->fScale);
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_over ();
		
	glBindTexture (GL_TEXTURE_2D, myData.iSpotFrontTexture);
	_cairo_dock_apply_current_texture_portion_at_size_with_offset (0., 0.,
		1., fRadiusFactor,
		pIcon->fWidth * pIcon->fScale, pIcon->fHeight * pIcon->fScale * fRadiusFactor,  // * fRadiusFactor
		0., 0.);
	_cairo_dock_disable_texture ();
	
	glPopMatrix ();
}


static void render (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	if (pCairoContext == NULL)  // no cairo rendering (yet).
	{
		cd_animation_render_spot (pIcon, pDock, pData->fRadiusFactor);
		if (pData->fHaloRotationAngle <= 90 || pData->fHaloRotationAngle >= 270)
			cd_animation_render_halo (pIcon, pDock, pData->fRadiusFactor, pData->fHaloRotationAngle);
		
		if (pData->pRaysSystem != NULL)
			_cd_animations_render_rays (pIcon, pDock, pData, 1);
		
		if (pDock->container.bIsHorizontal)
			glTranslatef (0., pData->fIconOffsetY * (pDock->container.bDirectionUp ? 1 : -1), 0.);
		else
			glTranslatef (pData->fIconOffsetY * (pDock->container.bDirectionUp ? -1 : 1), 0., 0.);
	}
}

static void post_render (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	if (pCairoContext == NULL)  // no cairo rendering (yet).
	{
		if (pDock->container.bIsHorizontal)
			glTranslatef (0., - pData->fIconOffsetY * (pDock->container.bDirectionUp ? 1 : -1), 0.);
		else
			glTranslatef (- pData->fIconOffsetY * (pDock->container.bDirectionUp ? -1 : 1), 0., 0.);
		if (pData->pRaysSystem != NULL)
			_cd_animations_render_rays (pIcon, pDock, pData, 1);
		
		cd_animation_render_spot_front (pIcon, pDock, pData->fRadiusFactor);
		if (pData->fHaloRotationAngle > 90 && pData->fHaloRotationAngle < 270)
			cd_animation_render_halo (pIcon, pDock, pData->fRadiusFactor, pData->fHaloRotationAngle);
	}
}

static gboolean update (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL, gboolean bRepeat)
{
	if (!bUseOpenGL)  // no cairo rendering (yet).
		return FALSE;

	gboolean bContinueAnimation = bRepeat;
	if (pData->bGrowingSpot)
	{
		bContinueAnimation = TRUE;
		pData->fRadiusFactor += 1./myConfig.iSpotDuration * dt;
		if (pData->fRadiusFactor > 1)
		{
			pData->fRadiusFactor = 1.;
			if (!bRepeat)
				pData->bGrowingSpot = FALSE;
		}
		pData->fIconOffsetY += 1.*myIconsParam.iconTextDescription.iSize / myConfig.iSpotDuration * dt;
		if (pData->fIconOffsetY > myIconsParam.iconTextDescription.iSize)
			pData->fIconOffsetY = myIconsParam.iconTextDescription.iSize;
	}
	else
	{
		pData->fRadiusFactor -= 1./myConfig.iSpotDuration * dt;
		if (pData->fRadiusFactor < 0)
			pData->fRadiusFactor = 0.;
		else
			bContinueAnimation = TRUE;
		pData->fIconOffsetY -= 1.*myIconsParam.iconTextDescription.iSize / myConfig.iSpotDuration * dt;
		if (pData->fIconOffsetY < 0)
			pData->fIconOffsetY = 0.;
		else
			bContinueAnimation = TRUE;
	}
	///pData->fIconOffsetY *= cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	pIcon->fDeltaYReflection += 2 * pData->fIconOffsetY;
	
	pData->fHaloRotationAngle += 360. / myConfig.iSpotDuration * dt;
	
	if (pData->pRaysSystem != NULL)
	{
		gboolean bContinueSpot = cd_animations_update_rays_system (pData->pRaysSystem, bRepeat);
		pData->pRaysSystem->fWidth = pIcon->fWidth * pIcon->fScale * pData->fRadiusFactor;
		if (bContinueSpot)
			bContinueAnimation = TRUE;
		else
		{
			cairo_dock_free_particle_system (pData->pRaysSystem);
			pData->pRaysSystem = NULL;
		}
	}
	
	cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));
	
	if (pData->fHaloRotationAngle > 360)  // un tour est passe.
	{
		pData->fHaloRotationAngle -= 360;
		if (pData->iNumRound > 0)
		{
			pData->iNumRound --;
		}
	}
	
	return bContinueAnimation;
}


void cd_animations_register_spot (void)
{
	CDAnimation *pAnimation = &myData.pAnimations[CD_ANIMATIONS_SPOT];
	pAnimation->cName = "spot";
	pAnimation->cDisplayedName = D_("Spot");
	pAnimation->id = CD_ANIMATIONS_SPOT;
	pAnimation->bDrawIcon = FALSE;
	pAnimation->bDrawReflect = FALSE;
	pAnimation->init = init;
	pAnimation->update = update;
	pAnimation->render = render;
	pAnimation->post_render = post_render;
	cd_animations_register_animation (pAnimation);
}
