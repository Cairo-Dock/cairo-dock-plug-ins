/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-evaporate.h"
#define CD_ILLUSION_EVAPORATE_LIMIT .90


gboolean cd_illusion_init_evaporate (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData, double dt)
{
	if (myData.iEvaporateTexture == 0)
		myData.iEvaporateTexture = cd_illusion_load_evaporate_texture ();
	double fMaxScale = (pDock->bAtBottom ? 1. : cairo_dock_get_max_scale (CAIRO_CONTAINER (pDock)));
	CairoParticleSystem *pEvaporateParticleSystem = cairo_dock_create_particle_system (myConfig.iNbEvaporateParticles, myData.iEvaporateTexture, pIcon->fWidth * pIcon->fScale, pIcon->fHeight * fMaxScale);
	g_return_val_if_fail (pEvaporateParticleSystem != NULL, FALSE);
	pEvaporateParticleSystem->dt = dt;
	pData->pEvaporateSystem = pEvaporateParticleSystem;
	
	double a = myConfig.fEvaporateParticleSpeed;
	static double epsilon = 0.1;
	double r = myConfig.iEvaporateParticleSize;
	double fBlend;
	double vmax = 1. / myConfig.iEvaporateDuration;
	CairoParticle *p;
	int i;
	for (i = 0; i < myConfig.iNbEvaporateParticles; i ++)
	{
		p = &(pEvaporateParticleSystem->pParticles[i]);
		
		p->x = 2 * g_random_double () - 1;
		p->x = p->x * p->x * (p->x > 0 ? 1 : -1);
		p->y = (myConfig.bEvaporateFromBottom ? 0. : 1.);
		p->z = 2 * g_random_double () - 1;
		p->fWidth = r*(p->z + 2)/3 * g_random_double ();
		p->fHeight = p->fWidth;
		
		p->vx = 0.;
		p->vy = a * vmax * ((p->z + 1)/2 + epsilon) * dt;
		p->iInitialLife = myConfig.iEvaporateDuration / dt;
		if (a > 1)
			p->iInitialLife = MIN (p->iInitialLife, 1. / p->vy);
		else
			p->iInitialLife = 8;
		p->iInitialLife *= g_random_double ();
		p->iLife = p->iInitialLife;
		
		if (myConfig.bMysticalEvaporate)
		{
			p->color[0] = g_random_double ();
			p->color[1] = g_random_double ();
			p->color[2] = g_random_double ();
		}
		else
		{
			fBlend = g_random_double ();
			p->color[0] = fBlend * myConfig.pEvaporateColor1[0] + (1 - fBlend) * myConfig.pEvaporateColor2[0];
			p->color[1] = fBlend * myConfig.pEvaporateColor1[1] + (1 - fBlend) * myConfig.pEvaporateColor2[1];
			p->color[2] = fBlend * myConfig.pEvaporateColor1[2] + (1 - fBlend) * myConfig.pEvaporateColor2[2];
		}
		p->color[3] = 1.;
		
		p->fOscillation = G_PI * (2 * g_random_double () - 1);
		p->fOmega = 2*G_PI / myConfig.iEvaporateDuration * dt;  // tr/s
		
		p->fSizeFactor = 1.;
		p->fResizeSpeed = -.5 / myConfig.iEvaporateDuration * dt;  // zoom 0.5 a la fin.
	}
	
	pData->fEvaporateSpeed = dt / myConfig.iEvaporateDuration;
	
	return TRUE;
}



static void _cd_illusion_rewind_evaporate_particle (CairoParticle *p, CDIllusionData *pData, double dt)
{
	static double epsilon = 0.1;
	double a = myConfig.fEvaporateParticleSpeed;
	double r = myConfig.iEvaporateParticleSize;
	double vmax = 1. / myConfig.iEvaporateDuration;
	p->x = 2 * g_random_double () - 1;
	p->x = p->x * p->x * (p->x > 0 ? 1 : -1);
	p->y = (myConfig.bEvaporateFromBottom ? pData->fEvaporatePercent : 1 - pData->fEvaporatePercent);
	p->fWidth = r*(p->z + 2)/3 * g_random_double ();
	p->fHeight = p->fWidth;
	p->vy = a * vmax * ((p->z + 1)/2 * g_random_double () + epsilon) * dt;
	
	p->iInitialLife = myConfig.iEvaporateDuration / dt;
	if (a > 1)
		p->iInitialLife = MIN (p->iInitialLife, 1. / p->vy);
	else
		p->iInitialLife = 8;
	p->iInitialLife *= g_random_double ();
	p->iLife = p->iInitialLife;
	
	p->fSizeFactor = 1.;
}
static void cd_illusion_update_evaporate_system (CairoParticleSystem *pParticleSystem, CDIllusionData *pData)
{
	CairoParticle *p;
	int i;
	for (i = 0; i < pParticleSystem->iNbParticles; i ++)
	{
		p = &(pParticleSystem->pParticles[i]);
		
		p->fOscillation += p->fOmega;
		p->x += p->vx + (p->z + 2)/3. * .02 * sin (p->fOscillation);  // 3%
		p->y += p->vy;
		p->color[3] = 1.*p->iLife / p->iInitialLife;
		p->fSizeFactor += p->fResizeSpeed;
		if (p->iLife > 0)
		{
			p->iLife --;
			if (p->iLife == 0)
			{
				_cd_illusion_rewind_evaporate_particle (p, pData, pParticleSystem->dt);
			}
		}
		else
			_cd_illusion_rewind_evaporate_particle (p, pData, pParticleSystem->dt);
	}
}

gboolean cd_illusion_update_evaporate (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	pData->fEvaporatePercent += pData->fEvaporateSpeed;
	
	cd_illusion_update_evaporate_system (pData->pEvaporateSystem, pData);
	pData->pEvaporateSystem->fHeight = pIcon->fHeight * pIcon->fScale;
	
	if (pData->fEvaporatePercent > CD_ILLUSION_EVAPORATE_LIMIT)
		cairo_dock_update_removing_inserting_icon_size_default (pIcon);
	
	cairo_dock_redraw_icon (pIcon, pDock);
	return (pData->fEvaporatePercent < 1 || pIcon->fPersonnalScale > .05);
}

void cd_illusion_draw_evaporate_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	glPushMatrix ();
	cairo_dock_set_icon_scale (pIcon, pDock, 1.);
	
	glColor4f (1., 1., 1., pIcon->fAlpha);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glEnable(GL_TEXTURE_2D); // Je veux de la texture
	glBindTexture(GL_TEXTURE_2D, pIcon->iIconTexture);
	glPolygonMode(GL_FRONT, GL_FILL);
	
	glNormal3f(0,0,1);
	glBegin(GL_QUADS);
	if (myConfig.bEvaporateFromBottom)
	{
		glTexCoord2f(0., 0.); glVertex3f(-.5,  .5, 0.);  // Bottom Left Of The Texture and Quad
		glTexCoord2f(1., 0.); glVertex3f( .5,  .5, 0.);  // Bottom Right Of The Texture and Quad
		glTexCoord2f(1., 1 - pData->fEvaporatePercent); glVertex3f( .5, -.5 + pData->fEvaporatePercent, 0.);  // Top Right Of The Texture and Quad
		glTexCoord2f(0., 1 - pData->fEvaporatePercent); glVertex3f(-.5, -.5 + pData->fEvaporatePercent, 0.);  // Top Left Of The Texture and Quad
	}
	else
	{
		glTexCoord2f(0., pData->fEvaporatePercent); glVertex3f(-.5,  .5 - pData->fEvaporatePercent, 0.);  // Bottom Left Of The Texture and Quad
		glTexCoord2f(1., pData->fEvaporatePercent); glVertex3f( .5,  .5 - pData->fEvaporatePercent, 0.);  // Bottom Right Of The Texture and Quad
		glTexCoord2f(1., 1.); glVertex3f( .5, -.5, 0.);  // Top Right Of The Texture and Quad
		glTexCoord2f(0., 1.); glVertex3f(-.5, -.5, 0.);  // Top Left Of The Texture and Quad
	}
	glEnd();
	
	glPopMatrix ();
	
	/*if (pDock->bUseReflect)
	{
		glPushMatrix ();
		double x0, y0, x1, y1;
		double fReflectRatio = myIcons.fReflectSize * pDock->fRatio / pIcon->fHeight / pIcon->fScale;
		double fOffsetY = pIcon->fHeight * pIcon->fScale/2 + (myIcons.fReflectSize/2 + pIcon->fDeltaYReflection) * pDock->fRatio;
		if (pDock->bHorizontalDock)
		{
			if (pDock->bDirectionUp)
			{
				fOffsetY = pIcon->fHeight * pIcon->fScale + pIcon->fDeltaYReflection;
				glTranslatef (0., - fOffsetY, 0.);
				glScalef (pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, - pIcon->fHeight * pIcon->fScale, 1.);  // taille du reflet et on se retourne.
				x0 = 0.;
				y0 = 1. - fReflectRatio;
				x1 = 1.;
				y1 = 1.;
			}
			else
			{
				glTranslatef (0., fOffsetY, 0.);
				glScalef (pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, myIcons.fReflectSize * pDock->fRatio, 1.);
				x0 = 0.;
				y0 = fReflectRatio;
				x1 = 1.;
				y1 = 0.;
			}
		}
		else
		{
			if (pDock->bDirectionUp)
			{
				glTranslatef (fOffsetY, 0., 0.);
				glScalef (- myIcons.fReflectSize * pDock->fRatio, pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, 1.);
				x0 = 1. - fReflectRatio;
				y0 = 0.;
				x1 = 1.;
				y1 = 1.;
			}
			else
			{
				glTranslatef (- fOffsetY, 0., 0.);
				glScalef (myIcons.fReflectSize * pDock->fRatio, pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, 1.);
				x0 = fReflectRatio;
				y0 = 0.;
				x1 = 0.;
				y1 = 1.;
			}
		}
		//glEnableClientState(GL_COLOR_ARRAY);
		
		//glColorPointer(4, GL_FLOAT, 0, pData->pColors);
		//glDrawArrays (GL_TRIANGLE_FAN, 0, pData->iNumActiveNodes);
		
		//glDisableClientState(GL_COLOR_ARRAY);
		
		
		glActiveTexture(GL_TEXTURE0_ARB); // Go pour le multitexturing 1ere passe
		glEnable(GL_TEXTURE_2D); // On active le texturing sur cette passe
		glBindTexture(GL_TEXTURE_2D, pIcon->iIconTexture);
		
		glColor4f(1., 1., 1., myIcons.fAlbedo * pIcon->fAlpha);  // transparence du reflet.
		glEnable(GL_BLEND);
		glBlendFunc (1, 0);
		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		
		glActiveTexture(GL_TEXTURE1_ARB); // Go pour le texturing 2eme passe
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, g_pGradationTexture[pDock->bHorizontalDock]);
		glColor4f(1., 1., 1., 1.);  // transparence du reflet.
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // Le mode de combinaison des textures
		glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);  // multiplier les alpha.
		glEnableClientState (GL_TEXTURE_COORD_ARRAY);
		glEnableClientState (GL_VERTEX_ARRAY);
		
	glTexCoordPointer (2, GL_FLOAT, 0, pData->pCoords);
	glVertexPointer (3, GL_FLOAT, 0, pData->pVertices);
		
		glDrawArrays (GL_TRIANGLE_FAN, 0, pData->iNumActiveNodes);
		
		glActiveTexture(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		glDisableClientState (GL_TEXTURE_COORD_ARRAY);
		glDisableClientState (GL_VERTEX_ARRAY);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glActiveTexture(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		
		glPopMatrix ();
	}*/
	
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
	
	if (pData->fEvaporatePercent <= CD_ILLUSION_EVAPORATE_LIMIT)
	{
		glPushMatrix ();
		glTranslatef (0., - pIcon->fHeight * pIcon->fScale/2, 0.);
		cairo_dock_render_particles (pData->pEvaporateSystem);
		glPopMatrix ();
	}
}
