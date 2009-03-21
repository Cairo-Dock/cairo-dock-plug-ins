/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-decorator-curly.h"


void cd_decorator_set_frame_size_curly (CairoDialog *pDialog) {
	int iMargin = .5 * myConfig.iCurlyLineWidth + (1. - sqrt (2) / 2) * myConfig.iCurlyRadius;
	pDialog->iRightMargin = iMargin;
	pDialog->iLeftMargin = iMargin;
	pDialog->iTopMargin = iMargin;
	pDialog->iBottomMargin = iMargin;
	pDialog->iMinBottomGap = MAX (20, myConfig.iCurlyRadius);
	pDialog->iMinFrameWidth = 10;  // au pif.
	pDialog->fAlign = .5;
	pDialog->fReflectAlpha = 0.;
}


void cd_decorator_draw_decorations_curly (cairo_t *pCairoContext, CairoDialog *pDialog) {
	double fLineWidth = myConfig.iCurlyLineWidth;
	double fRadius = myConfig.iCurlyRadius;
	double fTipHeight = MIN (pDialog->iDistanceToDock, 20);  // on completera par un trait.
	double dh = MAX (1, myConfig.fCurlyCurvature * fTipHeight);
	
	double fOffsetX = fRadius + fLineWidth / 2;
	double fOffsetY = (pDialog->bDirectionUp ? fLineWidth / 2 : pDialog->iHeight - fLineWidth / 2);
	int sens = (pDialog->bDirectionUp ? 1 : -1);
	double fDemiWidth = .5 * pDialog->iWidth - fRadius - fLineWidth/2;
	
	//On se déplace la ou il le faut
	cairo_move_to (pCairoContext, fOffsetX, fOffsetY);
	
	// Ligne du haut (Haut gauche -> Haut Droite)
	///cairo_rel_line_to (pCairoContext, iWidth - (2 * fRadius + fLineWidth), 0);
	cairo_rel_curve_to (pCairoContext,
		fDemiWidth/2, 0,
		fDemiWidth/2, sens * pDialog->iTopMargin,
		fDemiWidth, sens * pDialog->iTopMargin);
	cairo_rel_curve_to (pCairoContext,
		fDemiWidth/2, 0,
		fDemiWidth/2, - sens * pDialog->iTopMargin,
		fDemiWidth, - sens * pDialog->iTopMargin);
	
	// Coin haut droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius, 0,
		fRadius, sens * fRadius);
	
	// Ligne droite. (Haut droit -> Bas droit)
	double fDemiHeight = .5 * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (fRadius + fLineWidth/2));
	cairo_rel_curve_to (pCairoContext,
		0, sens * fDemiHeight/2,
		- .5 * pDialog->iRightMargin, sens * fDemiHeight/2,
		- .5 * pDialog->iRightMargin, sens * fDemiHeight);
	cairo_rel_curve_to (pCairoContext,
		0, sens * fDemiHeight/2,
		.5 * pDialog->iRightMargin, sens * fDemiHeight/2,
		.5 * pDialog->iRightMargin, sens * fDemiHeight);
	///cairo_rel_line_to (pCairoContext, 0, sens * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (2 * fRadius + fLineWidth)));
	
	fDemiWidth = .5 * pDialog->iWidth - fLineWidth/2;
	// Coin bas droit et pointe.
	cairo_rel_curve_to (pCairoContext,
		0, sens * dh,
		- fDemiWidth, sens * (fTipHeight - dh),
		- fDemiWidth, sens * fTipHeight);
	
	// Coin bas gauche et pointe.
	cairo_rel_curve_to (pCairoContext,
		0, - sens * dh,
		- fDemiWidth, - sens * (fTipHeight - dh),
		- fDemiWidth, - sens * fTipHeight);
	
	// On remonte par la gauche.
	///cairo_rel_line_to (pCairoContext, 0, - sens * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (2 * fRadius + fLineWidth)));
	cairo_rel_curve_to (pCairoContext,
		0, - sens * fDemiHeight/2,
		.5 * pDialog->iRightMargin, - sens * fDemiHeight/2,
		.5 * pDialog->iRightMargin, - sens * fDemiHeight);
	cairo_rel_curve_to (pCairoContext,
		0, - sens * fDemiHeight/2,
		- .5 * pDialog->iRightMargin, - sens * fDemiHeight/2,
		- .5 * pDialog->iRightMargin, - sens * fDemiHeight);
	
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
	
	if (fTipHeight < pDialog->iDistanceToDock)  // on descend jusqu'a l'icone.
	{
		double fGap = pDialog->iDistanceToDock - fTipHeight;
		cairo_move_to (pCairoContext,
			pDialog->iWidth/2,
			pDialog->bDirectionUp ? pDialog->iHeight - fGap : fGap);
		cairo_rel_line_to (pCairoContext, 0, sens * fGap);
		cairo_stroke (pCairoContext);
	}
}


void cd_decorator_register_curly (void)
{
	CairoDialogDecorator *pDecorator = g_new (CairoDialogDecorator, 1);
	pDecorator->set_size = cd_decorator_set_frame_size_curly;
	pDecorator->render = cd_decorator_draw_decorations_curly;
	pDecorator->render_opengl = NULL;
	cairo_dock_register_dialog_decorator (MY_APPLET_DECORATOR_CURLY_NAME, pDecorator);
}
