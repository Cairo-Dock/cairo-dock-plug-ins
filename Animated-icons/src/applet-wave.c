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
#include "applet-wave.h"


void cd_animations_init_wave (CDAnimationData *pData)
{
	pData->pCoords[0] = .5;
	pData->pCoords[1] = .5;
	
	pData->pVertices[0] = 0.;
	pData->pVertices[1] = 0.;
	pData->pVertices[2] = 0.;
	pData->fWavePosition = - myConfig.fWaveWidth / 2 + .01;  // on rajoute epsilon pour commencer avec 2 points.
	
	pData->iNumActiveNodes = 6;
	pData->pVertices[3*1+0] = -.5;
	pData->pVertices[3*1+1] = -.5;
	pData->pVertices[3*2+0] = .5;
	pData->pVertices[3*2+1] = -.5;
	pData->pVertices[3*3+0] = .5;
	pData->pVertices[3*3+1] = .5;
	pData->pVertices[3*4+0] = -.5;
	pData->pVertices[3*4+1] = .5;
	pData->pVertices[3*5+0] = -.5;
	pData->pVertices[3*5+1] = -.5;
	pData->pCoords[2*1+0] = 0.;
	pData->pCoords[2*1+1] = 1.;
	pData->pCoords[2*2+0] = 1.;
	pData->pCoords[2*2+1] = 1.;
	pData->pCoords[2*3+0] = 1.;
	pData->pCoords[2*3+1] = 0.;
	pData->pCoords[2*4+0] = 0.;
	pData->pCoords[2*4+1] = 0.;
	pData->pCoords[2*5+0] = 0.;
	pData->pCoords[2*5+1] = 1.;
	pData->bIsWaving = TRUE;
}


gboolean cd_animations_update_wave (CairoDock *pDock, CDAnimationData *pData, double dt)
{
	GLfloat *pVertices = &pData->pVertices[3];
	GLfloat *pCoords = &pData->pCoords[2];
	double x, y, a, a_, x_;
	double p = pData->fWavePosition, w = myConfig.fWaveWidth;
	int n = CD_WAVE_NB_POINTS / 2;  // nbre de points pour une demi-vague (N est impair).
	int i, j=0, k;
	for (i = 0; i < CD_WAVE_NB_POINTS; i ++)  // on discretise la vague.
	{
		a = 1. * (i-n) / (n);  // position sur la vague, dans [-1, 1].
		x = p + a * w/2;  // abscisse correspondante.
		if (i == 0)  // 1er point.
		{
			if (p - w/2 < 0)  // la vague depasse du bas.
			{
				a_ = 1. * (i+1-n) / (n);
				x_ = p + a_ * w/2;
				if (x_ > 0)  // le point suivant est dedans.
				{
					y = myConfig.fWaveAmplitude * cos (G_PI/2 * a + x / (w/2) * G_PI/2);
					x = 0.;
					//y = myConfig.fWaveAmplitude * cos (G_PI/2 * (a+a_)/2);
				}
				else
					continue ;
			}
			else
			{
				x = 0.;
				y = 0.;
			}
		}
		else if (i == CD_WAVE_NB_POINTS-1)  // dernier point.
		{
			if (p + w/2 > 1)  // la vague depasse du haut.
			{
				a_ = 1. * (i-1-n) / (n);
				x_ = p + a_ * w/2;
				if (x_ < 1)  // le point precedent est dedans.
				{
					y = myConfig.fWaveAmplitude * cos (G_PI/2 * a - (x-1) / (w/2) * G_PI/2);
					x = 1.;
					//y = myConfig.fWaveAmplitude * cos (G_PI/2 * (a+a_)/2);
				}
				else
					continue;
			}
			else
			{
				x = 1.;
				y = 0.;
			}
		}
		else  // dedans.
		{
			a = 1. * (i-n) / (n);
			x = p + a * w/2;
			if (x < 0)  // le point sort par le bas.
			{
				a_ = 1. * (i+1-n) / (n);
				x_ = p + a_ * w/2;
				if (x_ > 0)  // le point suivant est dedans.
				{
					y = myConfig.fWaveAmplitude * cos (G_PI/2 * a - x / (w/2) * G_PI/2);
					x = 0.;  // coin de la texture.
					//y = myConfig.fWaveAmplitude * cos (G_PI/2 * (a+a_)/2);
				}
				else  // tout le segment est dehors, on zappe.
					continue ;
			}
			else if (x > 1)  // le point sort par le haut.
			{
				a_ = 1. * (i-1-n) / (n);
				x_ = p + a_ * w/2;
				if (x_ < 1)  // le point precedent est dedans.
				{
					y = myConfig.fWaveAmplitude * cos (G_PI/2 * a - (x-1) / (w/2) * G_PI/2);
					x = 1.;  // coin de la texture.
					//y = myConfig.fWaveAmplitude * cos (G_PI/2 * (a+a_)/2);
				}
				else  // tout le segment est dehors, on zappe.
					continue;
			}
			else  // le point est dans l'icone.
				y = myConfig.fWaveAmplitude * cos (G_PI/2 * a);
		}
		
		pCoords[2*j] = 0.;
		pCoords[2*j+1] = x;
		
		pVertices[3*j] = -.5 - y;
		pVertices[3*j+1] = .5 - x;
		pVertices[3*j+2] = 0.;
		
		j ++;
	}
	
	for (i = 0; i < j; i ++)
	{
		k = 2 * j - 1 - i;
		pCoords[2*k] = 1.;
		pCoords[2*k+1] = pCoords[2*i+1];
		
		pVertices[3*k] = - pVertices[3*i];
		pVertices[3*k+1] = pVertices[3*i+1];
		pVertices[3*k+2] = 0.;
	}
	
	// on boucle.
	j = 2 * j;
	pCoords[2*j] = pCoords[0];
	pCoords[2*j+1] = pCoords[1];
	
	pVertices[3*j] = pVertices[0];
	pVertices[3*j+1] = pVertices[1];
	pVertices[3*j+2] = 0.;
	
	pData->fWavePosition += dt / myConfig.iWaveDuration;
	pData->iNumActiveNodes = j + 2;
	
	cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));
	return (pData->fWavePosition - w/2 < 1);
}

void cd_animations_draw_wave_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData)
{
	glPushMatrix ();
	cairo_dock_set_icon_scale (pIcon, pDock, 1.);
	
	glColor4f (1., 1., 1., pIcon->fAlpha);
	glEnable(GL_BLEND);
	if (pIcon->fAlpha == 1)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		_cairo_dock_set_blend_alpha ();
	
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glEnable(GL_TEXTURE_2D); // Je veux de la texture
	glBindTexture(GL_TEXTURE_2D, pIcon->iIconTexture);
	glPolygonMode(GL_FRONT, GL_FILL);
	
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnableClientState (GL_VERTEX_ARRAY);
	
	glTexCoordPointer (2, GL_FLOAT, 0, pData->pCoords);
	glVertexPointer (3, GL_FLOAT, 0, pData->pVertices);

	glDrawArrays (GL_TRIANGLE_FAN, 0, pData->iNumActiveNodes);
	
	glPopMatrix ();
	
	if (pDock->container.bUseReflect)
	{
		glPushMatrix ();
		double x0, y0, x1, y1;
		double fReflectRatio = myIcons.fReflectSize * pDock->container.fRatio / pIcon->fHeight / pIcon->fScale;
		double fOffsetY = pIcon->fHeight * pIcon->fScale/2 + (myIcons.fReflectSize/2 + pIcon->fDeltaYReflection) * pDock->container.fRatio;
		if (pDock->container.bIsHorizontal)
		{
			if (pDock->container.bDirectionUp)
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
				glScalef (pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, myIcons.fReflectSize * pDock->container.fRatio, 1.);
				x0 = 0.;
				y0 = fReflectRatio;
				x1 = 1.;
				y1 = 0.;
			}
		}
		else
		{
			if (pDock->container.bDirectionUp)
			{
				glTranslatef (fOffsetY, 0., 0.);
				glScalef (- myIcons.fReflectSize * pDock->container.fRatio, pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, 1.);
				x0 = 1. - fReflectRatio;
				y0 = 0.;
				x1 = 1.;
				y1 = 1.;
			}
			else
			{
				glTranslatef (- fOffsetY, 0., 0.);
				glScalef (myIcons.fReflectSize * pDock->container.fRatio, pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, 1.);
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
		glBindTexture(GL_TEXTURE_2D, g_pGradationTexture[pDock->container.bIsHorizontal]);
		glColor4f(1., 1., 1., 1.);  // transparence du reflet.
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // Le mode de combinaison des textures
		glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);  // multiplier les alpha.
		/*glEnable(GL_TEXTURE_GEN_S);                                // generation texture en S
		glEnable(GL_TEXTURE_GEN_T);        // et en T
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR); // la je veux un mapping tout ce qu'il y a de plus classique
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);*/
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
	}
	
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
}
