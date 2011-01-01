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
#include "applet-storm.h"

static double ar = .1;  // variation du rayon des particules.
static double ad = .5;  // dispersion.
static double at = .6;  // transparence initiale des particules.
static double n = 2;  // nbre de tours.

#define cd_icon_effect_load_storm_texture cd_icon_effect_load_fire_texture

static gboolean init (Icon *pIcon, CairoDock *pDock, double dt, CDIconEffectData *pData)
{
	if (pData->pStormSystem != NULL)
		return TRUE;
	
	if (myData.iFireTexture == 0)
		myData.iFireTexture = cd_icon_effect_load_storm_texture ();
	
	double fMaxScale = 1. + myIconsParam.fAmplitude * pDock->fMagnitudeMax;
	CairoParticleSystem *pParticleSystem = cairo_dock_create_particle_system (myConfig.iNbStormParticles, myData.iFireTexture, pIcon->fWidth * pIcon->fScale, pIcon->fHeight * fMaxScale);
	g_return_val_if_fail (pParticleSystem != NULL, FALSE);
	pParticleSystem->dt = dt;
	if (myConfig.bRotateEffects && ! pDock->container.bDirectionUp && pDock->container.bIsHorizontal)
		pParticleSystem->bDirectionUp = FALSE;
	
	static double epsilon = 0.1;
	double r = myConfig.iStormParticleSize;
	double vmax = 1. / myConfig.iStormDuration * 2;
	double fBlend;
	CairoParticle *p;
	int i;
	for (i = 0; i < myConfig.iNbStormParticles; i ++)
	{
		p = &pParticleSystem->pParticles[i];
		
		p->x = 0.;  // on le calculera a la main.
		p->y = -1. * i /myConfig.iNbStormParticles + .01 * (2 * g_random_double () - 1);
		p->z = 1.;  // idem.
		p->fWidth = r * (1 + ar * (2 * g_random_double () - 1));
		p->fHeight = p->fWidth;
		
		p->vx = ad * (2 * g_random_double () - 1);  // utilisation detournee : dispersion.
		p->vy = vmax * (1 - ad * g_random_double ()) * dt * 2;
		
		p->iInitialLife = MIN ((1 - p->y) / p->vy, ceil (myConfig.iStormDuration/2 / dt));
		p->iLife = p->iInitialLife;
		
		fBlend = g_random_double ();
		p->color[0] = fBlend * myConfig.pStormColor1[0] + (1 - fBlend) * myConfig.pStormColor2[0];
		p->color[1] = fBlend * myConfig.pStormColor1[1] + (1 - fBlend) * myConfig.pStormColor2[1];
		p->color[2] = fBlend * myConfig.pStormColor1[2] + (1 - fBlend) * myConfig.pStormColor2[2];
		p->color[3] = (p->y < 0 ? 0. : at);
		
		p->fOscillation = 0.;
		p->fOmega = 0.;
		
		p->fSizeFactor = 1.;
		p->fResizeSpeed = 0.;  // zoom constant.
	}
	
	pData->pStormSystem = pParticleSystem;
	return TRUE;
}

static gboolean _update_storm_system (CairoParticleSystem *pParticleSystem, CairoDockRewindParticleFunc pRewindParticle)
{
	gboolean bAllParticlesEnded = TRUE;
	double x;
	CairoParticle *p;
	int i;
	for (i = 0; i < pParticleSystem->iNbParticles; i ++)
	{
		p = &(pParticleSystem->pParticles[i]);
		
		p->y += p->vy;
		p->x = (1 + p->vx) * sin (p->y * 2 * n * G_PI);  // n tours.
		p->z = (1 + p->vx) * cos (p->y * 2 * n * G_PI);
		p->fSizeFactor = 1 - (1 - p->z)/2. * .33;  // 33% de variation.
		//p->color[3] = sqrt (MAX (0, p->y));
		p->color[3] = (p->y < 0 ? 0. : at * (0.1 + 1.*p->iLife / p->iInitialLife) / 1.1);  // on finit a 0.1 en haut, sinon on voit pas bien la derniere boucle.
		
		if (p->iLife > 0)  // p->y > 1
		{
			p->iLife --;
			if (pRewindParticle && p->iLife == 0)
			{
				pRewindParticle (p, pParticleSystem->dt);
			}
			if (bAllParticlesEnded && p->iLife != 0)
				bAllParticlesEnded = FALSE;
		}
	}
	return ! bAllParticlesEnded;
}

static void _rewind_storm_particle (CairoParticle *p, double dt)
{
	p->x = 0;
	p->y = .03 * (2 * g_random_double () - 1);
	p->z = 1.;
	p->fSizeFactor = 1.;
	p->color[3] = at;
	p->iInitialLife = MIN (1. / p->vy, ceil (myConfig.iStormDuration/2 / dt));
	p->iLife = p->iInitialLife;
}

static gboolean update (Icon *pIcon, CairoDock *pDock, gboolean bRepeat, CDIconEffectData *pData)
{
	if (pData->pStormSystem == NULL)
		return FALSE;
		
	gboolean bContinue = _update_storm_system (pData->pStormSystem,
		(bRepeat ? _rewind_storm_particle : NULL));
	pData->pStormSystem->fWidth = pIcon->fWidth * pIcon->fScale;
	
	double fMaxScale = 1. + myIconsParam.fAmplitude * pDock->fMagnitudeMax;
	pData->fAreaWidth = pData->pStormSystem->fWidth * (1 + ad) + myConfig.iStormParticleSize * pDock->container.fRatio;  // dispersion + demi-largeur des particules a droite et a gauche.
	pData->fAreaHeight = pIcon->fHeight * fMaxScale + myConfig.iStormParticleSize * pDock->container.fRatio;
	pData->fBottomGap = myConfig.iStormParticleSize * pDock->container.fRatio / 2;
	
	return bContinue;
}


static void render (CDIconEffectData *pData)
{
	if (pData->pStormSystem == NULL)
		return ;
	
	cairo_dock_render_particles_full (pData->pStormSystem, -1);
}


static void post_render (CDIconEffectData *pData)
{
	if (pData->pStormSystem == NULL)
		return ;
	
	cairo_dock_render_particles_full (pData->pStormSystem, 1);
}


static void free_effect (CDIconEffectData *pData)
{
	if (pData->pStormSystem != NULL)
	{
		cairo_dock_free_particle_system (pData->pStormSystem);
		pData->pStormSystem = NULL;
	}
}


void cd_icon_effect_register_storm (CDIconEffect *pEffect)
{
	pEffect->init = init;
	pEffect->update = update;
	pEffect->render = render;
	pEffect->post_render = post_render;
	pEffect->free = free_effect;
}
