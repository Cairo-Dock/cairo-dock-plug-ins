/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-decorator-comics.h"

#define CAIRO_DIALOG_MIN_GAP 20
#define CAIRO_DIALOG_TIP_ROUNDING_MARGIN 12
#define CAIRO_DIALOG_TIP_MARGIN 25
#define CAIRO_DIALOG_TIP_BASE 25


void cd_decorator_set_frame_size_comics (CairoDialog *pDialog)
{
	int iMargin = .5 * myConfig.iComicsLineWidth + (1. - sqrt (2) / 2) * myConfig.iComicsRadius;
	pDialog->iRightMargin = iMargin;
	pDialog->iLeftMargin = iMargin;
	pDialog->iTopMargin = iMargin;
	pDialog->iBottomMargin = iMargin;
	pDialog->iMinBottomGap = CAIRO_DIALOG_MIN_GAP;
	pDialog->iMinFrameWidth = CAIRO_DIALOG_TIP_MARGIN + CAIRO_DIALOG_TIP_ROUNDING_MARGIN + CAIRO_DIALOG_TIP_BASE;  // dans l'ordre.
	pDialog->fAlign = 0.;  // la pointe colle au bord du dialogue.
	pDialog->fReflectAlpha = 0.;  // pas de reflet merci.
}

void cd_decorator_draw_decorations_comics (cairo_t *pCairoContext, CairoDialog *pDialog)
{
	double fLineWidth = myConfig.iComicsLineWidth;
	double fRadius = myConfig.iComicsRadius;
	
	double fGapFromDock = pDialog->iDistanceToDock + .5 * fLineWidth;
	double cos_gamma = 1 / sqrt (1. + 1. * (CAIRO_DIALOG_TIP_MARGIN + CAIRO_DIALOG_TIP_BASE) / fGapFromDock * (CAIRO_DIALOG_TIP_MARGIN + CAIRO_DIALOG_TIP_BASE) / fGapFromDock);
	double cos_theta = 1 / sqrt (1. + 1. * CAIRO_DIALOG_TIP_MARGIN / fGapFromDock * CAIRO_DIALOG_TIP_MARGIN / fGapFromDock);
	double fTipHeight = fGapFromDock / (1. + fLineWidth / 2. / CAIRO_DIALOG_TIP_BASE * (1./cos_gamma + 1./cos_theta));
	//g_print ("TipHeight <- %d\n", (int)fTipHeight);

	double fOffsetX	= fRadius +	fLineWidth / 2;
	double fOffsetY	= (pDialog->bDirectionUp ? fLineWidth / 2 : pDialog->iHeight - fLineWidth / 2);
	int	sens = (pDialog->bDirectionUp ?	1 :	-1);
	cairo_move_to (pCairoContext, fOffsetX, fOffsetY);
	//g_print ("  fOffsetX : %.2f; fOffsetY	: %.2f\n", fOffsetX, fOffsetY);
	int	iWidth = pDialog->iWidth;

	cairo_rel_line_to (pCairoContext, iWidth - (2 *	fRadius + fLineWidth), 0);
	// Coin	haut droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius, 0,
		fRadius, sens *	fRadius);
	cairo_rel_line_to (pCairoContext, 0, sens *	(pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (2 * fRadius + fLineWidth)));
	// Coin	bas	droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, sens	* fRadius,
		-fRadius, sens * fRadius);
	// La pointe.
	double fDeltaMargin;
	if (pDialog->bRight)
	{
		fDeltaMargin = MAX (0, pDialog->iAimedX	- pDialog->iPositionX -	fRadius	- fLineWidth / 2);
		//g_print ("fDeltaMargin : %.2f\n",	fDeltaMargin);
		cairo_rel_line_to (pCairoContext, -iWidth +	fDeltaMargin + fLineWidth +	2 * fRadius + CAIRO_DIALOG_TIP_MARGIN + CAIRO_DIALOG_TIP_BASE + CAIRO_DIALOG_TIP_ROUNDING_MARGIN ,	0);	
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			- CAIRO_DIALOG_TIP_ROUNDING_MARGIN,	0,
			- (CAIRO_DIALOG_TIP_ROUNDING_MARGIN	+ CAIRO_DIALOG_TIP_MARGIN +	CAIRO_DIALOG_TIP_BASE),	sens * fTipHeight);
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			CAIRO_DIALOG_TIP_MARGIN, - sens	* fTipHeight,
			CAIRO_DIALOG_TIP_MARGIN	- CAIRO_DIALOG_TIP_ROUNDING_MARGIN,	- sens * fTipHeight);
		cairo_rel_line_to (pCairoContext, -	CAIRO_DIALOG_TIP_MARGIN	- fDeltaMargin + CAIRO_DIALOG_TIP_ROUNDING_MARGIN, 0);
	}
	else
	{
		fDeltaMargin = MAX (0, MIN (- CAIRO_DIALOG_TIP_MARGIN -	CAIRO_DIALOG_TIP_ROUNDING_MARGIN - CAIRO_DIALOG_TIP_BASE - fRadius - fLineWidth / 2, pDialog->iPositionX - pDialog->iAimedX	- fRadius -	fLineWidth / 2)	+ pDialog->iWidth);
		//g_print ("fDeltaMargin : %.2f	/ %d\n", fDeltaMargin, pDialog->iWidth);
		cairo_rel_line_to (pCairoContext, -	(CAIRO_DIALOG_TIP_MARGIN + fDeltaMargin) + CAIRO_DIALOG_TIP_ROUNDING_MARGIN, 0);
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			-CAIRO_DIALOG_TIP_ROUNDING_MARGIN, 0,
			CAIRO_DIALOG_TIP_MARGIN	- CAIRO_DIALOG_TIP_ROUNDING_MARGIN,	sens * fTipHeight);
		cairo_rel_curve_to (pCairoContext,
			0, 0,
			- (CAIRO_DIALOG_TIP_MARGIN + CAIRO_DIALOG_TIP_BASE), - sens	* fTipHeight,
			- (CAIRO_DIALOG_TIP_MARGIN + CAIRO_DIALOG_TIP_BASE)	- CAIRO_DIALOG_TIP_ROUNDING_MARGIN,	- sens * fTipHeight);
		cairo_rel_line_to (pCairoContext, -iWidth +	fDeltaMargin + fLineWidth +	2 *	fRadius	+ CAIRO_DIALOG_TIP_MARGIN +	CAIRO_DIALOG_TIP_BASE +	CAIRO_DIALOG_TIP_ROUNDING_MARGIN, 0);
	}

	// Coin	bas	gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		-fRadius, 0,
		-fRadius, -sens	* fRadius);
	cairo_rel_line_to (pCairoContext, 0, - sens * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (2 * fRadius + fLineWidth)));
	// Coin	haut gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -sens * fRadius,
		fRadius, -sens * fRadius);
	if (fRadius	< 1)
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
	cairo_dock_register_dialog_decorator (MY_APPLET_DECORATOR_COMICS_NAME, pDecorator);
}
