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
#include "applet-decorator-tooltip.h"

#define _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH 20
#define _CAIRO_DIALOG_TOOLTIP_MARGIN 4
#define CD_ARROW_HEIGHT 6
#define CD_ALIGN 0.5
#define CD_RADIUS (myDialogsParam.bUseDefaultColors ? myStyleParam.iCornerRadius : myDialogsParam.iCornerRadius)

/*
ic______^___  arrow height + margin
ic     msg
 |
 |   widget
 |
 |____________  bottom margin

*/

void cd_decorator_set_frame_size_tooltip (CairoDialog *pDialog)
{
	int iMargin = .5 * myDialogsParam.iLineWidth + (1. - sqrt (2) / 2) * CD_RADIUS;
	int iIconOffset = pDialog->iIconSize / 2;
	pDialog->iRightMargin = iMargin + _CAIRO_DIALOG_TOOLTIP_MARGIN;
	pDialog->iLeftMargin = iIconOffset + iMargin + _CAIRO_DIALOG_TOOLTIP_MARGIN;
	pDialog->iTopMargin = iIconOffset + _CAIRO_DIALOG_TOOLTIP_MARGIN + myDialogsParam.iLineWidth;
	pDialog->iBottomMargin = _CAIRO_DIALOG_TOOLTIP_MARGIN;
	pDialog->iMinBottomGap = CD_ARROW_HEIGHT;
	pDialog->iMinFrameWidth = _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH;
	pDialog->fAlign = .5;
	pDialog->container.fRatio = 0.;
	pDialog->container.bUseReflect = FALSE;
	pDialog->iIconOffsetX = iIconOffset;
	pDialog->iIconOffsetY = pDialog->iTopMargin;
}


void cd_decorator_draw_decorations_tooltip (cairo_t *pCairoContext, CairoDialog *pDialog)
{
	double fLineWidth = myDialogsParam.iLineWidth;
	double fRadius = CD_RADIUS;
	double fIconOffset = pDialog->iIconSize / 2;  // myDialogsParam.iDialogIconSize/2
	
	double fOffsetX = fRadius + fLineWidth / 2 + fIconOffset;
	double fOffsetY = (pDialog->container.bDirectionUp ? fLineWidth / 2 : pDialog->container.iHeight - fLineWidth / 2) + (pDialog->container.bDirectionUp ? fIconOffset : /**-fIconOffset*/ -_CAIRO_DIALOG_TOOLTIP_MARGIN);  // _CAIRO_DIALOG_TOOLTIP_MARGIN is to compensate for the slightly different placement of top dialogs
	int sens = (pDialog->container.bDirectionUp ? 1 : -1);
	int iWidth = pDialog->container.iWidth - fIconOffset;
	
	int h = pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (2 * fRadius + fLineWidth);
	if (pDialog->container.bDirectionUp)
		h -= fIconOffset;
	else
		h -= _CAIRO_DIALOG_TOOLTIP_MARGIN;
	
	//On se déplace la ou il le faut
	cairo_move_to (pCairoContext, fOffsetX, fOffsetY);
	
	// Ligne du haut (Haut gauche -> Haut Droite)
	cairo_rel_line_to (pCairoContext, iWidth - (2 * fRadius + fLineWidth), 0);
	
	// Coin haut droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius, 0,
		fRadius, sens * fRadius);
	
	// Ligne droite. (Haut droit -> Bas droit)
	cairo_rel_line_to (pCairoContext, 0, sens * h);
	
	// Coin bas droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, sens * fRadius,
		-fRadius, sens * fRadius);
	
	// La pointe.
	int iDeltaIconX = pDialog->container.iWindowPositionX + pDialog->container.iWidth - fRadius - fLineWidth/2 - pDialog->iAimedX;
	cairo_rel_line_to (pCairoContext, - iDeltaIconX + _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH/2, 0);
	cairo_rel_line_to (pCairoContext, - _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH/2, sens * CD_ARROW_HEIGHT);
	cairo_rel_line_to (pCairoContext, - _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH/2, -sens * CD_ARROW_HEIGHT);
	cairo_rel_line_to (pCairoContext, - iWidth + 2*fRadius + fLineWidth + iDeltaIconX + _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH/2, 0);
	
	// Coin bas gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		-fRadius, 0,
		-fRadius, -sens * fRadius);
	
	// On remonte.
	cairo_rel_line_to (pCairoContext, 0, - sens * h);
	
	// Coin haut gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -sens * fRadius,
		fRadius, -sens * fRadius);
	if (fRadius < 1)
		cairo_close_path (pCairoContext);
	
	// draw background
	if (myDialogsParam.bUseDefaultColors)
		gldi_style_colors_set_bg_color (pCairoContext);
	else
		cairo_set_source_rgba (pCairoContext, myDialogsParam.fBgColor[0], myDialogsParam.fBgColor[1], myDialogsParam.fBgColor[2], myDialogsParam.fBgColor[3]);
	///cairo_fill_preserve (pCairoContext);
	cairo_save (pCairoContext);
	cairo_clip_preserve (pCairoContext);
	gldi_style_colors_paint_bg_color (pCairoContext, pDialog->container.iWidth);
	cairo_restore (pCairoContext);
	
	// draw outline
	if (myDialogsParam.bUseDefaultColors)
		gldi_style_colors_set_line_color (pCairoContext);
	else
		cairo_set_source_rgba (pCairoContext, myDialogsParam.fLineColor[0], myDialogsParam.fLineColor[1], myDialogsParam.fLineColor[2], myDialogsParam.fLineColor[3]);
	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_stroke (pCairoContext);
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
	double fRadius = CD_RADIUS, fLineWidth = myDialogsParam.iLineWidth;
	
	// draw the outline and set the clip
	GtkAllocation alloc;
	gtk_widget_get_allocation (pMenu, &alloc);
	
	int w = alloc.width, h = alloc.height;
	int x, y;
	gdk_window_get_position (gtk_widget_get_window (gtk_widget_get_toplevel(pMenu)), &x, &y);
	int _ah = ah - fLineWidth;  // we want the tip of the arrow to reach the border, not the middle of the stroke
	int aw = _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH/2;
	int _aw = aw;
	double dx, dy;
	
	double fDockOffsetX = fRadius + fLineWidth/2;
	double fDockOffsetY = fLineWidth/2;
	double fFrameWidth, fFrameHeight;
	fFrameWidth = w - 2*fRadius - fLineWidth;
	fFrameHeight = h - fLineWidth;
	switch (iMarginPosition)
	{
		case 0:  // bottom
			fFrameHeight -= ah;
		break;
		case 1:  // top
			fFrameHeight -= ah;
			fDockOffsetY += ah;
		break;
		case 2:  // right
			fFrameWidth -= ah;
		break;
		case 3:  // left
			fFrameWidth -= ah;
			fDockOffsetX += ah;
		break;
		default:
		break;
	}
	
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	
	if (iMarginPosition == 1)  // top arrow
	{
		dx = MIN (w - fRadius - 2*aw, MAX (fRadius, iAimedX - x - aw));
		cairo_line_to (pCairoContext, dx, fDockOffsetY);
		cairo_line_to (pCairoContext, MIN (w, MAX (0, iAimedX - x)), fDockOffsetY - _ah);
		cairo_line_to (pCairoContext, dx + 2*aw, fDockOffsetY);
		cairo_line_to (pCairoContext, fFrameWidth, fDockOffsetY);
	}
	else
		cairo_rel_line_to (pCairoContext, fFrameWidth, 0);
	
	//\_________________ Coin haut droit.
	cairo_arc (pCairoContext,
		fDockOffsetX + fFrameWidth, fDockOffsetY + fRadius,
		fRadius,
		-G_PI/2, 0.);
	if (iMarginPosition == 2)  // right arrow
	{
		if (h < 2*aw + 2*fRadius)
			_aw = (h - 2*fRadius) / 2;
		dy = MIN (h - fRadius - 2*aw, MAX (fRadius, iAimedY - y - _aw));
		cairo_line_to (pCairoContext, w - ah, dy);
		cairo_line_to (pCairoContext, w - ah + _ah, MAX (0, iAimedY - y));
		cairo_line_to (pCairoContext, w - ah, dy + 2*_aw);
		cairo_line_to (pCairoContext, w - ah, h - fRadius);
	}
	else
		cairo_rel_line_to (pCairoContext, 0, (fFrameHeight - fRadius * 2));
	
	//\_________________ Coin bas droit.
	cairo_arc (pCairoContext,
		fDockOffsetX + fFrameWidth, fDockOffsetY + fFrameHeight - fRadius,
		fRadius,
		0., G_PI/2);
	
	if (iMarginPosition == 0)  // bottom arrow
	{
		dx = MIN (w - fRadius - 2*aw, MAX (fRadius, iAimedX - x - aw));
		cairo_line_to (pCairoContext, dx + 2*aw, fDockOffsetY + fFrameHeight);
		cairo_line_to (pCairoContext, MIN (w, MAX (0, iAimedX - x)), fDockOffsetY + fFrameHeight + _ah);
		cairo_line_to (pCairoContext, dx, fDockOffsetY + fFrameHeight);
		cairo_line_to (pCairoContext, fDockOffsetX, fDockOffsetY + fFrameHeight);
	}
	else
		cairo_rel_line_to (pCairoContext, - fFrameWidth, 0);
	
	//\_________________ Coin bas gauche.
	cairo_arc (pCairoContext,
		fDockOffsetX, fDockOffsetY + fFrameHeight - fRadius,
		fRadius,
		G_PI/2, G_PI);
	
	if (iMarginPosition == 3)  // left arrow
	{
		if (h < 2*aw + 2*fRadius)
			_aw = (h - 2*fRadius) / 2;
		dy = MIN (h - fRadius - 2*aw, MAX (fRadius, iAimedY - y - _aw));
		cairo_line_to (pCairoContext, ah, dy);
		cairo_line_to (pCairoContext, ah - _ah, MAX (0, iAimedY - y));
		cairo_line_to (pCairoContext, ah, dy + 2*_aw);
		cairo_line_to (pCairoContext, ah, fRadius);
	}
	else
		cairo_rel_line_to (pCairoContext, 0, - fFrameHeight + fRadius * 2);
	//\_________________ Coin haut gauche.
	cairo_arc (pCairoContext,
		fDockOffsetX, fDockOffsetY + fRadius,
		fRadius,
		G_PI, -G_PI/2);
	
	// draw the background
	if (myDialogsParam.bUseDefaultColors)
		gldi_style_colors_set_bg_color (pCairoContext);
	else
		cairo_set_source_rgba (pCairoContext, myDialogsParam.fBgColor[0], myDialogsParam.fBgColor[1], myDialogsParam.fBgColor[2], myDialogsParam.fBgColor[3]);
	cairo_save (pCairoContext);
	cairo_clip_preserve (pCairoContext);
	gldi_style_colors_paint_bg_color (pCairoContext, alloc.width);
	cairo_restore (pCairoContext);
	
	// draw outline
	if (fLineWidth != 0)  // draw the outline with same color as bg, but opaque
	{
		if (myDialogsParam.bUseDefaultColors)
			gldi_style_colors_set_line_color (pCairoContext);
		else
			cairo_set_source_rgba (pCairoContext, myDialogsParam.fLineColor[0], myDialogsParam.fLineColor[1], myDialogsParam.fLineColor[2], myDialogsParam.fLineColor[3]);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_stroke_preserve (pCairoContext);
	}
	
	cairo_clip (pCairoContext);  // clip
}

static void _setup_menu (GtkWidget *pMenu)
{
	GldiMenuParams *pParams = g_object_get_data (G_OBJECT(pMenu), "gldi-params");
	pParams->iRadius = CD_RADIUS;
	pParams->fAlign = CD_ALIGN;
	pParams->iArrowHeight = CD_ARROW_HEIGHT;
}

void cd_decorator_register_tooltip (void)
{
	CairoDialogDecorator *pDecorator = g_new (CairoDialogDecorator, 1);
	pDecorator->set_size = cd_decorator_set_frame_size_tooltip;
	pDecorator->render = cd_decorator_draw_decorations_tooltip;
	pDecorator->render_opengl = NULL;
	pDecorator->setup_menu = _setup_menu;
	pDecorator->render_menu = _render_menu;
	pDecorator->cDisplayedName = D_ (MY_APPLET_DECORATOR_TOOLTIP_NAME);
	cairo_dock_register_dialog_decorator (MY_APPLET_DECORATOR_TOOLTIP_NAME, pDecorator);
}
