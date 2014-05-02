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
#include "applet-decorator-curly.h"

#define CD_ARROW_HEIGHT 12
#define CD_ALIGN 0.5
#define CD_RADIUS (myDialogsParam.bUseDefaultColors ? myStyleParam.iCornerRadius : myDialogsParam.iCornerRadius)

void cd_decorator_set_frame_size_curly (CairoDialog *pDialog)
{
	int iMargin = .5 * myDialogsParam.iLineWidth + (1. - sqrt (2) / 2) * CD_RADIUS;
	pDialog->iRightMargin = iMargin;
	pDialog->iLeftMargin = iMargin;
	pDialog->iTopMargin = iMargin;
	pDialog->iBottomMargin = iMargin;
	pDialog->iMinBottomGap = CD_ARROW_HEIGHT + CD_RADIUS;
	pDialog->iMinFrameWidth = 10;  // au pif.
	pDialog->fAlign = .5;
	pDialog->container.fRatio = 0.;
	pDialog->container.bUseReflect = FALSE;
}


void cd_decorator_draw_decorations_curly (cairo_t *pCairoContext, CairoDialog *pDialog)
{
	double fLineWidth = myDialogsParam.iLineWidth;
	double fRadius = CD_RADIUS;
	// double fBottomRadius = 2 * fRadius;
	double fTipHeight = pDialog->iMinBottomGap + fLineWidth/2;
	double dh = MIN (fTipHeight + fRadius, .3 * pDialog->container.iWidth);
	
	double fOffsetX = fRadius + fLineWidth / 2;
	double fOffsetY = (pDialog->container.bDirectionUp ? fLineWidth / 2 : pDialog->container.iHeight - fLineWidth / 2);
	int sens = (pDialog->container.bDirectionUp ? 1 : -1);
	double fDemiWidth = .5 * pDialog->container.iWidth - fRadius - fLineWidth/2;
	
	int iDeltaIconX = MIN (0, pDialog->iAimedX - (pDialog->container.iWindowPositionX + pDialog->container.iWidth/2));
	if (iDeltaIconX == 0)
		iDeltaIconX = MAX (0, pDialog->iAimedX - (pDialog->container.iWindowPositionX + pDialog->container.iWidth/2));
	if (fabs (iDeltaIconX) < 3)  // filter useless tiny delta (and rounding errors).
		iDeltaIconX = 0;
	
	double dh1, dh2;
	if (iDeltaIconX != 0)  // on va limiter la courbature du petit cote.
	{
		double dhmin = dh * MAX (1. - fabs (iDeltaIconX) / fDemiWidth, .5);
		if (iDeltaIconX > 0)  // pointe decale vers la droite.
		{
			dh1 = dhmin;
			dh2 = dh;
		}
		else
		{
			dh1 = dh;
			dh2 = dhmin;
		}
	}
	else
	{
		dh1 = dh2 = dh;
	}
	
	
	//On se déplace la ou il le faut
	cairo_move_to (pCairoContext, fOffsetX, fOffsetY);
	cairo_set_tolerance (pCairoContext, 0.33);
	
	// Ligne du haut (Haut gauche -> Haut Droite)
	cairo_rel_line_to (pCairoContext,
		2*fDemiWidth,
		0.);
	
	// Coin haut droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius, 0,
		fRadius, sens * fRadius);
	
	// Ligne droite. (Haut droit -> Bas droit)
	double fHeight = pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (fRadius + fLineWidth/2);
	cairo_rel_line_to (pCairoContext,
		0,
		sens * fHeight);
	
	fDemiWidth = .5 * pDialog->container.iWidth - fLineWidth/2;
	// Coin bas droit et pointe.
	cairo_rel_curve_to (pCairoContext,
		0, sens * dh1,
		- fDemiWidth + iDeltaIconX, sens * (fTipHeight - dh1),
		- fDemiWidth + iDeltaIconX, sens * fTipHeight);
	
	// Coin bas gauche et pointe.
	cairo_rel_curve_to (pCairoContext,
		0, - sens * dh2,
		- fDemiWidth - iDeltaIconX, - sens * (fTipHeight - dh2),
		- fDemiWidth - iDeltaIconX, - sens * fTipHeight);
	
	// On remonte par la gauche.
	cairo_rel_line_to (pCairoContext,
		0,
		- sens * fHeight);
	// Coin haut gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -sens * fRadius,
		fRadius, -sens * fRadius);
	if (fRadius < 1)
		cairo_close_path (pCairoContext);
	
	// On remplit le fond.
	if (myDialogsParam.bUseDefaultColors)
		gldi_style_colors_set_bg_color (pCairoContext);
	else
		gldi_color_set_cairo (pCairoContext, &myDialogsParam.fBgColor);
	///cairo_fill_preserve (pCairoContext);
	cairo_save (pCairoContext);
	cairo_clip_preserve (pCairoContext);
	///gldi_style_colors_paint_bg_color (pCairoContext, pDialog->container.iWidth);
	cairo_paint (pCairoContext);
	cairo_restore (pCairoContext);
	
	// On trace le contour.
	if (fLineWidth != 0)  // draw the outline with same color as bg, but opaque
	{
		if (myDialogsParam.bUseDefaultColors)
			gldi_style_colors_set_line_color (pCairoContext);
		else
			gldi_color_set_cairo (pCairoContext, &myDialogsParam.fLineColor);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_stroke (pCairoContext);
	}
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
	
	// draw the outline and set the clip
	GtkAllocation alloc;
	gtk_widget_get_allocation (pMenu, &alloc);
	
	int w = alloc.width, h = alloc.height;
	int x, y;
	gdk_window_get_position (gtk_widget_get_window (gtk_widget_get_toplevel(pMenu)), &x, &y);
	
	double fRadius = CD_RADIUS, fLineWidth = myDialogsParam.iLineWidth;
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
	
	double fTipHeight = ah + fRadius;  // we want the tip of the arrow to reach the border, not the middle of the stroke
	double dh = fTipHeight + fRadius;
	int iDeltaIconX = 0, iDeltaIconY = 0;
	double dh1, dh2;
	switch (iMarginPosition)
	{
		case 0:
		case 1:
			dh = MIN (dh, .3 * w);
			iDeltaIconX = iAimedX - (x + w/2);
			if (fabs (iDeltaIconX) < 3)  // filter useless tiny delta (and rounding errors).
				iDeltaIconX = 0;
			if (iDeltaIconX != 0)  // on va limiter la courbature du petit cote.
			{
				double dhmin = dh * MAX (1. - fabs (iDeltaIconX) / (w/2), .5);
				if (iDeltaIconX > 0)  // pointe decale vers la droite.
				{
					dh1 = dhmin;
					dh2 = dh;
				}
				else
				{
					dh1 = dh;
					dh2 = dhmin;
				}
			}
			else
			{
				dh1 = dh2 = dh;
			}
		break;
		case 2:
		case 3:
			dh = MIN (dh, .3 * h);
			iDeltaIconY = iAimedY - (y + h/2);
			if (fabs (iDeltaIconY) < 3)  // filter useless tiny delta (and rounding errors).
				iDeltaIconY = 0;
			if (iDeltaIconY != 0)  // on va limiter la courbature du petit cote.
			{
				double dhmin = dh * MAX (1. - fabs (iDeltaIconY) / (h/2), .5);
				if (iDeltaIconY > 0)  // pointe decale vers la droite.
				{
					dh1 = dhmin;
					dh2 = dh;
				}
				else
				{
					dh1 = dh;
					dh2 = dhmin;
				}
			}
			else
			{
				dh1 = dh2 = dh;
			}
		break;
		default:
		break;
	}
	
	if (iMarginPosition == 1)  // top arrow
	{
		cairo_move_to (pCairoContext, fLineWidth/2, fTipHeight);
		cairo_rel_curve_to (pCairoContext,
			0, - dh2,
			(w - fLineWidth)/2 + iDeltaIconX, - (fTipHeight - dh2),
			(w - fLineWidth)/2 + iDeltaIconX, - fTipHeight);
		
		cairo_rel_curve_to (pCairoContext,
			0, dh1,
			(w - fLineWidth)/2 - iDeltaIconX, fTipHeight - dh1,
			(w - fLineWidth)/2 - iDeltaIconX, fTipHeight);
	}
	else
	{
		cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
		cairo_rel_line_to (pCairoContext, fFrameWidth, 0);
		//\_________________ Coin haut droit.
		if (iMarginPosition != 2)  // not right arrow
			cairo_arc (pCairoContext,
				fDockOffsetX + fFrameWidth, fDockOffsetY + fRadius,
				fRadius,
				-G_PI/2, 0.);
	}
	
	if (iMarginPosition == 2)  // right arrow
	{
		cairo_rel_curve_to (pCairoContext,
			dh2, 0,
			fTipHeight - dh2, fFrameHeight/2 + iDeltaIconY,
			fTipHeight, fFrameHeight/2 + iDeltaIconY);
		
		cairo_rel_curve_to (pCairoContext,
			- dh1, 0,
			- (fTipHeight - dh1), fFrameHeight/2 - iDeltaIconY,
			- fTipHeight, fFrameHeight/2 - iDeltaIconY);
	}
	else
	{
		cairo_rel_line_to (pCairoContext, 0, (fFrameHeight - fRadius * 2));
		//\_________________ Coin bas droit.
		if (iMarginPosition != 0)  // not bottom arrow
			cairo_arc (pCairoContext,
				fDockOffsetX + fFrameWidth, fDockOffsetY + fFrameHeight - fRadius,
				fRadius,
				0., G_PI/2);
	}
	
	if (iMarginPosition == 0)  // bottom arrow
	{
		cairo_rel_curve_to (pCairoContext,
			0, dh1,
			- (w - fLineWidth)/2 + iDeltaIconX, fTipHeight - dh1,
			- (w - fLineWidth)/2 + iDeltaIconX, fTipHeight);
		
		cairo_rel_curve_to (pCairoContext,
			0, - dh2,
			- (w - fLineWidth)/2 - iDeltaIconX, - (fTipHeight - dh2),
			- (w - fLineWidth)/2 - iDeltaIconX, - fTipHeight);
	}
	else
	{
		cairo_rel_line_to (pCairoContext, - fFrameWidth, 0);
		//\_________________ Coin bas gauche.
		if (iMarginPosition != 3)  // not left arrow
			cairo_arc (pCairoContext,
				fDockOffsetX, fDockOffsetY + fFrameHeight - fRadius,
				fRadius,
				G_PI/2, G_PI);
	}
	
	if (iMarginPosition == 3)  // left arrow
	{
		cairo_rel_curve_to (pCairoContext,
			- dh2, 0,
			- fTipHeight + dh2, - (fFrameHeight/2 + iDeltaIconY),
			- fTipHeight, -(fFrameHeight/2 + iDeltaIconY));
		
		cairo_rel_curve_to (pCairoContext,
			dh1, 0,
			fTipHeight - dh1, -(fFrameHeight/2 - iDeltaIconY),
			fTipHeight, - (fFrameHeight/2 - iDeltaIconY));
	}
	else
	{
		cairo_rel_line_to (pCairoContext, 0, (- fFrameHeight + fRadius * 2));
		//\_________________ Coin haut gauche.
		if (iMarginPosition != 1)  // not top arrow
			cairo_arc (pCairoContext,
				fDockOffsetX, fDockOffsetY + fRadius,
				fRadius,
				G_PI, -G_PI/2);
	}
	
	// draw the background
	if (myDialogsParam.bUseDefaultColors)
		gldi_style_colors_set_bg_color_full (pCairoContext, FALSE);
	else
		gldi_color_set_cairo_rgb (pCairoContext, &myDialogsParam.fBgColor);
	cairo_save (pCairoContext);
	cairo_clip_preserve (pCairoContext);
	gldi_style_colors_paint_bg_color_with_alpha (pCairoContext, alloc.width, myDialogsParam.bUseDefaultColors ? -1. : myDialogsParam.fBgColor.rgba.alpha);
	cairo_restore (pCairoContext);
	
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
}

static void _setup_menu (GtkWidget *pMenu)
{
	GldiMenuParams *pParams = g_object_get_data (G_OBJECT(pMenu), "gldi-params");
	pParams->iRadius = CD_RADIUS;
	pParams->fAlign = CD_ALIGN;
	pParams->iArrowHeight = CD_ARROW_HEIGHT;
}

void cd_decorator_register_curly (void)
{
	CairoDialogDecorator *pDecorator = g_new (CairoDialogDecorator, 1);
	pDecorator->set_size = cd_decorator_set_frame_size_curly;
	pDecorator->render = cd_decorator_draw_decorations_curly;
	pDecorator->render_opengl = NULL;
	pDecorator->setup_menu = _setup_menu;
	pDecorator->render_menu = _render_menu;
	pDecorator->cDisplayedName = D_ (MY_APPLET_DECORATOR_CURLY_NAME);
	cairo_dock_register_dialog_decorator (MY_APPLET_DECORATOR_CURLY_NAME, pDecorator);
}
