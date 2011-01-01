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
#include "applet-lightning.h"

#include "evaporate-tex.h"

gboolean cd_illusion_init_lightning (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	pData->iNbSources = myConfig.iLightningNbSources;
	pData->pLightnings = g_new0 (CDIllusionLightning, pData->iNbSources);
	pData->iNbVertex = myConfig.iLightningNbCtrlPts + 2;
	
	CDIllusionLightning *l;
	int i, j;
	for (i = 0; i < pData->iNbSources; i ++)
	{
		l = &pData->pLightnings[i];
		l->iNbCurrentVertex = 2;
		l->pVertexTab = g_new0 (GLfloat, 2 * pData->iNbVertex);
		for (j = 0; j < pData->iNbVertex; j ++)
		{
			l->pVertexTab[2*j+1] = - (double) j / (pData->iNbVertex-1);  // on part d'en haut et on descend.
		}
	}
	
	if (myData.iLightningTexture == 0)
		myData.iLightningTexture = cairo_dock_load_texture_from_raw_data (evaporateTex, 32, 1);
	
	return TRUE;
}


void cd_illusion_update_lightning (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	int iWidth, iHeight;
	cairo_dock_get_icon_extent (pIcon, CAIRO_CONTAINER (pDock), &iWidth, &iHeight);
	double fSizeX, fSizeY;
	cairo_dock_get_current_icon_size (pIcon, CAIRO_CONTAINER (pDock), &fSizeX, &fSizeY);
	
	double f = 1 - MIN (1., pData->fTime / myConfig.iLightningDuration);
	CDIllusionLightning *l;
	double xbase, dx = .05;  // 5% de la largeur a chaque petite branche.
	double xsource, ximpact; 
	double alpha_s, alpha_t;  // correction pour aller plus a gauche ou plus a droite.
	int sign;
        int dt = cairo_dock_get_animation_delta_t (CAIRO_CONTAINER (pDock));
	int Nt = myConfig.iLightningDuration / dt, Nv = pData->iNbVertex;
	double xt, xs;
	int i, j;
	for (i = 0; i < pData->iNbSources; i ++)
	{
		l = &pData->pLightnings[i];
		
		xbase = (pData->iNbSources != 1 ? 2. * i / (pData->iNbSources - 1) - 1 : 0.);  // [-1, 1]
		xsource = f*xbase/2;
		ximpact = f*xbase;
		sign = (xbase < 0 ? -1 : 1);
		
		alpha_t = 2 * (xbase/2) / (Nt * dx);  // correlation temporelle.
		alpha_s = 2 * (ximpact - xsource) / (Nv * dx);  // correlation spatiale.
		
		l->pVertexTab[2*0] = xsource;
		for (j = 1; j < pData->iNbVertex; j ++)
		{
			xt = l->pVertexTab[2*j] + sign * (g_random_boolean () ? 1 + alpha_t * j/Nv : -1) * dx;  // correlation temporelle.
			xs = l->pVertexTab[2*(j-1)] + (g_random_boolean () ? 1 + alpha_s : -1) * dx;  // correlation spatiale.
			l->pVertexTab[2*j] = (xs + xt) / 2;  // correlation spatio-temporelle (et ouais).
		}
		l->pVertexTab[2*j] = ximpact;
		l->iNbCurrentVertex = MIN (pData->iNbVertex, l->iNbCurrentVertex+1);
	}
	
	pData->fLightningAlpha = MIN (1, .2 + sqrt (f));
	
	cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));
}

void cd_illusion_draw_lightning_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData)
{
	//pIcon->fAlpha = pData->fLightningAlpha;
	//cairo_dock_draw_icon_texture (pIcon, CAIRO_CONTAINER (pDock));
	_cairo_dock_enable_texture ();
	_cairo_dock_set_alpha (pIcon->fAlpha);
	_cairo_dock_set_blend_over ();
	
	glBindTexture(GL_TEXTURE_2D, pIcon->iIconTexture);
	
	double fSizeX, fSizeY;
	cairo_dock_get_current_icon_size (pIcon, CAIRO_CONTAINER (pDock), &fSizeX, &fSizeY);
	double f = pData->fLightningAlpha;
	 _cairo_dock_apply_current_texture_portion_at_size_with_offset(0, 0,
	 	1, f,
	 	fSizeX, f * fSizeY,
	 	0, 0);
	
	_cairo_dock_disable_texture ();
	
	
	int iWidth, iHeight;
	cairo_dock_get_icon_extent (pIcon, CAIRO_CONTAINER (pDock), &iWidth, &iHeight);
	
	glPushMatrix ();
	glTranslatef (0., - fSizeY/2, 0.);  // en bas au milieu.
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	double fScale = (1 + fDockMagnitude * myIconsParam.fAmplitude) / (1 + myIconsParam.fAmplitude);
	glTranslatef (0., iHeight * fScale, 0.);  // en haut du dock, au milieu de l'icone.
	
	glScalef (iWidth/2 * fScale, iHeight * fScale, 1.);
	
	
	glPolygonMode(GL_FRONT, GL_LINE);
	glEnable (GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	_cairo_dock_set_blend_over ();
	glLineWidth (1);
	glColor4f (myConfig.fLightningColor1[0], myConfig.fLightningColor1[1], myConfig.fLightningColor1[2], myConfig.fLightningColor1[3]);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	//glEnable(GL_TEXTURE_1D);
	//glBindTexture (GL_TEXTURE_1D, myData.iLightningTexture);
	
	CDIllusionLightning *l;
	GLfloat *pVertexTab;
	int i;
	for (i = 0; i < pData->iNbSources; i ++)
	{
		l = &pData->pLightnings[i];
		
		///_cairo_dock_set_vertex_pointer (l->pVertexTab);
		glDrawArrays (GL_LINE_STRIP, 0, l->iNbCurrentVertex);
	}
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
	
	//glDisable(GL_TEXTURE_1D);
	
	glPopMatrix ();
}
