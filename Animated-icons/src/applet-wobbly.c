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
static GLfloat pColorPts[2][2][4] = {{{0., 0., 0., 0.}, {0., 1., 1., 0.}}, {{1., 1., 1., 0.}, {1., 1., 1., 0.}}};


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

/*y' = f(t, y),
=> y_{n+1} = y_n + {h / 6} (k_1 + 2k_2 + 2k_3 + k_4)
où
    k_1 = f ( t_n, y_n )
    k_2 = f ( t_n + {h / 2}, y_n + {h / 2} k_1 )
    k_3 = f ( t_n + {h / 2}, y_n + {h / 2} k_2 )
    k_4 = f ( t_n + h, y_n + h k_3)
L'idée est que la valeur suivante (yn+1) est approchée par la somme de la valeur actuelle (yn) et du produit de la taille de l'intervalle (h) par la pente estimée. La pente est obtenue par une moyenne pondérée de pentes :
    * k1 est la pente au début de l'intervalle ;
    * k2 est la pente au milieu de l'intervalle, en utilisant la pente k1 pour calculer la valeur de y au point tn + h/2 par le biais de la méthode d'Euler ;
    * k3 est de nouveau la pente au milieu de l'intervalle, mais obtenue cette fois en utilisant la pente k2 pour calculer y;
    * k4 est la pente à la fin de l'intervalle, avec la valeur de y calculée en utilisant k3.
Dans la moyenne des quatre pentes, un poids plus grand est donné aux pentes au point milieu.
    pente = {k_1 + 2k_2 + 2k_3 + k_4} / 6*/

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
	glPopMatrix ();
	
	if (pDock->bUseReflect)
	{
		glPushMatrix ();
		double x0, y0, x1, y1;
		double fReflectRatio = myIcons.fReflectSize / pIcon->fHeight / pIcon->fScale;
		double fOffsetY = pIcon->fHeight * pIcon->fScale/2 + myIcons.fReflectSize/2 + pIcon->fDeltaYReflection;
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
				glScalef (pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, myIcons.fReflectSize, 1.);
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
				glScalef (- myIcons.fReflectSize, pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, 1.);
				x0 = 1. - fReflectRatio;
				y0 = 0.;
				x1 = 1.;
				y1 = 1.;
			}
			else
			{
				glTranslatef (- fOffsetY, 0., 0.);
				glScalef (myIcons.fReflectSize, pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, 1.);
				x0 = fReflectRatio;
				y0 = 0.;
				x1 = 0.;
				y1 = 1.;
			}
		}
		glDisable(GL_TEXTURE_2D);
		
		///glActiveTextureARB(GL_TEXTURE0_ARB); // Go pour le multitexturing 1ere passe
		glEnable(GL_TEXTURE_2D); // On active le texturing sur cette passe
		glBindTexture(GL_TEXTURE_2D, pIcon->iIconTexture);
		glColor4f(1.0f, 1.0f, 1.0f, 1.);  // transparence du reflet.
		glEnable(GL_BLEND);
		glBlendFunc (1, 0);
		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glEnable(GL_MAP2_TEXTURE_COORD_2);
		glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
			0, 1, 4, 2, &pTexPts[0][0][0]);
		
		/*glActiveTextureARB(GL_TEXTURE1_ARB); // Go pour le texturing 2eme passe
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, g_pGradationTexture[pDock->bHorizontalDock]);
		glColor4f(1.0f, 1.0f, 1.0f, myIcons.fAlbedo * pIcon->fAlpha);  // transparence du reflet.  // myIcons.fAlbedo * pIcon->fAlpha
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // Le mode de combinaison des textures
		//glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);  // multiplier les alpha.
		glEnable(GL_MAP2_TEXTURE_COORD_2);
		glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
			0, 1, 4, 2, &pTexPts[0][0][0]);*/
		glColor4f(1.0f, 1.0f, 1.0f, myIcons.fAlbedo * pIcon->fAlpha);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glMap2f(GL_MAP2_COLOR_4, 0, 1, 2, 4,
			0, 1, 8, 4, &pColorPts[0][0][0]);
		
		glEvalMesh2(GL_FILL, 0, myConfig.iNbGridNodes, myConfig.iNbGridNodes*y0, myConfig.iNbGridNodes*y1);
		/*glBegin(GL_QUADS);
		glNormal3f(0,0,1);
		glMultiTexCoord2fARB (GL_TEXTURE0_ARB, x0, y0);
		glMultiTexCoord2fARB (GL_TEXTURE1_ARB, 0., 0.);
		glVertex3f (-0.5, .5, 0.);  // Bottom Left Of The Texture and Quad
		
		glMultiTexCoord2fARB (GL_TEXTURE0_ARB, x1, y0);
		glMultiTexCoord2fARB (GL_TEXTURE1_ARB, 1., 0.);
		glVertex3f ( 0.5, .5, 0.);  // Bottom Right Of The Texture and Quad
		
		glMultiTexCoord2fARB (GL_TEXTURE0_ARB, x1, y1);
		glMultiTexCoord2fARB (GL_TEXTURE1_ARB, 1., 1.);
		glVertex3f ( 0.5, -.5, 0.);  // Top Right Of The Texture and Quad
		
		glMultiTexCoord2fARB (GL_TEXTURE0_ARB, x0, y1);
		glMultiTexCoord2fARB (GL_TEXTURE1_ARB, 0., 1.);
		glVertex3f (-0.5, -.5, 0.);  // Top Left Of The Texture and Quad
		glEnd();*/
		
		/**glActiveTextureARB(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);*/
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		
		glPopMatrix ();
	}
	glDisable(GL_MAP2_VERTEX_3);
	glDisable(GL_MAP2_TEXTURE_COORD_2);
	glDisable(GL_TEXTURE_2D);
}
