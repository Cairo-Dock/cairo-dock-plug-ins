/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-notifications.h"
#include "star-tex.h"

static double fRadius = .33;
static double a = .2;


gboolean cd_show_mouse_render (gpointer pUserData, CairoContainer *pContainer)
{
	CDShowMouseData *pData = CD_APPLET_GET_MY_CONTAINER_DATA (pContainer);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	glPushMatrix();
	glLoadIdentity();
	
	if (pContainer->bIsHorizontal)
		glTranslatef (pContainer->iMouseX ,pContainer->iHeight - pContainer->iMouseY , 0.);
	else
		glTranslatef (pContainer->iMouseY, pContainer->iWidth - pContainer->iMouseX, 0.);
	cairo_dock_render_particles (pData->pSystem);
	
	glPopMatrix();
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_show_mouse_update_container (gpointer pUserData, CairoContainer *pContainer, gboolean *bContinueAnimation)
{
	CDShowMouseData *pData = CD_APPLET_GET_MY_CONTAINER_DATA (pContainer);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (! pContainer->bInside)
	{
		pData->fAlpha -= .05;
		if (pData->fAlpha <= 0)
		{
			cairo_dock_free_particle_system (pData->pSystem);
			g_free (pData);
			CD_APPLET_SET_MY_CONTAINER_DATA (pContainer, NULL);
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		}
	}
	else if (pData->fAlpha != 1)
		pData->fAlpha = MIN (1., pData->fAlpha + .05);
	
	pData->fRotationAngle += 2*G_PI * myConfig.fRotationSpeed * g_iGLAnimationDeltaT / 1000.;
	
	cd_show_mouse_update_sources (pData);
	pData->pSystem->fWidth = 2 * MIN (96, pContainer->iHeight);
	pData->pSystem->fHeight =  MIN (96, pContainer->iHeight);
	cd_show_mouse_update_particle_system (pData->pSystem, pData);
	
	*bContinueAnimation = TRUE;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_show_mouse_enter_container (gpointer pUserData, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	if (! CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock)))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	CDShowMouseData *pData = CD_APPLET_GET_MY_CONTAINER_DATA (pContainer);
	if (pData == NULL)
	{
		pData = g_new0 (CDShowMouseData, 1);
		pData->fAlpha = 1.;
		
		double dt = g_iGLAnimationDeltaT;
		pData->pSourceCoords = cd_show_mouse_init_sources ();
		pData->pSystem = cd_show_mouse_init_system (pContainer, dt, pData->pSourceCoords);
		
		CD_APPLET_SET_MY_CONTAINER_DATA (pContainer, pData);
	}
	
	
	*bStartAnimation = TRUE;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}



gdouble *cd_show_mouse_init_sources (void)
{
	double *pSourceCoords = g_new (double, myConfig.iNbSources * 2);
	double fTheta;
	int i;
	for (i = 0; i < myConfig.iNbSources; i ++)
	{
		fTheta = 2*G_PI * i / myConfig.iNbSources;
		pSourceCoords[2*i] = fRadius * cos (fTheta);
		pSourceCoords[2*i+1] = fRadius * sin (fTheta);
	}
	return pSourceCoords;
}

CairoParticleSystem *cd_show_mouse_init_system (CairoContainer *pContainer, double dt, double *pSourceCoords)
{
	if (myData.iTexture == 0)
		myData.iTexture = cairo_dock_load_texture_from_raw_data (starTex, 32, 32);  /// 32 = sqrt (4096/4)
	double fHeight = pContainer->iHeight;  // iMaxDockHeight ?
	CairoParticleSystem *pParticleSystem = cairo_dock_create_particle_system (myConfig.iNbParticles * myConfig.iNbSources, myData.iTexture, 2*fHeight, fHeight);
	pParticleSystem->dt = dt;
	
	int iNumSource;
	double fBlend;
	double r = myConfig.iParticleSize / (1 + a);
	double sigma = myConfig.fScattering;
	CairoParticle *p;
	int i;
	for (i = 0; i < pParticleSystem->iNbParticles; i ++)
	{
		p = &(pParticleSystem->pParticles[i]);
		
		iNumSource = i / myConfig.iNbParticles;
		
		p->x = pSourceCoords[2*iNumSource];
		p->y = pSourceCoords[2*iNumSource+1];
		p->z = 0;
		p->fWidth = r * (a + g_random_double ());
		p->fHeight = p->fWidth;
		
		p->vx = sigma * (2 * g_random_double () - 1) * dt / myConfig.iParticleLifeTime;
		p->vy = sigma * (2 * g_random_double () - 1) * dt / myConfig.iParticleLifeTime;
		p->iInitialLife = ceil (myConfig.iParticleLifeTime / dt);
		p->iLife = g_random_int_range (1, p->iInitialLife+1);
		
		if (myConfig.bMysticalFire)
		{
			p->color[0] = g_random_double ();
			p->color[1] = g_random_double ();
			p->color[2] = g_random_double ();
		}
		else
		{
			fBlend = g_random_double ();
			p->color[0] = fBlend * myConfig.pColor1[0] + (1 - fBlend) * myConfig.pColor2[0];
			p->color[1] = fBlend * myConfig.pColor1[1] + (1 - fBlend) * myConfig.pColor2[1];
			p->color[2] = fBlend * myConfig.pColor1[2] + (1 - fBlend) * myConfig.pColor2[2];
		}
		p->color[3] = 1.;
		
		p->fSizeFactor = 1.;
		p->fResizeSpeed = .5 / myConfig.iParticleLifeTime * dt;  // zoom 1.5 a la fin.
	}
	
	return pParticleSystem;
	
}

void cd_show_mouse_update_sources (CDShowMouseData *pData)
{
	double *pSourceCoords = pData->pSourceCoords;
	double fTheta;
	int i;
	for (i = 0; i < myConfig.iNbSources; i ++)
	{
		fTheta = 2*G_PI * i / myConfig.iNbSources + pData->fRotationAngle;
		pSourceCoords[2*i] = fRadius * cos (fTheta);
		pSourceCoords[2*i+1] = fRadius * sin (fTheta);
	}
}


void cd_show_mouse_update_particle_system (CairoParticleSystem *pParticleSystem, CDShowMouseData *pData)
{
	double *pSourceCoords = pData->pSourceCoords;
	CairoParticle *p;
	double dt = pParticleSystem->dt;
	int i;
	double sigma = myConfig.fScattering;
	for (i = 0; i < pParticleSystem->iNbParticles; i ++)
	{
		p = &(pParticleSystem->pParticles[i]);
		
		p->x += p->vx;
		p->y += p->vy;
		p->color[3] = pData->fAlpha * p->iLife / p->iInitialLife;
		p->fSizeFactor += p->fResizeSpeed;
		if (p->iLife > 0)
		{
			p->iLife --;
			if (p->iLife == 0)
			{
				int iNumSource = i / myConfig.iNbParticles;
				p->x = pSourceCoords[2*iNumSource];
				p->y = pSourceCoords[2*iNumSource+1];
				
				p->vx = sigma * (2 * g_random_double () - 1) * dt / myConfig.iParticleLifeTime;
				p->vy = sigma * (2 * g_random_double () - 1) * dt / myConfig.iParticleLifeTime;
				
				p->color[3] = pData->fAlpha;
				p->fSizeFactor = 1.;
				
				p->iLife = g_random_int_range (1, p->iInitialLife+1);
			}
		}
	}
}
