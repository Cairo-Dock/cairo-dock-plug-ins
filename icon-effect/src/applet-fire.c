/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-fire.h"


CairoParticleSystem *cd_icon_effect_init_fire (Icon *pIcon, CairoDock *pDock, double dt)
{
	if (myData.iFireTexture == 0)
		myData.iFireTexture = cd_icon_effect_load_fire_texture ();
	double fMaxScale = (pDock->bAtBottom ? 1. : cairo_dock_get_max_scale (CAIRO_CONTAINER (pDock)));
	CairoParticleSystem *pFireParticleSystem = cairo_dock_create_particle_system (myConfig.iNbFireParticles, myData.iFireTexture, pIcon->fWidth * pIcon->fScale, pIcon->fHeight * fMaxScale);
	g_return_val_if_fail (pFireParticleSystem != NULL, NULL);
	pFireParticleSystem->dt = dt;
	if (myConfig.bRotateEffects && ! pDock->bDirectionUp && pDock->bHorizontalDock)
		pFireParticleSystem->bDirectionUp = FALSE;
	
	double a = myConfig.fFireParticleSpeed;
	static double epsilon = 0.1;
	double r = myConfig.iFireParticleSize;
	double fBlend;
	double vmax = 1. / myConfig.iFireDuration;
	CairoParticle *p;
	int i;
	for (i = 0; i < myConfig.iNbFireParticles; i ++)
	{
		p = &(pFireParticleSystem->pParticles[i]);
		
		p->x = 2 * g_random_double () - 1;
		p->x = p->x * p->x * (p->x > 0 ? 1 : -1);
		p->y = 0.;
		p->z = 2 * g_random_double () - 1;
		p->fWidth = r*(p->z + 2)/3 * .5;
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
	
	return pFireParticleSystem;
}


gboolean cd_icon_effect_update_fire_system (CairoParticleSystem *pParticleSystem, CairoDockRewindParticleFunc pRewindParticle)
{
	gboolean bAllParticlesEnded = TRUE;
	CairoParticle *p;
	int i;
	for (i = 0; i < pParticleSystem->iNbParticles; i ++)
	{
		p = &(pParticleSystem->pParticles[i]);
		
		p->fOscillation += p->fOmega;
		p->x += p->vx + (p->z + 2)/3. * .02 * sin (p->fOscillation);  // 3%
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

void cd_icon_effect_rewind_fire_particle (CairoParticle *p, double dt)
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

