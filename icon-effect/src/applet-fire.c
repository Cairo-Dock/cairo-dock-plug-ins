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
	double fMaxScale = cairo_dock_get_max_scale (CAIRO_CONTAINER (pDock));
	CairoParticleSystem *pFireParticleSystem = cairo_dock_create_particle_system (myConfig.iNbFireParticles, myData.iFireTexture, pIcon->fWidth, pIcon->fHeight * fMaxScale);
	pFireParticleSystem->dt = dt;
	
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
		//if (fabs (p->x) > .75)  // bof ...
		//	p->x = (4 * fabs (p->x) - 3) * (p->x > 0 ? 1 : -1);
		p->y = 0.;
		p->z = 2 * g_random_double () - 1;
		p->fWidth = r*(p->z + 2)/3 * g_random_double ();
		p->fHeight = p->fWidth;
		
		p->vx = 0.;
		p->vy = a * vmax * ((p->z + 1)/2 * g_random_double () + epsilon) * dt;
		p->iInitialLife = MIN (1./ p->vy, ceil (myConfig.iFireDuration / dt));
		p->iLife = p->iInitialLife;
		
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
			//fBlend = g_random_double ();
			p->color[1] = fBlend * myConfig.pFireColor1[1] + (1 - fBlend) * myConfig.pFireColor2[1];
			//fBlend = g_random_double ();
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



void cd_icon_effect_rewind_fire_particle (CairoParticle *p, double dt)
{
	static double epsilon = 0.1;
	double a = myConfig.fFireParticleSpeed/3;
	double r = myConfig.iFireParticleSize;
	double vmax = 1. / myConfig.iFireDuration;
	p->x = 2 * g_random_double () - 1;
	//if (fabs (p->x) > .75)  // bof ...
	//	p->x = (4 * fabs (p->x) - 3) * (p->x > 0 ? 1 : -1);
	p->x = p->x * p->x * (p->x > 0 ? 1 : -1);
	p->y = 0;
	p->z = 2 * g_random_double () - 1;
	p->fWidth = r*(p->z + 2)/3 * g_random_double ();
	p->fHeight = p->fWidth;
	p->vy = a * vmax * ((p->z + 1)/2 * g_random_double () + epsilon) * dt;
	
	p->iInitialLife = MIN (1./ p->vy, ceil (myConfig.iFireDuration / dt));
	p->iLife = p->iInitialLife;
	
	p->fSizeFactor = 1.;
}

