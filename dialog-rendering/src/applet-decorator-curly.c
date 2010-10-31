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


void cd_decorator_set_frame_size_curly (CairoDialog *pDialog)
{
	int iMargin = .5 * myConfig.iCurlyLineWidth + (1. - sqrt (2) / 2) * myConfig.iCurlyRadius;
	pDialog->iRightMargin = iMargin;
	pDialog->iLeftMargin = iMargin;
	pDialog->iTopMargin = iMargin;
	pDialog->iBottomMargin = 2*iMargin;
	pDialog->iMinBottomGap = MAX (20, 2*myConfig.iCurlyRadius);
	pDialog->iMinFrameWidth = 10;  // au pif.
	pDialog->fAlign = .5;
	pDialog->container.fRatio = 0.;
	pDialog->container.bUseReflect = FALSE;
}


void cd_decorator_draw_decorations_curly (cairo_t *pCairoContext, CairoDialog *pDialog)
{
	double fLineWidth = myConfig.iCurlyLineWidth;
	double fRadius = myConfig.iCurlyRadius;
	double fBottomRadius = 2 * fRadius;
	double fTipHeight = pDialog->iMinBottomGap + 0*pDialog->iBottomMargin + fLineWidth/2;
	double dh = MIN (MAX (1, myConfig.fCurlyCurvature * fTipHeight), .4 * pDialog->container.iWidth);
	
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
	double fDeltaTop = MIN (pDialog->iTopMargin, .2 * fDemiWidth);
	cairo_rel_curve_to (pCairoContext,
		fDemiWidth/2, 0,
		fDemiWidth/2, sens * fDeltaTop,
		fDemiWidth, sens * fDeltaTop);
	cairo_rel_curve_to (pCairoContext,
		fDemiWidth/2, 0,
		fDemiWidth/2, - sens * fDeltaTop,
		fDemiWidth, - sens * fDeltaTop);
	
	// Coin haut droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius, 0,
		fRadius, sens * fRadius);
	
	// Ligne droite. (Haut droit -> Bas droit)
	double fDemiHeight = .5 * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (fRadius + fLineWidth/2));
	double fDeltaSide = MIN (pDialog->iRightMargin, .2 * fDemiHeight);
	if (myConfig.bCulrySideToo)
	{
		cairo_rel_curve_to (pCairoContext,
			0, sens * fDemiHeight/2,
			- .5 * fDeltaSide, sens * fDemiHeight/2,
			- .5 * fDeltaSide, sens * fDemiHeight);
		cairo_rel_curve_to (pCairoContext,
			0, sens * fDemiHeight/2,
			.5 * fDeltaSide, sens * fDemiHeight/2,
			.5 * fDeltaSide, sens * fDemiHeight);
	}
	else
	{
		cairo_rel_line_to (pCairoContext,
			0,
			sens * fDemiHeight * 2);
	}
	
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
	if (myConfig.bCulrySideToo)
	{
		cairo_rel_curve_to (pCairoContext,
			0, - sens * fDemiHeight/2,
			.5 * fDeltaSide, - sens * fDemiHeight/2,
			.5 * fDeltaSide, - sens * fDemiHeight);
		cairo_rel_curve_to (pCairoContext,
			0, - sens * fDemiHeight/2,
			- .5 * fDeltaSide, - sens * fDemiHeight/2,
			- .5 * fDeltaSide, - sens * fDemiHeight);
	}
	else
	{
		cairo_rel_line_to (pCairoContext,
			0,
			- sens * fDemiHeight * 2);
	}
	// Coin haut gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -sens * fRadius,
		fRadius, -sens * fRadius);
	if (fRadius < 1)
		cairo_close_path (pCairoContext);
	
	// On remplit le fond.
	cairo_set_source_rgba (pCairoContext, myDialogs.fDialogColor[0], myDialogs.fDialogColor[1], myDialogs.fDialogColor[2], myDialogs.fDialogColor[3]);
	cairo_fill_preserve (pCairoContext);
	
	// On trace le contour.
	cairo_set_source_rgba (pCairoContext, myConfig.fCurlyLineColor[0], myConfig.fCurlyLineColor[1], myConfig.fCurlyLineColor[2], myConfig.fCurlyLineColor[3]);
	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_stroke (pCairoContext);
}


void cd_decorator_register_curly (void)
{
	CairoDialogDecorator *pDecorator = g_new (CairoDialogDecorator, 1);
	pDecorator->set_size = cd_decorator_set_frame_size_curly;
	pDecorator->render = cd_decorator_draw_decorations_curly;
	pDecorator->render_opengl = NULL;
	pDecorator->cDisplayedName = D_ (MY_APPLET_DECORATOR_CURLY_NAME);
	cairo_dock_register_dialog_decorator (MY_APPLET_DECORATOR_CURLY_NAME, pDecorator);
}
