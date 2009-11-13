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
#include "applet-rain.h"


#define cd_icon_effect_load_rain_texture(...) CD_APPLET_LOAD_LOCAL_TEXTURE ("rain.png")

#define cd_icon_effect_update_rain_system cairo_dock_update_default_particle_system


static gboolean init (Icon *pIcon, CairoDock *pDock, double dt, CDIconEffectData *pData)
{
	if (pData->pRainSystem != NULL)
		return TRUE;
	
	if (myData.iRainTexture == 0)
		myData.iRainTexture = cd_icon_effect_load_rain_texture ();
	
	double fMaxScale = 1. + g_fAmplitude * pDock->fMagnitudeMax;
	CairoParticleSystem *pParticleSystem = cairo_dock_create_particle_system (myConfig.iNbRainParticles, myData.iRainTexture, pIcon->fWidth * pIcon->fScale, pIcon->fHeight * fMaxScale);
	g_return_val_if_fail (pParticleSystem != NULL, FALSE);
	pParticleSystem->dt = dt;
	if (myConfig.bRotateEffects && ! pDock->container.bDirectionUp && pDock->container.bIsHorizontal)
		pParticleSystem->bDirectionUp = FALSE;
	
	double a = myConfig.fRainParticleSpeed;
	static double epsilon = 0.1;
	double r = myConfig.iRainParticleSize;
	double fBlend;
	double vmax = 1. / myConfig.iRainDuration;
	CairoParticle *p;
	int i;
	for (i = 0; i < myConfig.iNbRainParticles; i ++)
	{
		p = &(pParticleSystem->pParticles[i]);
		
		p->x = 2 * g_random_double () - 1;
		p->y = 1.;
		p->z = 2 * g_random_double () - 1;
		p->fWidth = r*(p->z + 2)/3 * g_random_double ();
		p->fHeight = p->fWidth;
		
		p->vx = 0.;
		p->vy = -a * vmax * ((p->z + 1)/2 * g_random_double () + epsilon) * dt;
		p->iInitialLife = MIN (-1./ p->vy, ceil (myConfig.iRainDuration / dt));
		p->iLife = p->iInitialLife;
		
		{
			fBlend = g_random_double ();
			p->color[0] = fBlend * myConfig.pRainColor1[0] + (1 - fBlend) * myConfig.pRainColor2[0];
			p->color[1] = fBlend * myConfig.pRainColor1[1] + (1 - fBlend) * myConfig.pRainColor2[1];
			p->color[2] = fBlend * myConfig.pRainColor1[2] + (1 - fBlend) * myConfig.pRainColor2[2];
		}
		p->color[3] = 0.;
		
		p->fOscillation = 0.;
		p->fOmega = 0.;  // tr/s
		
		p->fSizeFactor = 1.;
		p->fResizeSpeed = 0.;  // zoom 1 a la fin.
	}
	
	pData->pRainSystem = pParticleSystem;
	return TRUE;
}


void _rewind_rain_particle (CairoParticle *p, double dt)
{
	static double epsilon = 0.1;
	double a = myConfig.fRainParticleSpeed/2;
	double r = myConfig.iRainParticleSize;
	double vmax = 1. / myConfig.iRainDuration;
	p->x = 2 * g_random_double () - 1;
	p->y = 1.;
	p->z = 2 * g_random_double () - 1;
	
	p->fWidth = r*(p->z + 2)/3 * g_random_double ();
	p->fHeight = p->fWidth;
	
	p->vy = -a * vmax * ((p->z + 1)/2 * g_random_double () + epsilon) * dt;
	
	p->iInitialLife = MIN (-1./ p->vy, ceil (myConfig.iRainDuration / dt));
	p->iLife = p->iInitialLife;
	
	p->fSizeFactor = 1.;
}


static gboolean update (Icon *pIcon, CairoDock *pDock, gboolean bRepeat, CDIconEffectData *pData)
{
	if (pData->pRainSystem == NULL)
		return FALSE;
		
	gboolean bContinue = cairo_dock_update_default_particle_system (pData->pRainSystem,
		(bRepeat ? _rewind_rain_particle : NULL));
	pData->pRainSystem->fWidth = pIcon->fWidth * pIcon->fScale;
	
	double fMaxScale = 1. + g_fAmplitude * pDock->fMagnitudeMax;
	pData->fAreaWidth = pData->pRainSystem->fWidth + myConfig.iRainParticleSize * pDock->container.fRatio;  // demi-largeur des particules a droite et a gauche.
	pData->fAreaHeight = pIcon->fHeight * fMaxScale + myConfig.iRainParticleSize/2 * pDock->container.fRatio;
	pData->fBottomGap = 0.;
	
	return bContinue;
}


static void render (CDIconEffectData *pData)
{
	if (pData->pRainSystem == NULL)
		return ;
	
	cairo_dock_render_particles (pData->pRainSystem);
}


static void free_effect (CDIconEffectData *pData)
{
	if (pData->pRainSystem != NULL)
	{
		cairo_dock_free_particle_system (pData->pRainSystem);
		pData->pRainSystem = NULL;
	}
}


void cd_icon_effect_register_rain (CDIconEffect *pEffect)
{
	pEffect->init = init;
	pEffect->update = update;
	pEffect->render = render;
	pEffect->free = free_effect;
}
