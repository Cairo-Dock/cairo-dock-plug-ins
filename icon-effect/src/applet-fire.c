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
#include "applet-fire.h"


static gboolean init (Icon *pIcon, CairoDock *pDock, double dt, CDIconEffectData *pData)
{
	if (pData->pFireSystem != NULL)
		return TRUE;
	
	if (myData.iFireTexture == 0)
		myData.iFireTexture = cd_icon_effect_load_fire_texture ();
	
	double fMaxScale = 1. + g_fAmplitude * pDock->fMagnitudeMax;
	CairoParticleSystem *pParticleSystem = cairo_dock_create_particle_system (myConfig.iNbFireParticles, myData.iFireTexture, pIcon->fWidth * pIcon->fScale, pIcon->fHeight * fMaxScale);
	g_return_val_if_fail (pParticleSystem != NULL, FALSE);
	pParticleSystem->dt = dt;
	if (myConfig.bRotateEffects && ! pDock->container.bDirectionUp && pDock->container.bIsHorizontal)
		pParticleSystem->bDirectionUp = FALSE;
	pParticleSystem->bAddLuminance = myConfig.bFireLuminance;
	
	double a = myConfig.fFireParticleSpeed;
	static double epsilon = 0.1;
	double r = myConfig.iFireParticleSize;
	double fBlend;
	double vmax = 1. / myConfig.iFireDuration;
	CairoParticle *p;
	int i;
	for (i = 0; i < myConfig.iNbFireParticles; i ++)
	{
		p = &(pParticleSystem->pParticles[i]);
		
		p->x = 2 * g_random_double () - 1;
		p->x = p->x * p->x * (p->x > 0 ? 1 : -1);
		p->y = 0.;
		p->z = 2 * g_random_double () - 1;
		p->fWidth = r*(p->z + 2)/3 * .5 * pDock->container.fRatio;
		p->fHeight = p->fWidth;
		
		p->vx = 0.;
		p->vy = a * vmax * ((p->z + 1)/2 * 1. + epsilon) * dt;
		p->iInitialLife = MIN (1./ p->vy, ceil (myConfig.iFireDuration / dt));
		p->iLife = p->iInitialLife * (.8+.3*g_random_double ());  // dispersion entre .8 et 1.1
		
		if (myConfig.bMysticalFire)
		{
			p->color[0] = g_random_double ();
			p->color[1] = g_random_double ();
			p->color[2] = g_random_double ();
		}
		else
		{
			fBlend = g_random_double ();
			p->color[0] = fBlend * myConfig.pFireColor1[0] + (1 - fBlend) * myConfig.pFireColor2[0];
			p->color[1] = fBlend * myConfig.pFireColor1[1] + (1 - fBlend) * myConfig.pFireColor2[1];
			p->color[2] = fBlend * myConfig.pFireColor1[2] + (1 - fBlend) * myConfig.pFireColor2[2];
		}
		p->color[3] = 1.;
		
		p->fOscillation = G_PI * (2 * g_random_double () - 1);
		p->fOmega = 2*G_PI / myConfig.iFireDuration * dt;  // tr/s
		
		p->fSizeFactor = 1.;
		p->fResizeSpeed = -.5 / myConfig.iFireDuration * dt;  // zoom 0.5 a la fin.
	}
	
	pData->pFireSystem = pParticleSystem;
	return TRUE;
}


gboolean _update_fire_system (CairoParticleSystem *pParticleSystem, CairoDockRewindParticleFunc pRewindParticle)
{
	gboolean bAllParticlesEnded = TRUE;
	CairoParticle *p;
	int i;
	for (i = 0; i < pParticleSystem->iNbParticles; i ++)
	{
		p = &(pParticleSystem->pParticles[i]);
		
		p->fOscillation += p->fOmega;
		p->x += p->vx + (p->z + 2)/3. * .02 * sin (p->fOscillation);  // 2%
		p->y += p->vy;
		p->color[3] = .8*p->iLife / p->iInitialLife;
		p->fSizeFactor += p->fResizeSpeed;
		if (p->iLife > 0)
		{
			p->iLife --;
			if (pRewindParticle && p->iLife == 0)
			{
				pRewindParticle (p, pParticleSystem->dt);
			}
			if (bAllParticlesEnded && p->iLife != 0)
				bAllParticlesEnded = FALSE;
		}
		else if (pRewindParticle)
			pRewindParticle (p, pParticleSystem->dt);
	}
	return ! bAllParticlesEnded;
}

static void _rewind_fire_particle (CairoParticle *p, double dt)
{
	static double epsilon = 0.1;
	double a = myConfig.fFireParticleSpeed/myConfig.fFireParticleSpeed;
	double r = myConfig.iFireParticleSize;
	double vmax = 1. / myConfig.iFireDuration;
	p->x = 2 * g_random_double () - 1;
	p->x = p->x * p->x * (p->x > 0 ? 1 : -1);
	p->y = 0;
	p->z = 2 * g_random_double () - 1;
	p->fWidth = r*(p->z + 2)/3 * .5;
	p->fHeight = p->fWidth;
	p->vy = a * vmax * ((p->z + 1)/2 * 1. + epsilon) * dt;
	
	p->iInitialLife = MIN (1./ p->vy, ceil (myConfig.iFireDuration / dt));
	p->iLife = p->iInitialLife * (.9+.2*g_random_double ());  // dispersion entre .9 et 1.1
	
	p->fSizeFactor = 1.;
	p->color[3] = 1.;
}

static gboolean update (Icon *pIcon, CairoDock *pDock, gboolean bRepeat, CDIconEffectData *pData)
{
	if (pData->pFireSystem == NULL)
		return FALSE;
		
	gboolean bContinue = _update_fire_system (pData->pFireSystem,
		(bRepeat ? _rewind_fire_particle : NULL));
	
	pData->pFireSystem->fWidth = pIcon->fWidth * pIcon->fScale;
	
	double fMaxScale = 1. + g_fAmplitude * pDock->fMagnitudeMax;
	pData->fAreaWidth = (1. + .02) * pData->pFireSystem->fWidth + myConfig.iFireParticleSize * pDock->container.fRatio;  // 2% d'oscillation + demi-largeur des particules a droite et a gauche.
	pData->fAreaHeight = pIcon->fHeight * fMaxScale + myConfig.iFireParticleSize * pDock->container.fRatio;
	pData->fBottomGap = myConfig.iFireParticleSize * pDock->container.fRatio / 2;
	return bContinue;
}


static void render (CDIconEffectData *pData)
{
	if (pData->pFireSystem == NULL)
		return ;
	
	cairo_dock_render_particles (pData->pFireSystem);
}


static void free_effect (CDIconEffectData *pData)
{
	if (pData->pFireSystem != NULL)
	{
		cairo_dock_free_particle_system (pData->pFireSystem);
		pData->pFireSystem = NULL;
	}
}


void cd_icon_effect_register_fire (CDIconEffect *pEffect)
{
	pEffect->init = init;
	pEffect->update = update;
	pEffect->render = render;
	pEffect->free = free_effect;
}
