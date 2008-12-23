/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-snow.h"


CairoParticleSystem *cd_icon_effect_init_snow (Icon *pIcon, CairoDock *pDock, double dt)
{
	if (myData.iSnowTexture == 0)
		myData.iSnowTexture = cd_icon_effect_load_snow_texture ();
	double fMaxScale = cairo_dock_get_max_scale (CAIRO_CONTAINER (pDock));
	CairoParticleSystem *pSnowParticleSystem = cairo_dock_create_particle_system (myConfig.iNbSnowParticles, myData.iSnowTexture, pIcon->fWidth, pIcon->fHeight * fMaxScale);
	g_return_val_if_fail (pSnowParticleSystem != NULL, NULL);
	pSnowParticleSystem->dt = dt;
	if (myConfig.bRotateEffects && ! pDock->bDirectionUp && pDock->bHorizontalDock)
		pSnowParticleSystem->bDirectionUp = FALSE;
	
	double a = myConfig.fSnowParticleSpeed;
	static double epsilon = 0.1;
	double r = myConfig.iSnowParticleSize;
	double fBlend;
	double vmax = 1. / myConfig.iSnowDuration;
	CairoParticle *p;
	int i;
	for (i = 0; i < myConfig.iNbSnowParticles; i ++)
	{
		p = &(pSnowParticleSystem->pParticles[i]);
		
		p->x = 2 * g_random_double () - 1;
		p->y = 1.;
		p->z = 2 * g_random_double () - 1;
		p->fWidth = r*(p->z + 2)/3 * g_random_double ();
		p->fHeight = p->fWidth;
		
		p->vx = 0.;
		p->vy = -a * vmax * ((p->z + 1)/2 * g_random_double () + epsilon) * dt;
		p->iInitialLife = myConfig.iSnowDuration / dt;
		p->iLife = p->iInitialLife * (g_random_double () + 1)/2;
		
		{
			fBlend = g_random_double ();
			p->color[0] = fBlend * myConfig.pSnowColor1[0] + (1 - fBlend) * myConfig.pSnowColor2[0];
			p->color[1] = fBlend * myConfig.pSnowColor1[1] + (1 - fBlend) * myConfig.pSnowColor2[1];
			p->color[2] = fBlend * myConfig.pSnowColor1[2] + (1 - fBlend) * myConfig.pSnowColor2[2];
		}
		p->color[3] = 0.;
		
		p->fOscillation = G_PI * (2 * g_random_double () - 1);
		p->fOmega = 2*G_PI / myConfig.iSnowDuration * dt;  // tr/s
		
		p->fSizeFactor = 1.;
		p->fResizeSpeed = - .5 / myConfig.iSnowDuration * dt;  // zoom 0.5 a la fin.
	}
	
	return pSnowParticleSystem;
}



void cd_icon_effect_rewind_snow_particle (CairoParticle *p, double dt)
{
	static double epsilon = 0.1;
	double a = myConfig.fSnowParticleSpeed/1;
	double r = myConfig.iSnowParticleSize;
	double vmax = 1. / myConfig.iSnowDuration;
	p->x = 2 * g_random_double () - 1;
	p->y = 1.;
	p->z = 2 * g_random_double () - 1;
	
	p->fWidth = r*(p->z + 2)/3 * g_random_double ();
	p->fHeight = p->fWidth;
	
	p->vy = -a * vmax * ((p->z + 1)/2 * g_random_double () + epsilon) * dt;
	
	p->iInitialLife = myConfig.iSnowDuration / dt;
	p->iLife = p->iInitialLife * (g_random_double () + 1)/2;
	
	p->fSizeFactor = 1.;
}

