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

#define _CAIRO_DIALOG_TOOLTIP_MIN_GAP 10
#define _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH 20
#define _CAIRO_DIALOG_TOOLTIP_ARROW_HEIGHT 5
#define _CAIRO_DIALOG_TOOLTIP_MARGIN 4


void cd_decorator_set_frame_size_tooltip (CairoDialog *pDialog)
{
	int iMargin = .5 * myConfig.iTooltipLineWidth + (1. - sqrt (2) / 2) * myConfig.iTooltipRadius;
	int iIconOffset = myDialogs.iDialogIconSize/2;
	pDialog->iRightMargin = iMargin;
	pDialog->iLeftMargin = iIconOffset + iMargin;
	pDialog->iTopMargin = iIconOffset + _CAIRO_DIALOG_TOOLTIP_MARGIN + myConfig.iTooltipLineWidth;
	pDialog->iBottomMargin = _CAIRO_DIALOG_TOOLTIP_MARGIN;
	pDialog->iMinBottomGap = _CAIRO_DIALOG_TOOLTIP_MIN_GAP;
	pDialog->iMinFrameWidth = _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH;
	pDialog->fAlign = .5;
	pDialog->container.fRatio = 0.;
	pDialog->container.bUseReflect = FALSE;
	pDialog->iIconOffsetX = iIconOffset;
	pDialog->iIconOffsetY = pDialog->iTopMargin;
}


void cd_decorator_draw_decorations_tooltip (cairo_t *pCairoContext, CairoDialog *pDialog)
{
	double fLineWidth = myConfig.iTooltipLineWidth;
	double fRadius = myConfig.iTooltipRadius;
	double fIconOffset = myDialogs.iDialogIconSize/2;
	
	double fOffsetX = fRadius + fLineWidth / 2 + fIconOffset;
	double fOffsetY = (pDialog->container.bDirectionUp ? fLineWidth / 2 : pDialog->container.iHeight - fLineWidth / 2) + (pDialog->container.bDirectionUp ? fIconOffset : -fIconOffset);
	int sens = (pDialog->container.bDirectionUp ? 1 : -1);
	int iWidth = pDialog->container.iWidth - fIconOffset;
	
	int iDeltaIconX = MIN (0, pDialog->iAimedX - (pDialog->container.iWindowPositionX + pDialog->container.iWidth/2));
	if (iDeltaIconX == 0)
		iDeltaIconX = MAX (0, pDialog->iAimedX - (pDialog->container.iWindowPositionX + pDialog->container.iWidth/2));
	if (fabs (iDeltaIconX) < 3)  // filter useless tiny delta (and rounding errors).
		iDeltaIconX = 0;
	else if (iDeltaIconX > pDialog->container.iWidth/2 - (fRadius + fLineWidth / 2))
		iDeltaIconX = pDialog->container.iWidth/2 - (fRadius + fLineWidth / 2);
	else if (iDeltaIconX < - pDialog->container.iWidth/2 + fRadius + fLineWidth / 2)
		iDeltaIconX = - pDialog->container.iWidth/2 + fRadius + fLineWidth / 2;
	//g_print ("aim: %d, window: %d, width: %d => %d\n", pDialog->iAimedX, pDialog->container.iWindowPositionX, pDialog->container.iWidth, iDeltaIconX);
	
	int iArrowShift;
	if (iDeltaIconX != 0)  // il y'a un decalage, on va limiter la pente du cote le plus court de la pointe a 30 degres.
	{
		iArrowShift = MAX (0, fabs (iDeltaIconX) - _CAIRO_DIALOG_TOOLTIP_ARROW_HEIGHT * .577 - _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH/2);  // tan(30)
		if (iDeltaIconX < 0)
			iArrowShift = - iArrowShift;
		//g_print ("iArrowShift: %d\n", iArrowShift);
		
	}
	else
		iArrowShift = 0;
	
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
	cairo_rel_line_to (pCairoContext, 0, sens *     (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (2 * fRadius + fLineWidth + fIconOffset)));
	
	// Coin bas droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, sens * fRadius,
		-fRadius, sens * fRadius);
	
	// La pointe.
	double fDemiWidth = (iWidth - fLineWidth - 2 * fRadius - _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH)/2;
	
	if (- fDemiWidth + iArrowShift > 0)
		iArrowShift = fDemiWidth;
	else if (- fDemiWidth - iArrowShift > 0)
		iArrowShift = - fDemiWidth;
	cairo_rel_line_to (pCairoContext, - fDemiWidth + iArrowShift, 0);
	cairo_rel_line_to (pCairoContext, - _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH/2 - iArrowShift + iDeltaIconX, sens * _CAIRO_DIALOG_TOOLTIP_ARROW_HEIGHT);
	cairo_rel_line_to (pCairoContext, - _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH/2 + iArrowShift - iDeltaIconX, -sens * _CAIRO_DIALOG_TOOLTIP_ARROW_HEIGHT);
	cairo_rel_line_to (pCairoContext, - fDemiWidth - iArrowShift , 0);
	
	// Coin bas gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		-fRadius, 0,
		-fRadius, -sens * fRadius);
	
	// On remonte.
	cairo_rel_line_to (pCairoContext, 0, - sens * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (2 * fRadius + fLineWidth + fIconOffset)));
	
	// Coin haut gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -sens * fRadius,
		fRadius, -sens * fRadius);
	if (fRadius < 1)
		cairo_close_path (pCairoContext);
	
	cairo_set_source_rgba (pCairoContext, myDialogs.fDialogColor[0], myDialogs.fDialogColor[1],     myDialogs.fDialogColor[2], myDialogs.fDialogColor[3]);
	cairo_fill_preserve (pCairoContext); //Notre fond
	cairo_set_source_rgba (pCairoContext, myConfig.fTooltipLineColor[0], myConfig.fTooltipLineColor[1], myConfig.fTooltipLineColor[2], myConfig.fTooltipLineColor[3]);
	cairo_set_line_width (pCairoContext, fLineWidth); //La ligne externe
	
	cairo_stroke (pCairoContext); //On ferme notre chemin
}


void cd_decorator_register_tooltip (void)
{
	CairoDialogDecorator *pDecorator = g_new (CairoDialogDecorator, 1);
	pDecorator->set_size = cd_decorator_set_frame_size_tooltip;
	pDecorator->render = cd_decorator_draw_decorations_tooltip;
	pDecorator->render_opengl = NULL;
	pDecorator->cDisplayedName = D_ (MY_APPLET_DECORATOR_TOOLTIP_NAME);
	cairo_dock_register_dialog_decorator (MY_APPLET_DECORATOR_TOOLTIP_NAME, pDecorator);
}
