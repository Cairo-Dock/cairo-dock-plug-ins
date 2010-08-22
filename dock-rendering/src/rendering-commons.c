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

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>
#include "rendering-commons.h"

extern cairo_surface_t *my_pFlatSeparatorSurface[2];
extern GLuint my_iFlatSeparatorTexture;
extern int iVanishingPointY;

cairo_surface_t *cd_rendering_create_flat_separator_surface (int iWidth, int iHeight)
{
	cairo_pattern_t *pStripesPattern = cairo_pattern_create_linear (0.0f,
		iHeight,
		0.,
		0.);
	g_return_val_if_fail (cairo_pattern_status (pStripesPattern) == CAIRO_STATUS_SUCCESS, NULL);
	
	cairo_pattern_set_extend (pStripesPattern, CAIRO_EXTEND_REPEAT);
	
	double fStep = 1.;
	double y = 0, h0 = (fStep * (1 + sqrt (1 + 4. * iHeight / fStep)) / 2) - 1, hk = h0;
	int k = 0;
	for (k = 0; k < h0 / fStep; k ++)
	{
		//g_print ("step : %f ; y = %.2f\n", 1.*hk / iHeight, y);
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			0.,
			0.,
			0.,
			0.);
		y += 1.*hk / iHeight;
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			0.,
			0.,
			0.,
			0.);
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			myIcons.fSeparatorColor[0],
			myIcons.fSeparatorColor[1],
			myIcons.fSeparatorColor[2],
			myIcons.fSeparatorColor[3]);
		y += 1.*hk / iHeight;
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			myIcons.fSeparatorColor[0],
			myIcons.fSeparatorColor[1],
			myIcons.fSeparatorColor[2],
			myIcons.fSeparatorColor[3]);
		hk -= fStep;
	}
	
	cairo_surface_t *pNewSurface = cairo_dock_create_blank_surface (
		iWidth,
		iHeight);
	cairo_t *pImageContext = cairo_create (pNewSurface);
	cairo_set_source (pImageContext, pStripesPattern);
	cairo_paint (pImageContext);
	
	cairo_pattern_destroy (pStripesPattern);
	cairo_destroy (pImageContext);
	
	return pNewSurface;
}

void cd_rendering_load_flat_separator (CairoContainer *pContainer)
{
	cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL]);
	cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL]);
	
	my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL] = cd_rendering_create_flat_separator_surface ((g_bUseOpenGL?10:200), (g_bUseOpenGL?50:150));  // en opengl on etire la texture, donc pas besoin de la charger en grand.
	
	if (g_bUseOpenGL)
	{
		if (my_iFlatSeparatorTexture != 0)
			_cairo_dock_delete_texture (my_iFlatSeparatorTexture);
		my_iFlatSeparatorTexture = cairo_dock_create_texture_from_surface (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL]);
		cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL]);
		my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL] = NULL;
		my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL] = NULL;
	}
	else
	{
		my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL] = cairo_dock_rotate_surface (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL], 200, 150, -G_PI / 2);
	}
}



double cd_rendering_interpol (double x, double *fXValues, double *fYValues)
{
	double y;
	int i, i_inf=0, i_sup=RENDERING_INTERPOLATION_NB_PTS-1;
	do
	{
		i = (i_inf + i_sup) / 2;
		if (fXValues[i] < x)
			i_inf = i;
		else
			i_sup = i;
	}
	while (i_sup - i_inf > 1);
	
	double x_inf = fXValues[i_inf];
	double x_sup = fXValues[i_sup];
	return (x_sup != x_inf ? ((x - x_inf) * fYValues[i_sup] + (x_sup - x) * fYValues[i_inf]) / (x_sup - x_inf) : fYValues[i_inf]);
}



void cd_rendering_draw_flat_separator_opengl (Icon *icon, CairoDock *pDock)
{
	double hi = myIcons.fReflectSize * pDock->container.fRatio + myBackground.iFrameMargin;
	double fLeftInclination = (icon->fDrawX - pDock->container.iWidth / 2) / iVanishingPointY;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->container.iWidth / 2) / iVanishingPointY;
	
	double fHeight = pDock->iDecorationsHeight;
	double fBigWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi);
	double fLittleWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi - fHeight);
	
	double fDeltaXLeft = fHeight * fLeftInclination;
	double fDeltaXRight = fHeight * fRightInclination;
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	
	double fDockOffsetX, fDockOffsetY;
	fDockOffsetX = icon->fDrawX - (fHeight - hi) * fLeftInclination;
	fDockOffsetY = fHeight + myBackground.iDockLineWidth;
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (1., 1., 1., 1.);
	
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, my_iFlatSeparatorTexture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glPolygonMode (GL_FRONT, GL_FILL);
	
	if (pDock->container.bIsHorizontal)
	{
		if (! pDock->container.bDirectionUp)
			fDockOffsetY = pDock->container.iHeight - fDockOffsetY;
		
		glTranslatef (fDockOffsetX, fDockOffsetY, 0.);  // coin haut gauche.
		if (! pDock->container.bDirectionUp)
			glScalef (1., -1., 1.);
	}
	else
	{
		if (pDock->container.bDirectionUp)
			fDockOffsetY = pDock->container.iHeight - fDockOffsetY;
		fDockOffsetX = pDock->container.iWidth - fDockOffsetX;
		
		glTranslatef (fDockOffsetY, fDockOffsetX, 0.);
		glRotatef (-90., 0., 0., 1.);
		if (pDock->container.bDirectionUp)
			glScalef (1., -1., 1.);
	}
	
	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.);
	glVertex3f(0., 0., 0.);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1., 0.);
	glVertex3f(fLittleWidth, 0., 0.);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1., 1.);
	glVertex3f(fLittleWidth + fDeltaXRight, - fHeight, 0.);  // Top Right Of The Texture and Quad
	glTexCoord2f(0., 1.);
	glVertex3f(fLittleWidth + fDeltaXRight - fBigWidth, - fHeight, 0.);  // Top Left Of The Texture and Quad
	glEnd();
	
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
}

void cd_rendering_draw_physical_separator_opengl (Icon *icon, CairoDock *pDock, gboolean bBackGround, Icon *prev_icon, Icon *next_icon)
{
	if (prev_icon == NULL)
		prev_icon = icon;
	if (next_icon == NULL)
		next_icon = icon;
	double hi = myIcons.fReflectSize * pDock->container.fRatio + myBackground.iFrameMargin;
	hi = (pDock->container.bDirectionUp ? pDock->container.iHeight - (icon->fDrawY + icon->fHeight * icon->fScale) : icon->fDrawY);
	//g_print ("%s : hi = %.2f/%.2f\n", icon->cName, myIcons.fReflectSize * pDock->container.fRatio + myBackground.iFrameMargin, pDock->container.iHeight - (icon->fDrawY + icon->fHeight * icon->fScale));
	double fLeftInclination = (icon->fDrawX - pDock->container.iWidth / 2) / iVanishingPointY;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->container.iWidth / 2) / iVanishingPointY;
	
	double fHeight, fBigWidth, fLittleWidth;
	if (bBackGround)
	{
		fHeight = pDock->iDecorationsHeight + myBackground.iDockLineWidth - hi;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + 0);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + 0 - fHeight);
	}
	else
	{
		fHeight = hi + myBackground.iDockLineWidth;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi - fHeight);
	}
	double fDeltaXLeft = fHeight * fLeftInclination;
	double fDeltaXRight = fHeight * fRightInclination;
	
	double fDockOffsetX, fDockOffsetY;
	if (bBackGround)
	{
		fDockOffsetX = icon->fDrawX - fHeight * fLeftInclination;
		fDockOffsetY = pDock->iDecorationsHeight + 2*myBackground.iDockLineWidth;
	}
	else
	{
		fDockOffsetX = icon->fDrawX;
		fDockOffsetY = fHeight;
	}
	//g_print ("X : %.2f + %.2f/%.2f ; Y : %.2f + %.2f\n", fDockOffsetX, fBigWidth, fLittleWidth, fDockOffsetY, fHeight);
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ZERO);
	glColor4f (0., 0., 0., 0.);
	
	glPolygonMode (GL_FRONT, GL_FILL);
	
	if (pDock->container.bIsHorizontal)
	{
		if (! pDock->container.bDirectionUp)
			fDockOffsetY = pDock->container.iHeight - fDockOffsetY;
		
		glTranslatef (fDockOffsetX, fDockOffsetY, 0.);  // coin haut gauche.
		if (! pDock->container.bDirectionUp)
			glScalef (1., -1., 1.);
	}
	else
	{
		if (pDock->container.bDirectionUp)
			fDockOffsetY = pDock->container.iHeight - fDockOffsetY;
		fDockOffsetX = pDock->container.iWidth - fDockOffsetX;
		
		glTranslatef (fDockOffsetY, fDockOffsetX, 0.);
		glRotatef (-90., 0., 0., 1.);
		if (pDock->container.bDirectionUp)
			glScalef (1., -1., 1.);
	}
	
	glBegin(GL_QUADS);
	glVertex3f(0., 0., 0.);  // Bottom Left Of The Texture and Quad
	glVertex3f(fLittleWidth, 0., 0.);  // Bottom Right Of The Texture and Quad
	glVertex3f(fLittleWidth + fDeltaXRight, - fHeight, 0.);  // Top Right Of The Texture and Quad
	glVertex3f(fLittleWidth + fDeltaXRight - fBigWidth, - fHeight, 0.);  // Top Left Of The Texture and Quad
	glEnd();
	
	if (myBackground.iDockLineWidth != 0)
	{
		glPolygonMode (GL_FRONT, GL_LINE);
		glEnable (GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glLineWidth (myBackground.iDockLineWidth);
		glColor4f (myBackground.fLineColor[0], myBackground.fLineColor[1], myBackground.fLineColor[2], myBackground.fLineColor[3]);
		
		glBegin(GL_LINES);
		glVertex3f(fLittleWidth, 0., 0.);
		glVertex3f(fLittleWidth + fDeltaXRight, - fHeight, 0.);
		glEnd();
		
		glBegin(GL_LINES);
		glVertex3f(0., 0., 0.);
		glVertex3f(fLittleWidth + fDeltaXRight - fBigWidth, - fHeight, 0.);
		glEnd();
		
		glDisable(GL_LINE_SMOOTH);
	}
	
	glDisable (GL_BLEND);
}
