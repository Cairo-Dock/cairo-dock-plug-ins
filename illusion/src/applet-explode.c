/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-explode.h"

static double vmax = .4;

gboolean cd_illusion_init_explode (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData, double dt)
{
	pData->fExplodeDeltaT = dt;
	pData->iExplosionCount = 0;
	
	pData->pExplosionPart = g_new0 (CDIllusionExplosion, myConfig.iExplodeNbPiecesX * myConfig.iExplodeNbPiecesY);
	CDIllusionExplosion *pPart;
	double v;
	int i, j;
	for (i = 0; i < myConfig.iExplodeNbPiecesX; i ++)
	{
		for (j = 0; j < myConfig.iExplodeNbPiecesY; j ++)
		{
			pPart = &pData->pExplosionPart[i*myConfig.iExplodeNbPiecesY+j];
			pPart->fRotationSpeed = 2 * g_random_double ();  // au plus 2 tours sur lui-meme.
			pPart->vz = vmax * (2 * g_random_double () - 1);
			v = sqrt (1 - pPart->vz * pPart->vz);
			pPart->vx = v * (1 + .2 * (2 * g_random_double () - 1)) * sqrt (2)/2;
			pPart->vy = sqrt (1 - pPart->vx * pPart->vx);
		}
	}
	
	return TRUE;
}


gboolean cd_illusion_update_explode (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	pData->iExplosionCount ++;
	pData->fExplosionRadius = (1 + myConfig.fExplosionRadius * pData->iExplosionCount / myConfig.iExplodeDuration * pData->fExplodeDeltaT);
	pData->fExplosionRotation = 360. * pData->iExplosionCount / myConfig.iExplodeDuration * pData->fExplodeDeltaT;
	pData->fExplodeAlpha = MAX (0., 1 - pData->iExplosionCount * pData->fExplodeDeltaT / myConfig.iExplodeDuration);
	
	if (pData->iExplosionCount * pData->fExplodeDeltaT > myConfig.iExplodeDuration)
	{
		cairo_dock_update_removing_inserting_icon_size_default (pIcon);
	}
	
	return (pIcon->fPersonnalScale > .05);
}

void cd_illusion_draw_explode_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	if (pData->fExplodeAlpha == 0)
		return ;
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (1., 1., 1., pData->fExplodeAlpha);
	
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, pIcon->iIconTexture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	if (myConfig.bExplodeCube)
	{
		glEnable(GL_DEPTH_TEST);
		glPolygonMode (GL_FRONT, GL_FILL);
	}
	else
	{
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	}
	
	double fWidth = pIcon->fWidth * pIcon->fScale;
	double fHeight = pIcon->fHeight * pIcon->fScale;
	double dTexCoordX = 1. / myConfig.iExplodeNbPiecesX;
	double dTexCoordY = 1. / myConfig.iExplodeNbPiecesY;
	double x, y, z=0, u, v, u_, v_, angle;
	CDIllusionExplosion *pPart;
	double a = .5;
	int i, j;
	for (i = 0; i < myConfig.iExplodeNbPiecesX; i ++)
	{
		for (j = 0; j < myConfig.iExplodeNbPiecesY; j ++)
		{
			pPart = &pData->pExplosionPart[i*myConfig.iExplodeNbPiecesY+j];
			
			u = i * dTexCoordX;
			v = j * dTexCoordY;
			u_ = u + dTexCoordX;
			v_ = v + dTexCoordY;
			x = pData->fExplosionRadius * (u - .5 + dTexCoordX/2) * pPart->vx;
			y = pData->fExplosionRadius * (.5 - v - dTexCoordY/2) * pPart->vy;
			z = .5 * (pData->fExplosionRadius - 1) * pPart->vz;
			angle = pPart->fRotationSpeed * pData->fExplosionRotation;
			glPushMatrix ();
			
			glTranslatef (x * fWidth, y * fHeight, 0.);
			glRotatef (angle, 0., 1., 0.);
			glRotatef (angle, 1., 0., 0.);
			glScalef (fWidth / myConfig.iExplodeNbPiecesX * (1 + z), fHeight / myConfig.iExplodeNbPiecesY * (1 + z), fHeight / myConfig.iExplodeNbPiecesY * (1 + z));
			
			glBegin(GL_QUADS);
			if (myConfig.bExplodeCube)
			{
				glNormal3f(0,0,1);
				glTexCoord2f (u, v); glVertex3f(-a,  a,  a);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (u_, v); glVertex3f( a,  a, a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (u_, v_); glVertex3f( a, -a, a);  // Top Right Of The Texture and Quad
				glTexCoord2f (u, v_); glVertex3f(-a, -a, a);  // Top Left Of The Texture and Quad
				// Back Face
				glNormal3f(0,0,-1);
				glTexCoord2f (u_, v); glVertex3f( -a, a, -a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (u_, v_); glVertex3f( -a, -a, -a);  // Top Right Of The Texture and Quad
				glTexCoord2f (u, v_); glVertex3f(a, -a, -a);  // Top Left Of The Texture and Quad
				glTexCoord2f (u, v); glVertex3f(a, a, -a);  // Bottom Left Of The Texture and Quad
				// Top Face
				glNormal3f(0,1,0);
				glTexCoord2f (u, v_); glVertex3f(-a,  a,  a);  // Top Left Of The Texture and Quad
				glTexCoord2f (u, v); glVertex3f(-a,  a, -a);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (u_, v); glVertex3f( a,  a, -a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (u_, v_); glVertex3f( a,  a,  a);  // Top Right Of The Texture and Quad
				// Bottom Face
				glNormal3f(0,-1,0);
				glTexCoord2f (u_, v_); glVertex3f( a, -a, -a);  // Top Right Of The Texture and Quad
				glTexCoord2f (u, v_); glVertex3f(-a, -a, -a);  // Top Left Of The Texture and Quad
				glTexCoord2f (u, v); glVertex3f(-a, -a,  a);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (u_, v); glVertex3f( a, -a,  a);  // Bottom Right Of The Texture and Quad
				// Right face
				glNormal3f(1,0,0);
				glTexCoord2f (u_, v);  glVertex3f( a,  a, -a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (u_, v_);  glVertex3f( a, -a, -a);  // Top Right Of The Texture and Quad
				glTexCoord2f (u, v_);  glVertex3f( a, -a,  a);  // Top Left Of The Texture and Quad
				glTexCoord2f (u, v);  glVertex3f( a,  a,  a);  // Bottom Left Of The Texture and Quad
				// Left Face
				glNormal3f(-1,0,0);
				glTexCoord2f (u, v);  glVertex3f(-a,  a, -a);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (u_, v);  glVertex3f(-a,  a,  a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (u_, v_);  glVertex3f(-a, -a,  a);  // Top Right Of The Texture and Quad
				glTexCoord2f (u, v_);  glVertex3f(-a, -a, -a);  // Top Left Of The Texture and Quad
			}
			else
			{
				glNormal3f(0,0,1);
				glTexCoord2f (u, v); glVertex3f(-a,  a,  0.);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (u_, v); glVertex3f( a,  a, 0.);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (u_, v_); glVertex3f( a, -a, 0.);  // Top Right Of The Texture and Quad
				glTexCoord2f (u, v_); glVertex3f(-a, -a, 0.);  // Top Left Of The Texture and Quad
			}
			glEnd();
			
			glPopMatrix ();
		}
	}
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
	glDisable (GL_DEPTH_TEST);
}
