/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-wobbly.h"

#define l0 .33


static GLfloat pTexPts[2][2][2] = {{{0.0, 0.0}, {1.0, 0.0}}, {{0.0, 1.0}, {1.0, 1.0}}};


void cd_animations_init_wobbly (CDAnimationData *pData)
{
	double x, y, cx, cy;
	int i,j;
	for (i=0; i<4; i++)
	{
		x = (i-1.5)/3;
		cx = 1 + fabs (i-1.5) / 3;  // 1.5 -> 1
		for (j=0; j<4; j++)
		{
			y = - (j-1.5)/3;
			cy = 1 + fabs (j-1.5) / 3;  // 1.5 -> 1
			switch (myConfig.iInitialStrecth)
			{
				case CD_HORIZONTAL_STRECTH :
					pData->gridNodes[i][j].x = x * cx * cy;
					pData->gridNodes[i][j].y = y * cy;
				break ;
				
				case CD_VERTICAL_STRECTH :
					pData->gridNodes[i][j].x = x * cx;
					pData->gridNodes[i][j].y = y * cy * cx;
				break ;
				
				case CD_CORNER_STRECTH :
					pData->gridNodes[i][j].x = x * cx * cy/sqrt(2);
					pData->gridNodes[i][j].y = y * cy * cx/sqrt(2);
				break ;
				
			}
			
			/*if ((i == 0 || i == 3) && (j == 0 || j == 3))
				pData->gridNodes[i][j].x *= (1. + g_random_double ()/2);
			if ((i == 0 || i == 3) && (j == 0 || j == 3))
				pData->gridNodes[i][j].y *= (1. + g_random_double ()/2);*/
			
			pData->gridNodes[i][j].vx = 0.;
			pData->gridNodes[i][j].vy = 0.;
		}
	}
	pData->bIsWobblying = TRUE;
}

#define _pulled_by(i,j)\
	pNode2 = &pData->gridNodes[i][j];\
	dx = pNode2->x - pNode->x;\
	dy = pNode2->y - pNode->y;\
	l = sqrt (dx*dx + dy*dy);\
	if (l > 1.5e6*l0) {\
	dx /= l/(l0/2/sqrt(2));\
	dy /= l/(l0/2/sqrt(2));\
	l = 1.5*l0;}\
	pNode->fx += k * dx * (1. - l0 / l);\
	pNode->fy += k * dy * (1. - l0 / l);\
	if (!bContinue && fabs (l - l0) > .005) bContinue = TRUE;

gboolean cd_animations_update_wobbly (CDAnimationData *pData)
{
	const int n = 20;
	double k = myConfig.fSpringConstant;
	double f = myConfig.fFriction;
	double dt = g_iGLAnimationDeltaT / 1e3 / n;
	CDAnimationGridNode *pNode, *pNode2;
	gboolean bContinue = FALSE;
	
	double dx, dy, l;
	int i,j,m;
	for (m=0; m<n; m++)
	{
		for (i=0; i<4; i++)
		{
			for (j=0; j<4; j++)
			{
				pNode = &pData->gridNodes[i][j];
				pNode->fx = 0.;
				pNode->fy = 0.;
				
				if (i > 0)
				{
					_pulled_by (i-1, j);
				}
				if (i < 3)
				{
					_pulled_by (i+1, j);
				}
				if (j > 0)
				{
					_pulled_by (i, j-1);
				}
				if (j < 3)
				{
					_pulled_by (i, j+1);
				}
			}
		}
		
		double _vx, _vy;
		for (i=0; i<4; i++)
		{
			for (j=0; j<4; j++)
			{
				pNode = &pData->gridNodes[i][j];
				pNode->fx -= f * pNode->vx;
				pNode->fy -= f * pNode->vy;
				
				_vx = pNode->vx;
				_vy = pNode->vy;
				pNode->vx += pNode->fx * dt;  // Runge-Kutta d'ordre 1.
				pNode->vy += pNode->fy * dt;
				
				pNode->x += (pNode->vx + _vx)/2 * dt;
				pNode->y += (pNode->vy + _vy)/2 * dt;
				
				//pData->pCtrlPts[j][i][0] = pNode->x;
				//pData->pCtrlPts[j][i][1] = pNode->y;
			}
		}
	}
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			pNode = &pData->gridNodes[i][j];
			pData->pCtrlPts[j][i][0] = pNode->x;
			pData->pCtrlPts[j][i][1] = pNode->y;
		}
	}
	
	return bContinue;
}



void cd_animations_draw_wobbly_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData)
{
	glPushMatrix ();
	cairo_dock_set_icon_scale (pIcon, pDock, 1.);
	
	glColor4f (1., 1., 1., 1.);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glEnable(GL_TEXTURE_2D); // Je veux de la texture
	glBindTexture(GL_TEXTURE_2D, pIcon->iIconTexture);
	
	glEnable(GL_MAP2_VERTEX_3);  // active l'evaluateur 2D des sommets 3D
	glEnable(GL_MAP2_TEXTURE_COORD_2);
	
	glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4,
		0, 1, 12, 4, &pData->pCtrlPts[0][0][0]);
	glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
		0, 1, 4, 2, &pTexPts[0][0][0]);
	
	glMapGrid2f(myConfig.iNbGridNodes, 0.0, 1.0, myConfig.iNbGridNodes, 0.0, 1.0);  // Pour definir une grille reguliere de 0.0 a 1.0 en n etapes en u et m etapes en v
	glEvalMesh2(GL_FILL, 0, myConfig.iNbGridNodes, 0, myConfig.iNbGridNodes);  // Pour appliquer cette grille aux evaluateurs actives.
	glDisable(GL_MAP2_VERTEX_3);
	glDisable(GL_MAP2_TEXTURE_COORD_2);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix ();
}
