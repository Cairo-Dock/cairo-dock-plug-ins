

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

#define CAIRO_DIALOG_MIN_GAP 20
#define CD_TIP_INNER_MARGIN 12
#define CD_TIP_OUTER_MARGIN 25
#define CD_TIP_BASE 25
#define _CAIRO_DIALOG_COMICS_MARGIN 4

void cd_decorator_set_frame_size_comics (CairoDialog *pDialog)
{
	int iMargin = .5 * myConfig.iComicsLineWidth + (1. - sqrt (2) / 2) * myConfig.iComicsRadius + _CAIRO_DIALOG_COMICS_MARGIN;  // on laisse qques pixels d'espace en plus tout autour.
	g_print ("iMargin : %d\n", iMargin);
	pDialog->iRightMargin = iMargin;
	pDialog->iLeftMargin = iMargin;
	pDialog->iTopMargin = iMargin;
	pDialog->iBottomMargin = iMargin;
	pDialog->iMinBottomGap = CAIRO_DIALOG_MIN_GAP;
	pDialog->iMinFrameWidth = CD_TIP_OUTER_MARGIN + CD_TIP_BASE + CD_TIP_INNER_MARGIN + 2*iMargin;
	pDialog->fAlign = 0.;  // la pointe colle au bord du dialogue.
	pDialog->container.fRatio = 0.;  // pas de reflet merci.
	pDialog->container.bUseReflect = FALSE;
}

void cd_decorator_draw_decorations_comics (cairo_t *pCairoContext, CairoDialog *pDialog)
{
	double fLineWidth = myConfig.iComicsLineWidth;
	double fRadius = MIN (myConfig.iComicsRadius, pDialog->iBubbleHeight/2 - fLineWidth);
	
	/**double fGapFromDock = pDialog->iMinBottomGap + pDialog->iBottomMargin + fLineWidth/2;
	double cos_gamma = 1 / sqrt (1. + 1. * (CAIRO_DIALOG_TIP_MARGIN + CD_TIP_BASE) / fGapFromDock * (CAIRO_DIALOG_TIP_MARGIN + CD_TIP_BASE) / fGapFromDock);
	double cos_theta = 1 / sqrt (1. + 1. * CAIRO_DIALOG_TIP_MARGIN / fGapFromDock * CAIRO_DIALOG_TIP_MARGIN / fGapFromDock);
	double fTipHeight = fGapFromDock / (1. + fLineWidth / 2. / CD_TIP_BASE * (1./cos_gamma + 1./cos_theta));*/
	//g_print ("TipHeight <- %d\n", (int)fTipHeight);
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
	g_print ("%d, %d, %d -> %d\n", pDialog->container.iWindowPositionX, (int) fTipWidth, pDialog->iAimedX, bRight);
	int iDeltaIconX;
	if (bRight)  // dialogue a droite de l'icone, pointe vers la gauche.
	{
		iDeltaIconX = MIN (0, pDialog->container.iWindowPositionX - pDialog->iAimedX);  // < 0
		g_print ("iDeltaIconX right : %d / %d\n", iDeltaIconX, iWidth);
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
		g_print ("iDeltaIconX left : %d / %d\n", iDeltaIconX, iWidth);
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

	cairo_set_source_rgba (pCairoContext, myDialogs.fDialogColor[0], myDialogs.fDialogColor[1],	myDialogs.fDialogColor[2], myDialogs.fDialogColor[3]);
	cairo_fill_preserve (pCairoContext);

	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_set_source_rgba (pCairoContext, myConfig.fComicsLineColor[0], myConfig.fComicsLineColor[1], myConfig.fComicsLineColor[2], myConfig.fComicsLineColor[3]);
	cairo_stroke (pCairoContext);
}


void cd_decorator_register_comics (void)
{
	CairoDialogDecorator *pDecorator = g_new (CairoDialogDecorator, 1);
	pDecorator->set_size = cd_decorator_set_frame_size_comics;
	pDecorator->render = cd_decorator_draw_decorations_comics;
	pDecorator->render_opengl = NULL;
	pDecorator->cDisplayedName = D_ (MY_APPLET_DECORATOR_COMICS_NAME);
	cairo_dock_register_dialog_decorator (MY_APPLET_DECORATOR_COMICS_NAME, pDecorator);
}
