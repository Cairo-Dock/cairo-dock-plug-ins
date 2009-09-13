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
#include "applet-decorator-modern.h"


void cd_decorator_set_frame_size_modern (CairoDialog *pDialog)
{
	int iMargin = .5 * myConfig.iModernLineWidth + .5 * myConfig.iModernRadius;
	pDialog->iRightMargin = iMargin;
	pDialog->iLeftMargin = iMargin;
	pDialog->iTopMargin = iMargin;
	pDialog->iBottomMargin = iMargin;
	pDialog->iMinBottomGap = 30;
	pDialog->iMinFrameWidth = 30;  // valeur au pif.
	pDialog->fAlign = .33;  // la pointe est a 33% du bord du dialogue.
	pDialog->container.fRatio = 0.;  // pas de reflet merci.
	pDialog->container.bUseReflect = FALSE;
}


void cd_decorator_draw_decorations_modern (cairo_t *pCairoContext, CairoDialog *pDialog)
{
	double fLineWidth = myConfig.iModernLineWidth;
	double fRadius = MIN (myConfig.iModernRadius, pDialog->iBubbleHeight/2);
	int sens = (pDialog->container.bDirectionUp ?	1 :	-1);
	int sens2 = (pDialog->bRight ? 1 :	-1);
	
	//\_________________ On part du haut.
	double fOffsetX = (pDialog->bRight ? fLineWidth/2 : pDialog->container.iWidth - fLineWidth/2);
	double fOffsetY = (pDialog->container.bDirectionUp ? 0. : pDialog->container.iHeight);
	cairo_move_to (pCairoContext, fOffsetX, fOffsetY);
	
	//\_________________ On remplit le fond.
	cairo_rel_line_to (pCairoContext,
		0.,
		sens * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - fRadius));
	cairo_rel_line_to (pCairoContext,
		sens2 * fRadius,
		sens * fRadius);
	cairo_rel_line_to (pCairoContext,
		sens2 * pDialog->iBubbleWidth,
		0.);
	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_set_source_rgba (pCairoContext, myConfig.fModernLineColor[0], myConfig.fModernLineColor[1], myConfig.fModernLineColor[2], myConfig.fModernLineColor[3]);
	
	cairo_rel_line_to (pCairoContext,
		0.,
		- sens * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - fRadius));
	cairo_rel_line_to (pCairoContext,
		- sens2 * fRadius,
		- sens * fRadius);
	cairo_close_path (pCairoContext);
	cairo_set_source_rgba (pCairoContext, myDialogs.fDialogColor[0], myDialogs.fDialogColor[1],	myDialogs.fDialogColor[2], myDialogs.fDialogColor[3]);
	cairo_fill (pCairoContext);
	
	//\_________________ On trace le cadre.
	cairo_move_to (pCairoContext, fOffsetX, fOffsetY);
	cairo_rel_line_to (pCairoContext,
		0.,
		sens * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - fRadius));
	cairo_rel_line_to (pCairoContext,
		sens2 * fRadius,
		sens * fRadius);
	cairo_rel_line_to (pCairoContext,
		sens2 * pDialog->iBubbleWidth,
		0.);
	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_set_source_rgba (pCairoContext, myConfig.fModernLineColor[0], myConfig.fModernLineColor[1], myConfig.fModernLineColor[2], myConfig.fModernLineColor[3]);
	cairo_stroke (pCairoContext);
	
	//\_________________ On part du haut, petit cote.
	fOffsetX = (pDialog->bRight ? fRadius + fLineWidth/2 : pDialog->container.iWidth - fRadius - fLineWidth/2);
	fOffsetY = (pDialog->container.bDirectionUp ? pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin : pDialog->container.iHeight - (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin));
	
	//\_________________ On trace la pointe.
	cairo_set_line_width (pCairoContext, 1.);
	cairo_set_source_rgba (pCairoContext, myConfig.fModernLineColor[0], myConfig.fModernLineColor[1], myConfig.fModernLineColor[2], myConfig.fModernLineColor[3]);
	int i, h = pDialog->container.iHeight - (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin);
	double w1 = MAX (0, pDialog->iAimedX - pDialog->container.iWindowPositionX - (pDialog->bRight ? fOffsetX : 0));
	double w2 = MAX (0, pDialog->container.iWindowPositionX + pDialog->container.iWidth - pDialog->iAimedX - (pDialog->bRight ? 0 : fRadius + fLineWidth/2));
	//g_print ("%.2f,%.2f ; %d + %d > %d\n", w1, w2, pDialog->container.iWindowPositionX, pDialog->container.iWidth, pDialog->iAimedX);
	double x, y, w;
	for (i = 0; i < h; i += 4)
	{
		y = fOffsetY + sens * i;
		x = fOffsetX + 1. * sens2 * i / h * (pDialog->bRight ? w1 : w2);
		w = (w1 + w2) * (h - i) / h;
		cairo_move_to (pCairoContext, x, y);
		cairo_rel_line_to (pCairoContext,
			sens2 * w,
			0.);
		cairo_stroke (pCairoContext);
	}
	if (h-i > 1)
	{
		y = fOffsetY + sens * h;
		x = fOffsetX + 1. * sens2 * (pDialog->bRight ? w1 : w2);
		w = MIN (w/2, 15);
		cairo_move_to (pCairoContext, x, y);
		cairo_rel_line_to (pCairoContext,
			sens2 * w,
			0.);
		cairo_stroke (pCairoContext);
	}
}


void cd_decorator_register_modern (void)
{
	CairoDialogDecorator *pDecorator = g_new (CairoDialogDecorator, 1);
	pDecorator->set_size = cd_decorator_set_frame_size_modern;
	pDecorator->render = cd_decorator_draw_decorations_modern;
	pDecorator->render_opengl = NULL;
	pDecorator->cDisplayedName = D_ (MY_APPLET_DECORATOR_MODERN_NAME);
	cairo_dock_register_dialog_decorator (MY_APPLET_DECORATOR_MODERN_NAME, pDecorator);
}
