/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-rain.h"


CairoParticleSystem *cd_icon_effect_init_rain (Icon *pIcon, CairoDock *pDock, double dt)
{
	if (myData.iRainTexture == 0)
		myData.iRainTexture = cd_icon_effect_load_rain_texture ();
	double fMaxScale = pIcon->fScale;  // cairo_dock_get_max_scale (CAIRO_CONTAINER (pDock));
	CairoParticleSystem *pRainParticleSystem = cairo_dock_create_particle_system (myConfig.iNbRainParticles, myData.iRainTexture, pIcon->fWidth, pIcon->fHeight * fMaxScale);
	g_return_val_if_fail (pRainParticleSystem != NULL, NULL);
	pRainParticleSystem->dt = dt;
	if (myConfig.bRotateEffects && ! pDock->bDirectionUp && pDock->bHorizontalDock)
		pRainParticleSystem->bDirectionUp = FALSE;
	
	double a = myConfig.fRainParticleSpeed;
	static double epsilon = 0.1;
	double r = myConfig.iRainParticleSize;
	double fBlend;
	double vmax = 1. / myConfig.iRainDuration;
	CairoParticle *p;
	int i;
	for (i = 0; i < myConfig.iNbRainParticles; i ++)
	{
		p = &(pRainParticleSystem->pParticles[i]);
		
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
	
	return pRainParticleSystem;
}


void cd_icon_effect_rewind_rain_particle (CairoParticle *p, double dt)
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
