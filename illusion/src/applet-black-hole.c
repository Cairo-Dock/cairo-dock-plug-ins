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

static inline void _calculate_grid (CDIllusionData *pData)
{
	double fOmega0 = 2*G_PI*myConfig.fBlackHoleRotationSpeed;
	double r, R = sqrt(2)/2;
	double T = myConfig.iBlackHoleDuration;
	double t = pData->fTime;
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
			pPoint->y = - r * sin (pPoint->fTheta);  // le (-) est la pour palier a l'inversion de la texture par cairo.
			
			n ++;
		}
	}
}

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

gboolean cd_illusion_init_black_hole (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	pData->pBlackHolePoints = g_new0 (CDIllusionBlackHole, SPIRAL_NB_PTS * SPIRAL_NB_PTS);
	pData->pBlackHoleCoords = g_new0 (GLfloat, 8 * (SPIRAL_NB_PTS - 1) * (SPIRAL_NB_PTS - 1));
	pData->pBlackHoleVertices = g_new0 (GLfloat, 8 * (SPIRAL_NB_PTS - 1) * (SPIRAL_NB_PTS - 1));
	
	int i, j, n=0;
	double u, v, x, y, r;
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
			pPoint->fTheta0 = atan2 (y, x);
			pPoint->r0 = sqrt (x*x + y*y);
			
			n ++;
		}
	}
	
	_calculate_grid (pData);
	
	_update_coords (pData);
	
	return TRUE;
}


void cd_illusion_update_black_hole (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	_calculate_grid (pData);
	
	_update_coords (pData);
	
	cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));
}

static float fCapsuleObjectPlaneS[4] = { 0.5f, 0., 0., 0. }; // pour un plaquages propre des textures
static float fCapsuleObjectPlaneT[4] = { 0., 0.5f, 0., 0. };  // le 2 c'est le 'c'.
void cd_illusion_draw_black_hole_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	_cairo_dock_enable_texture ();
	_cairo_dock_set_alpha (pIcon->fAlpha);
	if (pIcon->fAlpha == 1)
		_cairo_dock_set_blend_over ();
	else
		_cairo_dock_set_blend_alpha ();
	glBindTexture(GL_TEXTURE_2D, pIcon->iIconTexture);
	
	glPushMatrix ();
	cairo_dock_set_icon_scale (pIcon, pDock, 1.);
	
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnableClientState (GL_VERTEX_ARRAY);
	
	glTexCoordPointer (2, GL_FLOAT, 2 * sizeof(GLfloat), pData->pBlackHoleCoords);
	glVertexPointer (2, GL_FLOAT, 2 * sizeof(GLfloat), pData->pBlackHoleVertices);
	glDrawArrays (GL_QUADS, 0, 4 * (SPIRAL_NB_PTS - 1) * (SPIRAL_NB_PTS - 1));
	
	glPopMatrix ();
	
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_VERTEX_ARRAY);
	_cairo_dock_disable_texture ();
}
