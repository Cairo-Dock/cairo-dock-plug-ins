/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

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
	pData->fWavePosition = - myConfig.fWaveWidth / 2;
	pData->bIsWaving = TRUE;
}


gboolean cd_animations_update_wave (CDAnimationData *pData)
{
	GLfloat *pVertices = &pData->pVertices[3];
	GLfloat *pCoords = &pData->pCoords[2];
	double x, y, a, a_, x_;
	double p = pData->fWavePosition, w = myConfig.fWaveWidth;
	int n = (CD_WAVE_NB_POINTS - 1) / 2;  // nbre de points pour une demi-vague.
	int i, j=0, k;
	for (i = 0; i < CD_WAVE_NB_POINTS; i ++)
	{
		if (i == 0)
		{
			if (p - w/2 < 0)
			{
				a_ = 1. * (i+1-n) / (n-1);
				x_ = p + a_ * w/2;
				if (x_ > 0)  // le point suivant est dedans.
				{
					y = myConfig.fWaveAmplitude * cos (G_PI/2 * a + x / (w/2) * G_PI/2);
					x = 0.;
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
		else if (i == CD_WAVE_NB_POINTS-1)
		{
			if (p + w/2 > 1)
			{
				a_ = 1. * (i-1-n) / (n-1);
				x_ = p + a_ * w/2;
				if (x_ < 1)  // le point precedent est dedans.
				{
					y = myConfig.fWaveAmplitude * cos (G_PI/2 * a - (x-1) / (w/2) * G_PI/2);
					x = 1.;
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
		else
		{
			a = 1. * (i-n) / (n-1);
			x = p + a * w/2;
			if (x < 0)
			{
				a_ = 1. * (i+1-n) / (n-1);
				x_ = p + a_ * w/2;
				if (x_ > 0)  // le point suivant est dedans.
				{
					y = myConfig.fWaveAmplitude * cos (G_PI/2 * a - x / (w/2) * G_PI/2);
					x = 0.;
				}
				else
					continue ;
			}
			else if (x > 1)
			{
				a_ = 1. * (i-1-n) / (n-1);
				x_ = p + a_ * w/2;
				if (x_ < 1)  // le point precedent est dedans.
				{
					y = myConfig.fWaveAmplitude * cos (G_PI/2 * a - (x-1) / (w/2) * G_PI/2);
					x = 1.;
				}
				else
					continue;
			}
			else
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
	
	pData->fWavePosition += 1. * g_iGLAnimationDeltaT / myConfig.iWaveDuration;
	pData->iNumActiveNodes = j + 2;
	return (pData->fWavePosition - w/2 < 1);
}

void cd_animations_draw_wave_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData)
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
	
	
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnableClientState (GL_VERTEX_ARRAY);
	
	glTexCoordPointer (2, GL_FLOAT, 0, pData->pCoords);
	glVertexPointer (3, GL_FLOAT, 0, pData->pVertices);

	glDrawArrays (GL_TRIANGLE_FAN, 0, pData->iNumActiveNodes);
	
	
	if (pDock->bUseReflect)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		
		glColorPointer(4, GL_FLOAT, 0, pData->pColors);
		
		glDisableClientState(GL_COLOR_ARRAY);
	}
	
	glPopMatrix ();
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_2D);
	glDisable (GL_BLEND);
}
