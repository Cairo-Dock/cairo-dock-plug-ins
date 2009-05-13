/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-black-hole.h"

#define SPIRAL_NB_PTS 31  // 1+2*15

static inline void _update_coords (CDIllusionData *pData)
{
	//g_print ("%s ()\n", __func__);
	int i, j, n=0;  // parcours des carre.
	int k, ix, iy;  // parcours des coins.
	CDIllusionBlackHole *pPoint;  // point de la grille correspondant au coin courant.
	for (j = 0; j < SPIRAL_NB_PTS-1; j ++)
	{
		for (i = 0; i < SPIRAL_NB_PTS-1; i ++)
		{
			//g_print (" %d) %d;%d\n", n, i, j);
			for (k = 0; k < 4; k ++)
			{
				ix = ((k+1)&2)/2;  // 0,1,1,0
				iy = (k&2)/2;  // 0,0,1,1
				
				//g_print ("   %d) %d;%d\n", k, ix, iy);
				pPoint = &pData->pBlackHolePoints[(j+iy) * SPIRAL_NB_PTS + (i+ix)];
				//g_print ("   -> point %d/%d, coord %d/%d\n", (j+iy) * SPIRAL_NB_PTS + (i+ix), SPIRAL_NB_PTS * SPIRAL_NB_PTS, 2*(4*n+k)+1, 8 * (SPIRAL_NB_PTS - 1) * (SPIRAL_NB_PTS - 1));
				pData->pBlackHoleCoords[2*(4*n+k)] = pPoint->u;
				pData->pBlackHoleCoords[2*(4*n+k)+1] = pPoint->v;
				
				pData->pBlackHoleVertices[2*(4*n+k)] = pPoint->x;
				pData->pBlackHoleVertices[2*(4*n+k)+1] = pPoint->y;
			}
			
			n ++;
		}
	}
	//g_print ("done.\n");
}

gboolean cd_illusion_init_black_hole (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData, double dt)
{
	pData->pBlackHolePoints = g_new0 (CDIllusionBlackHole, SPIRAL_NB_PTS * SPIRAL_NB_PTS);
	pData->pBlackHoleCoords = g_new0 (GLfloat, 8 * (SPIRAL_NB_PTS - 1) * (SPIRAL_NB_PTS - 1));
	pData->pBlackHoleVertices = g_new0 (GLfloat, 8 * (SPIRAL_NB_PTS - 1) * (SPIRAL_NB_PTS - 1));
	
	int i, j, n=0;
	double u, v, x, y;
	CDIllusionBlackHole *pPoint;
	for (j = 0; j < SPIRAL_NB_PTS; j ++)  // bas -> haut.
	{
		v = (double) j / SPIRAL_NB_PTS;
		y = v - .5;
		for (i = 0; i < SPIRAL_NB_PTS; i ++)  // gauche -> droite.
		{
			u = (double) i / SPIRAL_NB_PTS;
			x = u - .5;
			
			pPoint = &pData->pBlackHolePoints[n];  // n = j*N+i
			pPoint->u = u;
			pPoint->v = v;
			pPoint->x = x;
			pPoint->y = y;
			pPoint->fTheta0 = atan2 (y, x);
			pPoint->r0 = sqrt (x*x + y*y);
			
			n ++;
		}
	}
	
	_update_coords (pData);
	
	pData->fBlackHoleDeltaT = dt;
	return TRUE;
}


gboolean cd_illusion_update_black_hole (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	pData->fBlackHoleTime += pData->fBlackHoleDeltaT;
	//g_print ("t <- %f\n", pData->fBlackHoleTime);
	
	double fOmega0 = 2*G_PI*myConfig.fBlackHoleRotationSpeed;
	double r, R = sqrt(2)/2;
	double T = myConfig.iBlackHoleDuration;
	double t = MIN (T, pData->fBlackHoleTime);
	double a = myConfig.iAttraction;
	
	int i, j, n=0;
	CDIllusionBlackHole *pPoint;
	for (j = 0; j < SPIRAL_NB_PTS; j ++)
	{
		for (i = 0; i < SPIRAL_NB_PTS; i ++)
		{
			pPoint = &pData->pBlackHolePoints[n];
			r = pPoint->r0;
			r = pow (r / R, 1 + a*t/T) * R;  // effet "trou noir".
			pPoint->fTheta = pPoint->fTheta0 + fOmega0 * t * 1e-3 * (1 - (r / R) * (1 - .5 * t / T));  // w = w0 - k(t)*r, avec k tel que w(R,0) = 0 et w(R,T) = w0.
			pPoint->x = r * cos (pPoint->fTheta);
			pPoint->y = r * sin (pPoint->fTheta);
			
			n ++;
		}
	}
	
	_update_coords (pData);
	
	if (pData->fBlackHoleTime > T)
		cairo_dock_update_removing_inserting_icon_size_default (pIcon);
	
	cairo_dock_redraw_icon (pIcon, pDock);
	return (pIcon->fPersonnalScale > .05);
}

static float fCapsuleObjectPlaneS[4] = { 0.5f, 0., 0., 0. }; // pour un plaquages propre des textures
static float fCapsuleObjectPlaneT[4] = { 0., 0.5f, 0., 0. };  // le 2 c'est le 'c'.
void cd_illusion_draw_black_hole_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
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
	
	
	//glTexGenfv(GL_S, GL_OBJECT_PLANE, fCapsuleObjectPlaneS); // Je decale un peu la texture
	//glTexGenfv(GL_T, GL_OBJECT_PLANE, fCapsuleObjectPlaneT);
	/*glEnable(GL_TEXTURE_GEN_S);                                // oui je veux une generation en S
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR); // la je veux un mapping tout ce qu'il y a de plus classique
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);*/
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // pour les bouts de textures qui depassent.
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnableClientState (GL_VERTEX_ARRAY);
	
	glTexCoordPointer (2, GL_FLOAT, 2 * sizeof(GLfloat), pData->pBlackHoleCoords);
	glVertexPointer (2, GL_FLOAT, 2 * sizeof(GLfloat), pData->pBlackHoleVertices);

	glDrawArrays (GL_QUADS, 0, 4 * (SPIRAL_NB_PTS - 1) * (SPIRAL_NB_PTS - 1));
	
	
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	
	
	glPopMatrix ();
	
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
}
