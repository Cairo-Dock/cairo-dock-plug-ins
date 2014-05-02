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

#define CD_ARROW_HEIGHT 20
#define CD_ALIGN 0.33
#define CD_RADIUS (myDialogsParam.bUseDefaultColors ? myStyleParam.iCornerRadius : myDialogsParam.iCornerRadius) * 1.5

void cd_decorator_set_frame_size_modern (CairoDialog *pDialog)
{
	int iMargin = .5 * myDialogsParam.iLineWidth + .5 * CD_RADIUS;
	pDialog->iRightMargin = iMargin;
	pDialog->iLeftMargin = iMargin;
	pDialog->iTopMargin = iMargin;
	pDialog->iBottomMargin = iMargin;
	pDialog->iMinBottomGap = CD_ARROW_HEIGHT;
	pDialog->iMinFrameWidth = CD_RADIUS;
	pDialog->fAlign = CD_ALIGN;  // la pointe est a 33% du bord du dialogue.
	pDialog->container.fRatio = 0.;  // pas de reflet merci.
	pDialog->container.bUseReflect = FALSE;
}


void cd_decorator_draw_decorations_modern (cairo_t *pCairoContext, CairoDialog *pDialog)
{
	double fLineWidth = myDialogsParam.iLineWidth;
	double fRadius = MIN (CD_RADIUS, pDialog->iBubbleHeight/2);
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
	
	cairo_rel_line_to (pCairoContext,
		0.,
		- sens * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - fRadius));
	cairo_rel_line_to (pCairoContext,
		- sens2 * fRadius,
		- sens * fRadius);
	cairo_close_path (pCairoContext);
	gldi_style_colors_set_bg_color (pCairoContext);
	///cairo_fill (pCairoContext);
	cairo_save (pCairoContext);
	cairo_clip (pCairoContext);
	///gldi_style_colors_paint_bg_color (pCairoContext, pDialog->container.iWidth);
	cairo_paint (pCairoContext);
	cairo_restore (pCairoContext);
	
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
	if (myDialogsParam.bUseDefaultColors)
		gldi_style_colors_set_line_color (pCairoContext);
	else
		gldi_color_set_cairo (pCairoContext, &myDialogsParam.fLineColor);
	cairo_stroke (pCairoContext);
	
	//\_________________ On part du haut, petit cote.
	fOffsetX = (pDialog->bRight ? fRadius + fLineWidth/2 : pDialog->container.iWidth - fRadius - fLineWidth/2);
	fOffsetY = (pDialog->container.bDirectionUp ? pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin : pDialog->container.iHeight - (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin));
	
	//\_________________ On trace la pointe.
	cairo_set_line_width (pCairoContext, 1.);
	int i, h = pDialog->container.iHeight - (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin);
	double w1 = MAX (0, pDialog->iAimedX - pDialog->container.iWindowPositionX - (pDialog->bRight ? fOffsetX : 0));
	double w2 = MAX (0, pDialog->container.iWindowPositionX + pDialog->container.iWidth - pDialog->iAimedX - (pDialog->bRight ? 0 : fRadius + fLineWidth/2));
	//g_print ("%.2f,%.2f ; %d + %d > %d\n", w1, w2, pDialog->container.iWindowPositionX, pDialog->container.iWidth, pDialog->iAimedX);
	double x, y, w;
	for (i = 0; i < h; i += 3)
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


static void _render_submenu (GtkWidget *pMenu, cairo_t *pCairoContext)
{
	GtkAllocation alloc;
	gtk_widget_get_allocation (pMenu, &alloc);
	
	int w = alloc.width, h = alloc.height;
	double fRadius = CD_RADIUS, fLineWidth = myDialogsParam.iLineWidth;
	double fFrameWidth, fFrameHeight;
	fFrameWidth = w - 2*fRadius - fLineWidth;
	fFrameHeight = h - 2*fRadius - fLineWidth;
	
	//\_________________ build path
	cairo_move_to (pCairoContext, fRadius + fLineWidth/2, fLineWidth/2);
	cairo_rel_line_to (pCairoContext,
		fFrameWidth,
		0.);
	cairo_rel_line_to (pCairoContext,
		fRadius,
		fRadius);
	cairo_rel_line_to (pCairoContext,
		0.,
		fFrameHeight);
	cairo_rel_line_to (pCairoContext,
		- fRadius,
		fRadius);
	cairo_rel_line_to (pCairoContext,
		- fFrameWidth,
		- 0.);
	cairo_rel_line_to (pCairoContext,
		- fRadius,
		- fRadius);
	cairo_rel_line_to (pCairoContext,
		0.,
		- fFrameHeight);
	cairo_close_path (pCairoContext);
	
	// draw outline
	if (fLineWidth != 0)  // draw the outline with same color as bg, but opaque
	{
		if (myDialogsParam.bUseDefaultColors)
			gldi_style_colors_set_line_color (pCairoContext);
		else
			gldi_color_set_cairo (pCairoContext, &myDialogsParam.fLineColor);
		cairo_stroke_preserve (pCairoContext);
	}
	
	cairo_clip (pCairoContext);  // clip
	
	//\_________________ draw background
	if (myDialogsParam.bUseDefaultColors)
		gldi_style_colors_set_bg_color_full (pCairoContext, FALSE);
	else
		gldi_color_set_cairo_rgb (pCairoContext, &myDialogsParam.fBgColor);
	
	gldi_style_colors_paint_bg_color_with_alpha (pCairoContext, alloc.width, myDialogsParam.bUseDefaultColors ? -1. : myDialogsParam.fBgColor.rgba.alpha);
}

static void _render_menu (GtkWidget *pMenu, cairo_t *pCairoContext)
{
	GldiMenuParams *pParams = g_object_get_data (G_OBJECT(pMenu), "gldi-params");
	int iMarginPosition = -1;
	int iAimedX = 0, iAimedY = 0;
	int ah = CD_ARROW_HEIGHT;
	if (pParams && pParams->pIcon)  // main menu
	{
		iMarginPosition = pParams->iMarginPosition;
		iAimedX = pParams->iAimedX;
		iAimedY = pParams->iAimedY;
	}
	else
	{
		_render_submenu (pMenu, pCairoContext);
		return;
	}
	
	GtkAllocation alloc;
	gtk_widget_get_allocation (pMenu, &alloc);
	
	int w = alloc.width, h = alloc.height;
	int x, y;
	gdk_window_get_position (gtk_widget_get_window (gtk_widget_get_toplevel(pMenu)), &x, &y);
	
	double fRadius = CD_RADIUS, fLineWidth = myDialogsParam.iLineWidth;
	double fDockOffsetX = fLineWidth/2;
	double fDockOffsetY = fLineWidth/2;
	double fFrameWidth, fFrameHeight;
	int sw, sh;  // w/h direction
	fFrameWidth = w - fLineWidth;
	fFrameHeight = h - fLineWidth;
	
	switch (iMarginPosition)
	{
		case 0:  // bottom
		case 1:  // top
			fFrameHeight -= ah;
		break;
		case 2:  // right
		case 3:  // left
			fFrameWidth -= ah;
		break;
		default:
		break;
	}
	if (fFrameHeight < 2*fRadius)
		fRadius = fFrameHeight/2;
	if (fFrameWidth < 2*fRadius)
		fRadius = fFrameWidth/2;
	fFrameWidth -= fRadius;
	fFrameHeight -= fRadius;
	
	
	// set bg color/pattern
	gldi_style_colors_set_line_color (pCairoContext);
	
	//\_________________ draw outline
	if (fLineWidth != 0)
	{
		cairo_set_line_width (pCairoContext, fLineWidth);
		switch (iMarginPosition)
		{
			case 0:  // bottom
				fDockOffsetX = fLineWidth/2;
				fDockOffsetY = 0;
				sw = 1;
				sh = 1;
			break;
			case 1:  // top
				fDockOffsetX = fLineWidth/2;
				fDockOffsetY = h;
				sw = 1;
				sh = -1;
			break;
			case 2:  // right
				fDockOffsetX = w - ah;
				fDockOffsetY = h;
				sw = -1;
				sh = -1;
			break;
			case 3:  // left
				fDockOffsetX = ah;
				fDockOffsetY = h;
				sw = 1;
				sh = -1;
			break;
			default:
			break;
		}
		
		cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
		cairo_rel_line_to (pCairoContext,
			0.,
			sh * fFrameHeight);
		cairo_rel_line_to (pCairoContext,
			sw * fRadius,
			sh * fRadius);
		cairo_rel_line_to (pCairoContext,
			sw * fFrameWidth,
			0.);
		
		cairo_stroke (pCairoContext);
	}
	
	//\_________________ draw arrow
	switch (iMarginPosition)
	{
		case 0:  // bottom
			fDockOffsetX = fRadius;
			fDockOffsetY = h - ah;
			sh = 1;
		break;
		case 1:  // top
			fDockOffsetX = fRadius;
			fDockOffsetY = ah;
			sh = -1;
		break;
		case 2:  // right
			fDockOffsetX = w - ah;
			fDockOffsetY = fRadius;
			sh = 1;
		break;
		case 3:  // left
			fDockOffsetX = ah;
			fDockOffsetY = fRadius;
			sh = -1;
		break;
		default:
		break;
	}
	
	cairo_set_line_width (pCairoContext, 1.);
	
	double w1, w2;
	int i;
	double ax, ay, aw;
	switch (iMarginPosition)
	{
		case 0:  // bottom
		case 1:  // top
			w1 = MAX (0, iAimedX - x - fRadius);
			w2 = MAX (0, x + w - iAimedX);
			for (i = 0; i < ah; i += 3)
			{
				ay = fDockOffsetY + sh * i;
				ax = fDockOffsetX + (double)i / ah * w1;
				aw = (w1 + w2) * (ah - i) / ah;
				cairo_move_to (pCairoContext, ax, ay);
				cairo_rel_line_to (pCairoContext,
					aw,
					0.);
				cairo_stroke (pCairoContext);
			}
			if (ah-i > 1)
			{
				ay = fDockOffsetY + sh * ah;
				ax = fDockOffsetX + w1;
				aw = MIN (aw/2, 15);
				cairo_move_to (pCairoContext, ax, ay);
				cairo_rel_line_to (pCairoContext,
					aw,
					0.);
				cairo_stroke (pCairoContext);
			}
		break;
		case 2:  // right
		case 3:  // left
			w1 = MAX (0, iAimedY - y - fRadius);
			w2 = MAX (0, y + h - iAimedY);
			cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
			for (i = 0; i < ah; i += 3)
			{
				ax = fDockOffsetX + sh * i;
				ay = fDockOffsetY + (double)i / ah * w1;
				aw = (w1 + w2) * (ah - i) / ah;
				cairo_move_to (pCairoContext, ax, ay);
				cairo_rel_line_to (pCairoContext,
					0.,
					aw);
				cairo_stroke (pCairoContext);
			}
			if (ah-i > 1)
			{
				ax = fDockOffsetX + sh * ah;
				ay = fDockOffsetY + w1;
				aw = MIN (aw/2, 15);
				cairo_move_to (pCairoContext, ax, ay);
				cairo_rel_line_to (pCairoContext,
					0.,
					aw);
				cairo_stroke (pCairoContext);
			}
		break;
		default:
		break;
	}
	
	//\_________________ draw background
	switch (iMarginPosition)
	{
		case 0:  // bottom
			fDockOffsetX = fLineWidth;
			fDockOffsetY = 0;
			sw = 1;
		break;
		case 1:  // top
			fDockOffsetX = w - fLineWidth;
			fDockOffsetY = ah + fLineWidth/2;
			sw = -1;
		break;
		case 2:  // right
			fDockOffsetX = 0;
			fDockOffsetY = fLineWidth/2;
			sw = 1;
		break;
		case 3:  // left
			fDockOffsetX = w;
			fDockOffsetY = fLineWidth/2;
			sw = -1;
		break;
		default:
		break;
	}
	cairo_save (pCairoContext);
	
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	cairo_rel_line_to (pCairoContext,
		0.,
		fFrameHeight);
	cairo_rel_line_to (pCairoContext,
		sw * fRadius,
		fRadius);
	cairo_rel_line_to (pCairoContext,
		sw * fFrameWidth,
		0.);
	cairo_rel_line_to (pCairoContext,
		0.,
		- fFrameHeight);
	cairo_rel_line_to (pCairoContext,
		- sw * fRadius,
		- fRadius);
	cairo_close_path (pCairoContext);
	
	cairo_clip (pCairoContext);  // clip
	
	if (myDialogsParam.bUseDefaultColors)
		gldi_style_colors_set_bg_color_full (pCairoContext, FALSE);
	else
		gldi_color_set_cairo_rgb (pCairoContext, &myDialogsParam.fBgColor);
	
	gldi_style_colors_paint_bg_color_with_alpha (pCairoContext, alloc.width, myDialogsParam.bUseDefaultColors ? -1. : myDialogsParam.fBgColor.rgba.alpha);
}

static void _setup_menu (GtkWidget *pMenu)
{
	GldiMenuParams *pParams = g_object_get_data (G_OBJECT(pMenu), "gldi-params");
	pParams->iRadius = CD_RADIUS;
	pParams->fAlign = CD_ALIGN;
	pParams->iArrowHeight = CD_ARROW_HEIGHT;
}

void cd_decorator_register_modern (void)
{
	CairoDialogDecorator *pDecorator = g_new (CairoDialogDecorator, 1);
	pDecorator->set_size = cd_decorator_set_frame_size_modern;
	pDecorator->render = cd_decorator_draw_decorations_modern;
	pDecorator->render_opengl = NULL;
	pDecorator->setup_menu = _setup_menu;
	pDecorator->render_menu = _render_menu;
	pDecorator->cDisplayedName = D_ (MY_APPLET_DECORATOR_MODERN_NAME);
	cairo_dock_register_dialog_decorator (MY_APPLET_DECORATOR_MODERN_NAME, pDecorator);
}
