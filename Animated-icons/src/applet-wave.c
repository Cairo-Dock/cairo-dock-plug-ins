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
#include "applet-notifications.h"
#include "applet-wave.h"

static inline void _init_wave (GLfloat *pVertices, GLfloat *pCoords)
{
	// point bas gauche.
	pVertices[0] = -.5;
	pVertices[1] = -.5;
	pCoords[0] = 0.;
	pCoords[1] = 1.;
	
	// point bas droit.
	pVertices[2] = .5;
	pVertices[3] = -.5;
	pCoords[2] = 1.;
	pCoords[3] = 1.;
	
	// point haut gauche.
	pVertices[4] = -.5;
	pVertices[5] = .5;
	pCoords[4] = 0.;
	pCoords[5] = 0.;
	
	// point haut droit.
	pVertices[6] = .5;
	pVertices[7] = .5;
	pCoords[6] = 1;
	pCoords[7] = 0.;
}

static void init (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL)
{
	_init_wave (pData->pVertices, pData->pCoords);
	
	pData->iNumActiveNodes = 4;
	pData->fWavePosition = - myConfig.fWaveWidth / 2 + .01;  // on rajoute epsilon pour commencer avec 2 points.
}


static gboolean update (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL, gboolean bRepeat)
{
	GLfloat *pVertices = pData->pVertices;
	GLfloat *pCoords = pData->pCoords;
	
	double p = pData->fWavePosition, w = myConfig.fWaveWidth;
	if (p + w/2 < 0 || p - w/2 > 1)  // la vague est entierement en-dehors du cote.
	{
		_init_wave (pVertices, pCoords);  // on s'assure d'avoir au moins un carre.
		pData->iNumActiveNodes = 4;
	}
	else
	{
		int j = 0;  // nombre de points calcules.
		if (p - w/2 > 0)  // la vague n'englobe pas le point du bas.
		{
			pVertices[0] = -.5;
			pVertices[1] = -.5;
			pCoords[0] = 0.;
			pCoords[1] = 1.;
			j ++;
		}
		
		double x, x_, y;  // position du point et du point precedent.
		double a, a_;  // position sur la vague dans [-1, 1].
		int n = CD_WAVE_NB_POINTS / 2;  // nbre de points pour une demi-vague (N est impair).
		int i;
		for (i = 0; i < CD_WAVE_NB_POINTS; i ++)  // on discretise la vague.
		{
			a = 1. * (i-n) / (n);  // position courante sur la vague, dans [-1, 1].
			x = p + a * w/2;  // abscisse correspondante.
			
			a = 1. * (i-n) / n;  // position courante sur la vague, dans [-1, 1].
			x = p + a * w/2;  // abscisse correspondante.
			if (x < 0)  // le point sort par le bas.
			{
				a_ = 1. * (i+1-n) / n;
				x_ = p + a_ * w/2;
				if (x_ > 0)  // le point suivant est dedans.
				{
					y = myConfig.fWaveAmplitude * cos (G_PI/2 * a - x / (w/2) * G_PI/2);  // on depasse de x, donc on dephase d'autant.
					x = 0.;  // coin de la texture.
				}
				else  // tout le segment est dehors, on zappe.
					continue ;
			}
			else if (x > 1)  // le point sort par le haut.
			{
				a_ = 1. * (i-1-n) / n;
				x_ = p + a_ * w/2;
				if (x_ < 1)  // le point precedent est dedans.
				{
					y = myConfig.fWaveAmplitude * cos (G_PI/2 * a - (x-1) / (w/2) * G_PI/2);  // on depasse de x-1, donc on dephase d'autant.
					x = 1.;  // coin de la texture.
				}
				else  // tout le segment est dehors, on zappe.
					continue;
			}
			else  // le point est dans l'icone.
				y = myConfig.fWaveAmplitude * cos (G_PI/2 * a);
			
			pVertices[4*j] = -.5 - y;
			pVertices[4*j+1] = x - .5;
			pCoords[4*j] = 0.;
			pCoords[4*j+1] = 1. - x;
			j ++;
		}
		
		if (p + w/2 < 1)  // la vague n'englobe pas le point du haut.
		{
			pVertices[4*j] = -.5;
			pVertices[4*j+1] = .5;
			pCoords[4*j] = 0.;
			pCoords[4*j+1] = 0.;
			j ++;
		}
		
		for (i = 0; i < j; i ++)  // on complete l'autre cote symetriquement.
		{
			pVertices[4*i+2] = - pVertices[4*i];
			pVertices[4*i+3] = pVertices[4*i+1];
			pCoords[4*i+2] = 1.;
			pCoords[4*i+3] = pCoords[4*i+1];
		}
		pData->iNumActiveNodes = 2*j;
	}
	
	pData->fWavePosition += dt / myConfig.iWaveDuration;
	
	cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));
	
	gboolean bContinue = (pData->fWavePosition - w/2 < 1);
	if (! bContinue && bRepeat)
		pData->fWavePosition = - myConfig.fWaveWidth / 2;
	return bContinue;
}


static void render (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	glPushMatrix ();
	cairo_dock_set_icon_scale (pIcon, CAIRO_CONTAINER (pDock), 1.);
	
	glColor4f (1., 1., 1., pIcon->fAlpha);
	glEnable(GL_BLEND);
	if (pIcon->fAlpha == 1)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		_cairo_dock_set_blend_alpha ();
	
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glEnable(GL_TEXTURE_2D); // Je veux de la texture
	glBindTexture(GL_TEXTURE_2D, pIcon->image.iTexture);
	glPolygonMode(GL_FRONT, GL_FILL);
	
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnableClientState (GL_VERTEX_ARRAY);
	
	glTexCoordPointer (2, GL_FLOAT, 0, pData->pCoords);
	glVertexPointer (2, GL_FLOAT, 0, pData->pVertices);

	glDrawArrays (GL_QUAD_STRIP, 0, pData->iNumActiveNodes);
	
	glPopMatrix ();
	
	if (pDock->container.bUseReflect)
	{
		glPushMatrix ();
		// double fReflectRatio = pDock->iIconSize * myIconsParam.fReflectHeightRatio * pDock->container.fRatio / pIcon->fHeight / pIcon->fScale;
		double fOffsetY = pIcon->fHeight * pIcon->fScale/2 + (pDock->iIconSize * myIconsParam.fReflectHeightRatio/2 + pIcon->fDeltaYReflection) * pDock->container.fRatio;
		if (pDock->container.bIsHorizontal)
		{
			if (pDock->container.bDirectionUp)
			{
				fOffsetY = pIcon->fHeight * pIcon->fScale + pIcon->fDeltaYReflection;
				glTranslatef (0., - fOffsetY, 0.);
				glScalef (pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, - pIcon->fHeight * pIcon->fScale, 1.);  // taille du reflet et on se retourne.
			}
			else
			{
				glTranslatef (0., fOffsetY, 0.);
				glScalef (pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, pDock->iIconSize * myIconsParam.fReflectHeightRatio * pDock->container.fRatio, 1.);
			}
		}
		else
		{
			if (pDock->container.bDirectionUp)
			{
				glTranslatef (fOffsetY, 0., 0.);
				glScalef (- pDock->iIconSize * myIconsParam.fReflectHeightRatio * pDock->container.fRatio, pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, 1.);
			}
			else
			{
				glTranslatef (- fOffsetY, 0., 0.);
				glScalef (pDock->iIconSize * myIconsParam.fReflectHeightRatio * pDock->container.fRatio, pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, 1.);
			}
		}
		
		glActiveTexture(GL_TEXTURE0_ARB); // Go pour le multitexturing 1ere passe
		glEnable(GL_TEXTURE_2D); // On active le texturing sur cette passe
		glBindTexture(GL_TEXTURE_2D, pIcon->image.iTexture);
		
		glColor4f(1., 1., 1., myIconsParam.fAlbedo * pIcon->fAlpha);  // transparence du reflet.
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
		glVertexPointer (2, GL_FLOAT, 0, pData->pVertices);
		
		glDrawArrays (GL_QUAD_STRIP, 0, pData->iNumActiveNodes);
		
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


void cd_animations_register_wave (void)
{
	CDAnimation *pAnimation = &myData.pAnimations[CD_ANIMATIONS_WAVE];
	pAnimation->cName = "wave";
	pAnimation->cDisplayedName = D_("Wave");
	pAnimation->id = CD_ANIMATIONS_WAVE;
	pAnimation->bDrawIcon = TRUE;
	pAnimation->bDrawReflect = FALSE;
	pAnimation->init = init;
	pAnimation->update = update;
	pAnimation->render = render;
	pAnimation->post_render = NULL;
	cd_animations_register_animation (pAnimation);
}
