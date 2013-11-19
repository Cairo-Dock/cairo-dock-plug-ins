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
#include "applet-decorator-comics.h"

#define CD_TIP_INNER_MARGIN 12
#define CD_TIP_OUTER_MARGIN 25
#define CD_TIP_BASE 25
#define _CAIRO_DIALOG_COMICS_MARGIN 4
#define CD_ARROW_HEIGHT 16
#define CD_ALIGN 0.
#define CD_RADIUS (myDialogsParam.iCornerRadius * 1.5)

void cd_decorator_set_frame_size_comics (CairoDialog *pDialog)
{
	int iMargin = .5 * myDialogsParam.iLineWidth + (1. - sqrt (2) / 2) * CD_RADIUS + _CAIRO_DIALOG_COMICS_MARGIN;  // on laisse qques pixels d'espace en plus tout autour.
	pDialog->iRightMargin = iMargin;
	pDialog->iLeftMargin = iMargin;
	pDialog->iTopMargin = iMargin;
	pDialog->iBottomMargin = iMargin;
	pDialog->iMinBottomGap = CD_ARROW_HEIGHT;
	pDialog->iMinFrameWidth = CD_TIP_OUTER_MARGIN + CD_TIP_BASE + CD_TIP_INNER_MARGIN + 2*iMargin;
	pDialog->fAlign = CD_ALIGN;  // la pointe colle au bord du dialogue.
	pDialog->container.fRatio = 0.;  // pas de reflet merci.
	pDialog->container.bUseReflect = FALSE;
}

void cd_decorator_draw_decorations_comics (cairo_t *pCairoContext, CairoDialog *pDialog)
{
	double fLineWidth = myDialogsParam.iLineWidth;
	double fRadius = MIN (CD_RADIUS, (pDialog->iBubbleHeight - fLineWidth) / 2);
	
	/**double fGapFromDock = pDialog->iMinBottomGap + pDialog->iBottomMargin + fLineWidth/2;
	double cos_gamma = 1 / sqrt (1. + 1. * (CAIRO_DIALOG_TIP_MARGIN + CD_TIP_BASE) / fGapFromDock * (CAIRO_DIALOG_TIP_MARGIN + CD_TIP_BASE) / fGapFromDock);
	double cos_theta = 1 / sqrt (1. + 1. * CAIRO_DIALOG_TIP_MARGIN / fGapFromDock * CAIRO_DIALOG_TIP_MARGIN / fGapFromDock);
	double fTipHeight = fGapFromDock / (1. + fLineWidth / 2. / CD_TIP_BASE * (1./cos_gamma + 1./cos_theta));
	//g_print ("TipHeight <- %d\n", (int)fTipHeight);*/
	double fTipHeight =  pDialog->iMinBottomGap;
	
	int iWidth = pDialog->container.iWidth;
	double fMargin = 2 * fRadius + fLineWidth;
	double fBaseWidth = iWidth - fMargin;
	double fTipWidth = CD_TIP_OUTER_MARGIN + CD_TIP_BASE + CD_TIP_INNER_MARGIN;
	double fOffsetX = fRadius + fLineWidth / 2;
	double fOffsetY = fLineWidth / 2;
	
	// coin haut gauche.
	if (!pDialog->container.bDirectionUp)  // dessin a l'envers.
	{
		cairo_scale (pCairoContext, 1., -1.);
		cairo_translate (pCairoContext, 0., -pDialog->container.iHeight);
	}
	cairo_move_to (pCairoContext, fOffsetX, fOffsetY);
	
	cairo_rel_line_to (pCairoContext, fBaseWidth, 0);
	// Coin haut droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius, 0,
		fRadius, fRadius);
	cairo_rel_line_to (pCairoContext, 0, pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - fMargin);
	// Coin bas droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, fRadius,
		-fRadius, fRadius);
	// La pointe.
	gboolean bRight;
	if (pDialog->bRight)
		bRight = (pDialog->container.iWindowPositionX + iWidth > pDialog->iAimedX + fTipWidth);
	else
		bRight = (pDialog->container.iWindowPositionX + fTipWidth > pDialog->iAimedX);
	//g_print ("%d, %d, %d -> %d\n", pDialog->container.iWindowPositionX, (int) fTipWidth, pDialog->iAimedX, bRight);
	int iDeltaIconX;
	if (bRight)  // dialogue a droite de l'icone, pointe vers la gauche.
	{
		iDeltaIconX = MIN (0, pDialog->container.iWindowPositionX - pDialog->iAimedX);  // < 0
		//g_print ("iDeltaIconX right : %d / %d\n", iDeltaIconX, iWidth);
		cairo_rel_line_to (pCairoContext, -(fBaseWidth + iDeltaIconX - fTipWidth), 0);
		cairo_rel_curve_to (pCairoContext,
			- CD_TIP_OUTER_MARGIN, 0,
			- CD_TIP_OUTER_MARGIN, 0,
			- fTipWidth, fTipHeight);
		cairo_rel_curve_to (pCairoContext,
			CD_TIP_INNER_MARGIN, - fTipHeight,
			CD_TIP_INNER_MARGIN, - fTipHeight,
			0, - fTipHeight);
		cairo_rel_line_to (pCairoContext, iDeltaIconX, 0);
	}
	else  // dialogue a gauche de l'icone, pointe vers la droite.
	{
		iDeltaIconX = MAX (0, pDialog->container.iWindowPositionX + iWidth - pDialog->iAimedX);  // > 0
		//g_print ("iDeltaIconX left : %d / %d\n", iDeltaIconX, iWidth);
		cairo_rel_line_to (pCairoContext, - iDeltaIconX, 0);
		cairo_rel_curve_to (pCairoContext,
			- (CD_TIP_INNER_MARGIN), 0,
			- (CD_TIP_INNER_MARGIN), 0,
			0, fTipHeight);	
		cairo_rel_curve_to (pCairoContext,
			- (CD_TIP_INNER_MARGIN + CD_TIP_BASE), - fTipHeight,
			- (CD_TIP_INNER_MARGIN + CD_TIP_BASE), - fTipHeight,
			- fTipWidth, - fTipHeight);	
		cairo_rel_line_to (pCairoContext, - fBaseWidth + iDeltaIconX + fTipWidth, 0);
	}

	// Coin bas gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		-fRadius, 0,
		-fRadius, -fRadius);
	cairo_rel_line_to (pCairoContext, 0, - (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - fMargin));
	// Coin haut gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -fRadius,
		fRadius, -fRadius);
	if (fRadius < 1)
		cairo_close_path (pCairoContext);
	
	gldi_menu_set_bg_color (pCairoContext);
	///cairo_fill_preserve (pCairoContext);
	cairo_save (pCairoContext);
	cairo_clip_preserve (pCairoContext);
	gldi_menu_paint_bg_color (pCairoContext, pDialog->container.iWidth);
	cairo_restore (pCairoContext);

	cairo_set_line_width (pCairoContext, fLineWidth);
	gldi_menu_set_line_color (pCairoContext);
	cairo_stroke (pCairoContext);
}


static void _render_menu (GtkWidget *pMenu, cairo_t *pCairoContext)
{
	// get params
	GldiMenuParams *pParams = g_object_get_data (G_OBJECT(pMenu), "gldi-params");
	int iMarginPosition = -1;
	int iAimedX = 0;
	int ah = CD_ARROW_HEIGHT;
	if (pParams && pParams->pIcon)  // main menu
	{
		iMarginPosition = pParams->iMarginPosition;
		iAimedX = pParams->iAimedX;
	}
	GtkAllocation alloc;
	gtk_widget_get_allocation (pMenu, &alloc);
	int w = alloc.width, h = alloc.height;
	int x, y;
	gdk_window_get_position (gtk_widget_get_window (gtk_widget_get_toplevel(pMenu)), &x, &y);
	
	// get drawing params
	int _ah = ah - myDialogsParam.iLineWidth;  // we want the tip of the arrow (not the middle of the stroke) to reach the border; this is a good approximation
	double fRadius = CD_RADIUS, fLineWidth = myDialogsParam.iLineWidth;
	double fDockOffsetX = fLineWidth/2;
	double fDockOffsetY = fLineWidth/2;
	double fFrameWidth, fFrameHeight;
	fFrameWidth = w - fLineWidth;
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
	if (fFrameHeight < 2*fRadius)
		fRadius = fFrameHeight/2;
	if (fFrameWidth < 2*fRadius)
		fRadius = fFrameWidth/2;
	fFrameWidth -= 2*fRadius + fLineWidth;
	fDockOffsetX += fRadius;
	
	int iOuterMargin = CD_TIP_OUTER_MARGIN;
	int iBase = CD_TIP_BASE;
	int iInnerMargin = CD_TIP_INNER_MARGIN;
	double fTipWidth = CD_TIP_OUTER_MARGIN + CD_TIP_BASE + CD_TIP_INNER_MARGIN;
	switch (iMarginPosition)
	{
		case 0:  // bottom
		case 1:  // top
			if (fTipWidth > fFrameWidth)
			{
				double z = (double) fFrameWidth/fTipWidth;
				fTipWidth = fFrameWidth;
				iOuterMargin *= z;
				iInnerMargin *= z;
				iBase *= z;
			}
		break;
		case 2:  // right
		case 3:  // left
			if (fTipWidth > fFrameHeight - 2 * fRadius)
			{
				double z = (double) (fFrameHeight - 2 * fRadius)/fTipWidth;
				fTipWidth = fFrameHeight - 2 * fRadius;
				iOuterMargin *= z;
				iInnerMargin *= z;
				iBase *= z;
			}
		break;
		default:
		break;
	}
	
	// draw path
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	
	if (iMarginPosition == 1)  // top arrow
	{
		int iDeltaIconX = MIN (fFrameWidth, MAX (0, iAimedX - (x + fRadius)));
		if (iDeltaIconX + fTipWidth <= fFrameWidth)  // normal case: point to the left
		{
			cairo_rel_line_to (pCairoContext, iDeltaIconX, 0);
			cairo_rel_curve_to (pCairoContext,
				(iInnerMargin), 0,  // to adjust the curvature
				(iInnerMargin)/2, - _ah/2,
				0, - _ah);	
			cairo_rel_curve_to (pCairoContext,
				(iInnerMargin + iBase), _ah,
				(iInnerMargin + iBase), _ah,
				fTipWidth, _ah);	
			cairo_rel_line_to (pCairoContext, fFrameWidth - iDeltaIconX - fTipWidth, 0);
		}
		else
		{
			cairo_rel_line_to (pCairoContext, iDeltaIconX - fTipWidth, 0);
			cairo_rel_curve_to (pCairoContext,
				(iOuterMargin), 0,
				(iOuterMargin), 0,
				fTipWidth, - _ah);	
			cairo_rel_curve_to (pCairoContext,
				- (iInnerMargin)/2, _ah/2,  // to adjust the curvature
				- (iInnerMargin), _ah,
				0, _ah);
			cairo_rel_line_to (pCairoContext, fFrameWidth - iDeltaIconX, 0);	
		}
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
		cairo_rel_curve_to (pCairoContext,
			0, (iInnerMargin),
			_ah/2, (iInnerMargin)/2,  // to adjust the curvature
			_ah, 0);	
		cairo_rel_curve_to (pCairoContext,
			- _ah, (iInnerMargin + iBase),
			- _ah, (iInnerMargin + iBase),
			- _ah, fTipWidth);	
		cairo_rel_line_to (pCairoContext, 0, (fFrameHeight - 2*fRadius) - fTipWidth);
	}
	else
		cairo_rel_line_to (pCairoContext, 0, (fFrameHeight + fLineWidth - fRadius * 2));
	
	//\_________________ Coin bas droit.
	cairo_arc (pCairoContext,
		fDockOffsetX + fFrameWidth, fDockOffsetY + fFrameHeight - fLineWidth/2 - fRadius,
		fRadius,
		0., G_PI/2);
	
	if (iMarginPosition == 0)  // bottom arrow
	{
		int iDeltaIconX = MIN (fFrameWidth, MAX (0, x + w - iAimedX - fRadius));
		if (iDeltaIconX >= fTipWidth)  // normal case: point to the left
		{
			cairo_rel_line_to (pCairoContext, - iDeltaIconX + fTipWidth, 0);
			cairo_rel_curve_to (pCairoContext,
				- (iOuterMargin), 0,
				- (iOuterMargin), 0,
				- fTipWidth, _ah);	
			cairo_rel_curve_to (pCairoContext,
				(iInnerMargin)/2, - _ah/2,  // to adjust the curvature
				(iInnerMargin), - _ah,
				0, - _ah);	
			cairo_rel_line_to (pCairoContext, - fFrameWidth + iDeltaIconX, 0);
		}
		else
		{
			cairo_rel_line_to (pCairoContext, - iDeltaIconX, 0);
			cairo_rel_curve_to (pCairoContext,
				- (iInnerMargin), 0,  // to adjust the curvature
				- (iInnerMargin)/2, _ah/2,
				0, _ah);	
			cairo_rel_curve_to (pCairoContext,
				- (iInnerMargin + iBase), - _ah,
				- (iInnerMargin + iBase), - _ah,
				- fTipWidth, - _ah);
			cairo_rel_line_to (pCairoContext, - fFrameWidth + iDeltaIconX + fTipWidth, 0);
		}
	}
	else
		cairo_rel_line_to (pCairoContext, - fFrameWidth, 0);
	
	//\_________________ Coin bas gauche.
	cairo_arc (pCairoContext,
		fDockOffsetX, fDockOffsetY + fFrameHeight - fLineWidth/2 - fRadius,
		fRadius,
		G_PI/2, G_PI);
	
	if (iMarginPosition == 3)  // left arrow
	{
		cairo_rel_curve_to (pCairoContext,
			0, - (iOuterMargin),
			0, - (iOuterMargin),
			- _ah, - fTipWidth);	
		cairo_rel_curve_to (pCairoContext,
			_ah/2, (iInnerMargin)/2,  // to adjust the curvature
			_ah, (iInnerMargin),
			_ah, 0);
		cairo_rel_line_to (pCairoContext, 0, - (fFrameHeight - 2*fRadius) + fTipWidth);
	}
	else
		cairo_rel_line_to (pCairoContext, 0, (- fFrameHeight - fLineWidth + fRadius * 2));
	//\_________________ Coin haut gauche.
	cairo_arc (pCairoContext,
		fDockOffsetX, fDockOffsetY + fRadius,
		fRadius,
		G_PI, -G_PI/2);
	
	// draw outline
	if (fLineWidth != 0)  // draw the outline with same color as bg, but opaque
	{
		gldi_menu_set_line_color (pCairoContext);
		cairo_stroke_preserve (pCairoContext);
	}
	
	cairo_clip (pCairoContext);  // clip
	
	// draw the background
	gldi_menu_set_bg_color (pCairoContext);
	
	gldi_menu_paint_bg_color (pCairoContext, alloc.width);
}

static void _setup_menu (GtkWidget *pMenu)
{
	GldiMenuParams *pParams = g_object_get_data (G_OBJECT(pMenu), "gldi-params");
	pParams->iRadius = CD_RADIUS;
	pParams->fAlign = CD_ALIGN;
	pParams->iArrowHeight = CD_ARROW_HEIGHT;
}

void cd_decorator_register_comics (void)
{
	CairoDialogDecorator *pDecorator = g_new (CairoDialogDecorator, 1);
	pDecorator->set_size = cd_decorator_set_frame_size_comics;
	pDecorator->render = cd_decorator_draw_decorations_comics;
	pDecorator->render_opengl = NULL;
	pDecorator->setup_menu = _setup_menu;
	pDecorator->render_menu = _render_menu;
	pDecorator->cDisplayedName = D_ (MY_APPLET_DECORATOR_COMICS_NAME);
	cairo_dock_register_dialog_decorator (MY_APPLET_DECORATOR_COMICS_NAME, pDecorator);
}
