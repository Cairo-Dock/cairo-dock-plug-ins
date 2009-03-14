/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-break.h"

#define xctrl(j) pCtrlPoints[2*(j)]
#define yctrl(j) pCtrlPoints[2*(j)+1]
#define xpart(j) pPart->pCoords[2*(j)]
#define ypart(j) pPart->pCoords[2*(j)+1]

gboolean cd_illusion_init_break (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData, double dt)
{
	pData->fBreakDeltaT = dt;
	pData->iBreakCount = 0;
	
	int iNbCtrlPts = 4 + (2 * myConfig.iBreakNbBorderPoints) + (2 * myConfig.iBreakNbBorderPoints + 1);
	double *pCtrlPoints = g_new0 (double, 2 * iNbCtrlPts);
	//g_print ("iNbCtrlPts : %d\n", iNbCtrlPts);
	xctrl(0) = 0.;
	yctrl(0) = 1.;
	xctrl(1) = 1.;
	yctrl(1) = 1.;
	double f, f0 = 1. / (myConfig.iBreakNbBorderPoints + 1);  // f0 = fraction moyenne.
	int i,j=2;
	for (i = 0; i < 2*myConfig.iBreakNbBorderPoints+1; i ++, j++)
	{
		// un nouveau point sur l'un des cotes.
		if (i == 2*myConfig.iBreakNbBorderPoints)  // dernier coup.
			f = 1.;
		else
			f = f0 * (.5 + g_random_double ());  // entre .5f0 et 1.5f0, sachant que 1.5f0 <= 3/4 < 1
		xctrl(j) = ((j>>1) & 1);
		yctrl(j) = (1 - f) * (j > 3 ? yctrl(j-4) : yctrl(0));
		g_print ("yctrl(j) : %.3f (%.2f)\n", yctrl(j), f);
		
		// une brisure au milieu du segment forme.
		j ++;
		f = g_random_double ();
		xctrl(j) = f;
		yctrl(j) = f * yctrl(j-1) + (1 - f) * yctrl(j-2);
	}
	xctrl(j) = ((j>>2) & 1);  // en fait c'est toujours egal a 0.
	yctrl(j) = 0.;
	
	pData->iNbBreakParts = 2 + (2 * myConfig.iBreakNbBorderPoints + 1);
	pData->pBreakPart = g_new0 (CDIllusionBreak, pData->iNbBreakParts);
	CDIllusionBreak *pPart;
	for (i = 0; i < pData->iNbBreakParts; i ++)
	{
		pPart = &pData->pBreakPart[i];
		if (i == 0)
		{
			pPart->iNbPts = 3;
			xpart(0) = xctrl(0);
			ypart(0) = yctrl(0);
			xpart(1) = xctrl(1);
			ypart(1) = yctrl(1);
			xpart(2) = xctrl(2);
			ypart(2) = yctrl(2);
		}
		else if (i == 1)
		{
			pPart->iNbPts = 3;
			xpart(0) = xctrl(0);
			ypart(0) = yctrl(0);
			xpart(1) = xctrl(3);
			ypart(1) = yctrl(3);
			xpart(2) = xctrl(4);
			ypart(2) = yctrl(4);
		}
		else if (i == pData->iNbBreakParts - 1)
		{
			pPart->iNbPts = 3;
			xpart(0) = xctrl(iNbCtrlPts-3);
			ypart(0) = yctrl(iNbCtrlPts-3);
			xpart(1) = xctrl(iNbCtrlPts-2);
			ypart(1) = yctrl(iNbCtrlPts-2);
			xpart(2) = xctrl(iNbCtrlPts-1);
			ypart(2) = yctrl(iNbCtrlPts-1);
		}
		else
		{
			pPart->iNbPts = 4;
			xpart(0) = xctrl(2*(i-1));
			ypart(0) = yctrl(2*(i-1));
			xpart(1) = xctrl(2*(i-1)+1);
			ypart(1) = yctrl(2*(i-1)+1);
			xpart(2) = xctrl(2*(i-1)+3);
			ypart(2) = yctrl(2*(i-1)+3);
			xpart(3) = xctrl(2*(i-1)+4);
			ypart(3) = yctrl(2*(i-1)+4);
		}
		
		pPart->yinf = MIN (MIN (ypart(0), ypart(1)), ypart(2));
		if (pPart->iNbPts == 4)
			pPart->yinf = MIN (pPart->yinf, ypart(3));
		pPart->fRotationAngle = 5 + g_random_double () * 15.;  // ca separe un peu les morceaux, pour donner l'effet de brisure des le debut.
	}
	
	return TRUE;
}


gboolean cd_illusion_update_break (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	pData->iBreakCount ++;
	
	int iWidth, iHeight;
	cairo_dock_get_icon_extent (pIcon, pDock, &iWidth, &iHeight);
	double fSizeX, fSizeY;
	cairo_dock_get_current_icon_size (pIcon, pDock, &fSizeX, &fSizeY);
	
	double t_ = (pData->iBreakCount * pData->fBreakDeltaT / myConfig.iBreakDuration);  // t/T
	pData->dh = t_ * t_;  // dh = 1/2 * g * t^2, avec g = 2/T^2 (hauteur comptee unitairement).
	
	double yinf;
	CDIllusionBreak *pPart;
	int i, j;
	for (i = 0; i < pData->iNbBreakParts; i ++)
	{
		pPart = &pData->pBreakPart[i];
		if (pPart->yinf - pData->dh < 0)  // on a touche le sol.
		{
			pPart->fRotationAngle += pData->fBreakDeltaT / (.25 * myConfig.iBreakDuration) * 90.;  // formule faite a la va-vite, avec une acceleration ca serait mieux ...
		}
	}
	
	if (pData->iBreakCount * pData->fBreakDeltaT > myConfig.iBreakDuration)
	{
		cairo_dock_update_removing_inserting_icon_size_default (pIcon);
	}
	
	cairo_dock_redraw_icon (pIcon, CAIRO_CONTAINER (pDock));
	return (pIcon->fPersonnalScale > .05);
}

void cd_illusion_draw_break_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (1.);
	glBindTexture (GL_TEXTURE_2D, pIcon->iIconTexture);
	
	double fSizeX, fSizeY;
	cairo_dock_get_current_icon_size (pIcon, pDock, &fSizeX, &fSizeY);
	
	glPushMatrix ();
	glTranslatef (-fSizeX/2, -fSizeY/2, 0.);
	glMatrixMode(GL_TEXTURE); // On selectionne la matrice des textures
	glPushMatrix ();
	glLoadIdentity(); // On la reset
	glScalef (1., -1., 1.);
	glMatrixMode(GL_MODELVIEW);
	
	CDIllusionBreak *pPart;
	double xt, yt;  // coordonnees d'un point de texture
	double x, y;  // coordonnees du vertex associe.
	double dh = pData->dh;
	int i, j;
	for (i = 0; i < pData->iNbBreakParts; i ++)
	{
		pPart = &pData->pBreakPart[i];
		if (pPart->fRotationAngle > 90)  // il est a plat par terre, on ne le voit plus.
			continue;
		
		if (pPart->fRotationAngle != 0)
		{
			glPushMatrix ();
			glRotatef (pPart->fRotationAngle, 1., 0., 0.);
		}
		
		if (pPart->iNbPts == 3)
			glBegin(GL_TRIANGLES);
		else
			glBegin(GL_QUADS);
		
		for (j = 0; j < pPart->iNbPts; j ++)
		{
			xt = xpart(j);
			yt = ypart(j);
			
			x = xt * fSizeX;
			if (dh > pPart->yinf)
				y = (yt - pPart->yinf) * fSizeY;
			else
				y = (yt - dh) * fSizeY;
			
			glTexCoord2f (xt, yt);
			glVertex3f (x,  y,  0.);
		}
		
		glEnd ();
		
		if (pPart->fRotationAngle != 0)
		{
			glPopMatrix ();
		}
	}
	
	glPopMatrix ();
	glMatrixMode(GL_TEXTURE); // On selectionne la matrice des textures
	glPopMatrix ();
	glMatrixMode(GL_MODELVIEW);
	_cairo_dock_disable_texture ();
}
