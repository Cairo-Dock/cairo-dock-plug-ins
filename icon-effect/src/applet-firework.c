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
#include "applet-firework.h"

const double g = 0.81;
const double g_ = 3 * .81;


static inline void _launch_one_firework (CDFirework *pFirework, CairoDock *pDock, double dt)
{
	double k = myConfig.fFireworkFriction;
	double T = myConfig.iFireworkDuration;
	double r = myConfig.iFireworkParticleSize;
	pFirework->x_expl = 2 * g_random_double () - 1;  // entre -1 et 1;
	pFirework->y_expl = .5 + .3 * g_random_double ();  // entre 50% et 80% de h.
	//g_print ("expl (%.2f, %.2f)\n", pFirework->x_expl, pFirework->y_expl);
	pFirework->r_expl = myConfig.fFireworkRadius + .1 - .2 * g_random_double ();  // +/- 10%.
	pFirework->v_expl = pFirework->r_expl * k / (1 - exp (-k*T));  // vitesse initiale de dispersion pour atteindre le rayon.
	pFirework->t = 0;
	if (myConfig.bFireworkShoot)
	{
		pFirework->t_expl = sqrt (2 * pFirework->y_expl / g_);  // temps mis pour arriver au sommet.
		pFirework->vy_decol = g_ * pFirework->t_expl;  // vitesse verticale initiale necessaire pour atteindre exactement y_expl.
		pFirework->x_sol = pFirework->x_expl;  // la fusee part verticalement.
		pFirework->vx_decol = (pFirework->x_expl - pFirework->x_sol) / pFirework->t_expl;
		pFirework->xf = pFirework->x_sol;
		pFirework->yf = 0.;
	}
	else
	{
		pFirework->xf = pFirework->x_expl;
		pFirework->yf = pFirework->y_expl;
	}
	
	gdouble fColor[3];
	if (myConfig.bFireworkRandomColors)
	{
		fColor[0] = g_random_double ();
		fColor[1] = g_random_double ();
		fColor[2] = g_random_double ();
	}
	else
	{
		fColor[0] = myConfig.pFireworkColor[0];
		fColor[1] = myConfig.pFireworkColor[1];
		fColor[2] = myConfig.pFireworkColor[2];
		//memcpy (fColor, myConfig.pFireworkColor, 3 * sizeof (gdouble));
	}
	
	double angle;
	CairoParticleSystem *pParticleSystem = pFirework->pParticleSystem;
	CairoParticle *p;
	int j;
	for (j = 0; j < pParticleSystem->iNbParticles; j ++)
	{
		p = &(pParticleSystem->pParticles[j]);
		
		p->x = pFirework->x_expl;
		p->y = pFirework->y_expl;
		p->z = 1.;
		p->fWidth = r/2 * pDock->container.fRatio;
		p->fHeight = p->fWidth;
		
		angle = (double)j / pParticleSystem->iNbParticles * 2*G_PI;
		p->vx = pFirework->v_expl * cos (angle);
		p->vy = pFirework->v_expl * sin (angle);
		p->iInitialLife = ceil (T / dt);
		p->iLife = p->iInitialLife * (.8+.3*g_random_double ());  // dispersion entre .8 et 1.1
		
		//memcpy (p->color, fColor, 3 * sizeof (gdouble));
		p->color[0] = fColor[0];
		p->color[1] = fColor[1];
		p->color[2] = fColor[2];
		
		p->fOscillation = G_PI * (2 * g_random_double () - 1);
		p->fOmega = 2*G_PI / myConfig.iFireworkDuration * dt;  // tr/s
		
		p->fSizeFactor = 1.;
		p->fResizeSpeed = 0.;
	}
}

void cd_icon_effect_init_firework (Icon *pIcon, CairoDock *pDock, double dt, CDIconEffectData *pData)
{
	if (myData.iFireTexture == 0)
		myData.iFireTexture = cd_icon_effect_load_firework_texture ();
	double fMaxScale = 1. + g_fAmplitude * pDock->fMagnitudeMax;
	
	pData->pFireworks = g_new0 (CDFirework, myConfig.iNbFireworks);
	pData->iNbFireworks = myConfig.iNbFireworks;
	
	CDFirework *pFirework;
	int i, j;
	for (i = 0; i < pData->iNbFireworks; i ++)
	{
		pFirework = &pData->pFireworks[i];
		
		pFirework->pParticleSystem = cairo_dock_create_particle_system (myConfig.iNbFireworkParticles, myData.iFireTexture, pIcon->fWidth * fMaxScale, pIcon->fHeight * fMaxScale);
		g_return_if_fail (pFirework->pParticleSystem != NULL);
		pFirework->pParticleSystem->dt = dt;
		if (myConfig.bRotateEffects && ! pDock->container.bDirectionUp && pDock->container.bIsHorizontal)
			pFirework->pParticleSystem->bDirectionUp = FALSE;
		pFirework->pParticleSystem->bAddLuminance = TRUE;
		pFirework->pParticleSystem->bAddLight = myConfig.bFireworkLuminance;
		
		_launch_one_firework (pFirework, pDock, dt);
	}
}

static gboolean _cd_icon_effect_update_firework_system (CDFirework *pFirework, CairoParticleSystem *pParticleSystem, double t)
{
	double dt = mySystem.iGLAnimationDeltaT * 1e-3;
	double k = myConfig.fFireworkFriction;
	double a;
	// y" + ky' = -g -> Y = -g/k + a*exp (-kt) -> y = y0 - g/k*t + a/k*(1 - exp(-kt))
	gboolean bAllParticlesEnded = TRUE;
	CairoParticle *p;
	int i;
	for (i = 0; i < pParticleSystem->iNbParticles; i ++)
	{
		p = &(pParticleSystem->pParticles[i]);
		
		p->fOscillation += p->fOmega;
		p->x = pFirework->x_expl + p->vx / k * (1 - exp (-k*t)) * 2;  // *2 car x varie dans [-1;1]
		
		a = p->vy + g / k; 
		p->y = pFirework->y_expl - g / k * t + a / k * (1 - exp (-k*t));
		
		p->color[3] = sqrt ((double)p->iLife / p->iInitialLife);
		//p->color[3] = 1.;
		
		p->x += .04 * sin (p->fOscillation) * (1 - (double)p->iLife / p->iInitialLife);
		if (exp (-k*t) < .05)
		{
			p->color[3] *= (1 + sin (4*p->fOscillation)) / 2;
			//if (i == 0)
				//g_print ("oscillation -> %.2f\n", p->color[3]);
		}
		
		p->fSizeFactor += p->fResizeSpeed;
		if (p->iLife > 0)
		{
			p->iLife --;
			if (bAllParticlesEnded && p->iLife != 0)
				bAllParticlesEnded = FALSE;
		}
	}
	return bAllParticlesEnded;
}

gboolean cd_icon_effect_update_fireworks (Icon *pIcon, CairoDock *pDock, CDIconEffectData *pData, gboolean bWillContinue)
{
	double dt = mySystem.iGLAnimationDeltaT * 1e-3;
	gboolean bAllParticlesEnded = TRUE;
	gboolean bParticlesEnded;
	double t;
	CDFirework *pFirework;
	int i;
	for (i = 0; i < pData->iNbFireworks; i ++)
	{
		pFirework = &pData->pFireworks[i];
		
		pFirework->t += dt;
		t = pFirework->t;
		if (pFirework->vy_decol != 0)
		{
			//g_print ("launching : %.2fx%.2f (t=%.2f/%.2fs)\n", pFirework->xf, pFirework->yf, t, pFirework->t_expl);
			pFirework->xf += pFirework->vx_decol * dt;
			pFirework->yf = pFirework->vy_decol * t - .5 * g_ * t * t;
			if (t >= pFirework->t_expl)
			{
				pFirework->vy_decol = 0.;
				pFirework->t = 0;
			}
			bAllParticlesEnded = FALSE;
		}
		else
		{
			//g_print ("explosion\n");
			bParticlesEnded = _cd_icon_effect_update_firework_system (pFirework, pFirework->pParticleSystem, t);
			if (bParticlesEnded && bWillContinue)
			{
				_launch_one_firework (pFirework, pDock, mySystem.iGLAnimationDeltaT);
				bParticlesEnded = FALSE;
			}
			bAllParticlesEnded &= bParticlesEnded;
		}
		//pFirework->pParticleSystem->fWidth = pIcon->fWidth * pIcon->fScale;
	}
	
	return ! bAllParticlesEnded;
}


void cd_icon_effect_render_fireworks (CDIconEffectData *pData)
{
	CDFirework *pFirework;
	CairoParticleSystem *pParticleSystem;
	int i;
	for (i = 0; i < pData->iNbFireworks; i ++)
	{
		pFirework = &pData->pFireworks[i];
		pParticleSystem = pFirework->pParticleSystem;
		if (pFirework->vy_decol != 0)
		{
			_cairo_dock_enable_texture ();
			_cairo_dock_set_blend_alpha ();
			
			glColor4f (1., 1., 0., 1.);
			glBindTexture (GL_TEXTURE_2D, myData.iFireTexture);
			_cairo_dock_apply_current_texture_at_size_with_offset (7, 13, pFirework->xf * pParticleSystem->fWidth / 2, pFirework->yf * pParticleSystem->fHeight);
			
			_cairo_dock_disable_texture ();
		}
		else
		{
			cairo_dock_render_particles (pFirework->pParticleSystem);
		}
	}
}


void cd_icon_effect_free_fireworks (CDIconEffectData *pData)
{
	CDFirework *pFirework;
	int i;
	for (i = 0; i < pData->iNbFireworks; i ++)
	{
		pFirework = &pData->pFireworks[i];
		cairo_dock_free_particle_system (pFirework->pParticleSystem);
	}
	g_free (pData->pFireworks);
	pData->pFireworks = NULL;
	pData->iNbFireworks = 0;
}
