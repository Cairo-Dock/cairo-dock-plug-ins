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

#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-decorator-3Dplane.h"

#define DIALOG_REFLECT_SIZE 50
#define DIALOG_INCLINATION 60.


void cd_decorator_set_frame_size_3Dplane (CairoDialog *pDialog)
{
	double fInclination = tan (DIALOG_INCLINATION/180.*G_PI);
	double fFrameHeight = 10 + MIN (MAX (pDialog->iIconSize, pDialog->iTextHeight), DIALOG_REFLECT_SIZE);
	double fRadius = MIN (myConfig.iPlaneRadius, fFrameHeight/2);
	double fLineWidth = myConfig.iPlaneLineWidth;
	
	int iMargin = cairo_dock_calculate_extra_width_for_trapeze (fFrameHeight, fInclination, fRadius, fLineWidth)/2;
	pDialog->iRightMargin = iMargin;
	pDialog->iLeftMargin = iMargin;
	pDialog->iTopMargin = 0;
	pDialog->iBottomMargin = fFrameHeight - 10;
	pDialog->iMinBottomGap = 10;
	pDialog->iMinFrameWidth = 30;  // valeur au pif.
	pDialog->fAlign = .5;
	pDialog->container.fRatio = .4;
	pDialog->container.bUseReflect = TRUE;
}


void cd_decorator_draw_decorations_3Dplane (cairo_t *pCairoContext, CairoDialog *pDialog)
{
	double fInclination = tan (DIALOG_INCLINATION/180.*G_PI);
	double fReflectHeight = MIN (MAX (pDialog->iIconSize, pDialog->iTextHeight), DIALOG_REFLECT_SIZE);
	double fFrameHeight = 10 + fReflectHeight;
	double fRadius = MIN (myConfig.iPlaneRadius, fFrameHeight/2);
	double fLineWidth = myConfig.iPlaneLineWidth;
	int sens = (pDialog->container.bDirectionUp ?	1 :	-1);
	double fFrameWidth = pDialog->iBubbleWidth;
	double fOffsetX = pDialog->iLeftMargin;
	double fOffsetY = pDialog->iTopMargin + pDialog->iBubbleHeight - 10;
	
	cairo_dock_draw_frame (pCairoContext, fRadius, fLineWidth, fFrameWidth, fFrameHeight, fOffsetX, fOffsetY, sens, fInclination, pDialog->container.bIsHorizontal, FALSE);
	
	cairo_set_source_rgba (pCairoContext, myConfig.fPlaneColor[0], myConfig.fPlaneColor[1], myConfig.fPlaneColor[2], myConfig.fPlaneColor[3]);
	cairo_fill_preserve (pCairoContext);

	cairo_set_line_width (pCairoContext, 1.);
	cairo_set_source_rgba (pCairoContext, myConfig.fPlaneLineColor[0], myConfig.fPlaneLineColor[1], myConfig.fPlaneLineColor[2], myConfig.fPlaneLineColor[3]);
	cairo_stroke (pCairoContext);
	
	cairo_rectangle (pCairoContext,
		pDialog->iLeftMargin,
		pDialog->iTopMargin,
		pDialog->iBubbleWidth,
		pDialog->iBubbleHeight);
	cairo_set_source_rgba (pCairoContext, myDialogs.fDialogColor[0], myDialogs.fDialogColor[1], myDialogs.fDialogColor[2], myDialogs.fDialogColor[3]);
	cairo_fill (pCairoContext);
}


void cd_decorator_register_3Dplane (void)
{
	CairoDialogDecorator *pDecorator = g_new (CairoDialogDecorator, 1);
	pDecorator->set_size = cd_decorator_set_frame_size_3Dplane;
	pDecorator->render = cd_decorator_draw_decorations_3Dplane;
	pDecorator->render_opengl = NULL;
	pDecorator->cDisplayedName = D_ (MY_APPLET_DECORATOR_3DPLANE_NAME);
	cairo_dock_register_dialog_decorator (MY_APPLET_DECORATOR_3DPLANE_NAME, pDecorator);
}
