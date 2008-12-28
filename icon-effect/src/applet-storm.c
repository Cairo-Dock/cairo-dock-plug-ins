/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-storm.h"

static double ar = .1;  // variation du rayon des particules.
static double ad = .5;  // dispersion.
static double at = .6;  // transparence initiale des particules.
static double n = 2;  // nbre de tours.

CairoParticleSystem *cd_icon_effect_init_storm (Icon *pIcon, CairoDock *pDock, double dt)
{
	if (myData.iFireTexture == 0)
		myData.iFireTexture = cd_icon_effect_load_storm_texture ();
	double fMaxScale = (pDock->bAtBottom ? 1. : cairo_dock_get_max_scale (CAIRO_CONTAINER (pDock)));
	CairoParticleSystem *pStormParticleSystem = cairo_dock_create_particle_system (myConfig.iNbStormParticles, myData.iFireTexture, pIcon->fWidth * pIcon->fScale, pIcon->fHeight * fMaxScale);
	g_return_val_if_fail (pStormParticleSystem != NULL, NULL);
	pStormParticleSystem->dt = dt;
	if (myConfig.bRotateEffects && ! pDock->bDirectionUp && pDock->bHorizontalDock)
		pStormParticleSystem->bDirectionUp = FALSE;
	
	static double epsilon = 0.1;
	double r = myConfig.iStormParticleSize;
	double vmax = 1. / myConfig.iStormDuration * 2;
	double fBlend;
	CairoParticle *p;
	int i;
	for (i = 0; i < myConfig.iNbStormParticles; i ++)
	{
		p = &pStormParticleSystem->pParticles[i];
		
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
	
	return pStormParticleSystem;
}

gboolean cd_icon_effect_update_storm_system (CairoParticleSystem *pParticleSystem, CairoDockRewindParticleFunc pRewindParticle)
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

void cd_icon_effect_rewind_storm_particle (CairoParticle *p, double dt)
{
	p->x = 0;
	p->y = .03 * (2 * g_random_double () - 1);
	p->z = 1.;
	p->fSizeFactor = 1.;
	p->color[3] = at;
	p->iInitialLife = MIN (1. / p->vy, ceil (myConfig.iStormDuration/2 / dt));
	p->iLife = p->iInitialLife;
}
