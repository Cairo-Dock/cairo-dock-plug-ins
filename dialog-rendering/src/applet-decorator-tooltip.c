/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-decorator-tooltip.h"

//A bosser
#define _CAIRO_DIALOG_TOOLTIP_MIN_GAP 10
#define _CAIRO_DIALOG_TOOLTIP_BORDER_PADDING 10
#define _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH 10
#define _CAIRO_DIALOG_TOOLTIP_ARROW_HEIGHT 5
#define CAIRO_DIALOG_TIP_ROUNDING_MARGIN 12
#define CAIRO_DIALOG_TIP_MARGIN 25
#define CAIRO_DIALOG_TIP_BASE 25


void cd_decorator_set_frame_size_tooltip (CairoDialog *pDialog) {
  int iMargin = .5 * myConfig.iTooltipLineWidth + (1. - sqrt (2) / 2) * myConfig.iTooltipRadius;
  pDialog->iRightMargin = iMargin;
  pDialog->iLeftMargin = iMargin;
  pDialog->iTopMargin = 0;
  pDialog->iBottomMargin = 0;
  pDialog->iMinBottomGap = _CAIRO_DIALOG_TOOLTIP_MIN_GAP;
  pDialog->iMinFrameWidth = CAIRO_DIALOG_TIP_MARGIN + CAIRO_DIALOG_TIP_ROUNDING_MARGIN + CAIRO_DIALOG_TIP_BASE;  // dans l'ordre.
  pDialog->fAlign = .5;
  pDialog->fReflectAlpha = 0.;
}


void cd_decorator_draw_decorations_tooltip (cairo_t *pCairoContext, CairoDialog *pDialog) {
  double fLineWidth = myConfig.iTooltipLineWidth;
  double fRadius = myConfig.iTooltipRadius;

  double fOffsetX = fRadius +     fLineWidth / 2;
  double fOffsetY = (pDialog->bDirectionUp ? fLineWidth / 2 : pDialog->iHeight - fLineWidth / 2);
  int     sens = (pDialog->bDirectionUp ? 1 :     -1);
  int     iWidth = pDialog->iWidth;

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
  cairo_rel_line_to (pCairoContext, 0, sens *     (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (2 * fRadius + fLineWidth)));

  // Coin bas     droit.
  cairo_rel_curve_to (pCairoContext,
    0, 0,
    0, sens * fRadius,
    -fRadius, sens * fRadius);

  // La pointe.
  double fDeltaMargin;
  if (pDialog->bRight) {
    fDeltaMargin = MAX (_CAIRO_DIALOG_TOOLTIP_BORDER_PADDING, pDialog->iAimedX    - pDialog->iPositionX - fRadius - fLineWidth - _CAIRO_DIALOG_TOOLTIP_BORDER_PADDING / 2);
    cairo_rel_line_to (pCairoContext, -iWidth + fLineWidth +    2 * fRadius + (3 * _CAIRO_DIALOG_TOOLTIP_BORDER_PADDING) -      fDeltaMargin,   0);     
    cairo_rel_line_to (pCairoContext, - _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH, sens * _CAIRO_DIALOG_TOOLTIP_ARROW_HEIGHT);
    cairo_rel_line_to (pCairoContext, - _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH, -sens * _CAIRO_DIALOG_TOOLTIP_ARROW_HEIGHT);
    cairo_rel_line_to (pCairoContext, - _CAIRO_DIALOG_TOOLTIP_BORDER_PADDING + fDeltaMargin, 0);
  }
  else {
    fDeltaMargin = MAX (_CAIRO_DIALOG_TOOLTIP_BORDER_PADDING, MIN (- (3 * _CAIRO_DIALOG_TOOLTIP_BORDER_PADDING )- fRadius - fLineWidth / 2, pDialog->iPositionX - pDialog->iAimedX - fRadius - fLineWidth / 2) + pDialog->iWidth);
    cairo_rel_line_to (pCairoContext, - (_CAIRO_DIALOG_TOOLTIP_BORDER_PADDING + fDeltaMargin), 0);
    cairo_rel_line_to (pCairoContext, - _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH, sens * _CAIRO_DIALOG_TOOLTIP_ARROW_HEIGHT);
    cairo_rel_line_to (pCairoContext, - _CAIRO_DIALOG_TOOLTIP_ARROW_WIDTH, -sens * _CAIRO_DIALOG_TOOLTIP_ARROW_HEIGHT);
    cairo_rel_line_to (pCairoContext, -iWidth + fLineWidth +    2 * fRadius + (3 * _CAIRO_DIALOG_TOOLTIP_BORDER_PADDING) + fDeltaMargin,        0);
  }

  // Coin bas     gauche.
  cairo_rel_curve_to (pCairoContext,
    0, 0,
    -fRadius, 0,
    -fRadius, -sens * fRadius);
  cairo_rel_line_to (pCairoContext, 0, - sens * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (2 * fRadius + fLineWidth)));

  // Coin haut gauche.
  cairo_rel_curve_to (pCairoContext,
    0, 0,
    0, -sens * fRadius,
    fRadius, -sens * fRadius);
  if (fRadius     < 1)
    cairo_close_path (pCairoContext);

  cairo_set_source_rgba (pCairoContext, myDialogs.fDialogColor[0], myDialogs.fDialogColor[1],     myDialogs.fDialogColor[2], myDialogs.fDialogColor[3]);
  cairo_fill_preserve (pCairoContext); //Notre fond
  cairo_set_source_rgba (pCairoContext, myConfig.fTooltipLineColor[0], myConfig.fTooltipLineColor[1], myConfig.fTooltipLineColor[2], myConfig.fTooltipLineColor[3]);
  cairo_set_line_width (pCairoContext, fLineWidth); //La ligne externe

  cairo_stroke (pCairoContext); //On ferme notre chemin

  if (pDialog->iIconSize != 0) {
    //Ajout d'un cadre pour l'icône (Pas d'alpha)
    int iIconFrameWidth = (pDialog->iIconSize / 2) - (2 * fRadius + fLineWidth);
    //cd_debug ("Tooltip: %d", iIconFrameWidth);
    //cairo_move_to (pCairoContext, 0, 0); //On revient a l'origine
    cairo_move_to (pCairoContext, fOffsetX + 2 * (fLineWidth + 1), fOffsetY + 2 * (fLineWidth + 1)); //Pour créer l'effet d'inclusion
    //On trace une ligne HautGauche -> HautDroit
    cairo_rel_line_to (pCairoContext, iIconFrameWidth, 0);
    //On trace une ligne HautDroit -> BasDroit
    cairo_rel_line_to (pCairoContext, 0, sens * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (fRadius + fLineWidth)));
    //On trace une ligne BasDroit -> BasGauche
    cairo_rel_line_to (pCairoContext, - iIconFrameWidth, 0);
    // Coin bas     gauche.
    cairo_rel_curve_to (pCairoContext,
      0, 0,
      -fRadius, 0,
      -fRadius, -sens * fRadius);
    //On trace une ligne BasGauche -> HautGauche
    cairo_rel_line_to (pCairoContext, 0, - sens * (pDialog->iBubbleHeight + pDialog->iTopMargin + pDialog->iBottomMargin - (2 * fRadius + 2 * (fLineWidth + 2.5))));

    // Coin haut gauche.
    cairo_rel_curve_to (pCairoContext,
      0, 0,
      0, -sens * fRadius,
      fRadius, -sens * fRadius);

    if (fRadius < 1)
      cairo_close_path (pCairoContext);

   /* double fBorderDialogColor[3];
    if (myDialogs.fDialogColor[0] <= .5 || myDialogs.fDialogColor[1] <= .5 || myDialogs.fDialogColor[2] <= .5) {
      fBorderDialogColor[0] = myDialogs.fDialogColor[0] + .1;
      fBorderDialogColor[1] = myDialogs.fDialogColor[1] + .1;
      fBorderDialogColor[2] = myDialogs.fDialogColor[2] + .1;
      //On eclaircit le fond
    }
    else {
      fBorderDialogColor[0] = myDialogs.fDialogColor[0] - .1;
      fBorderDialogColor[1] = myDialogs.fDialogColor[1] - .1;
      fBorderDialogColor[2] = myDialogs.fDialogColor[2] - .1;
      //On fonce le fond
    }*/
    cairo_set_source_rgba (pCairoContext, myConfig.fTooltipMarginColor[0], myConfig.fTooltipMarginColor[1],  myConfig.fTooltipMarginColor[2], myConfig.fTooltipMarginColor[3]);
    cairo_fill_preserve (pCairoContext);
    cairo_set_source_rgba (pCairoContext, myConfig.fTooltipLineColor[0], myConfig.fTooltipLineColor[1], myConfig.fTooltipLineColor[2], myConfig.fTooltipLineColor[3]);
    cairo_set_line_width (pCairoContext, fLineWidth);
    
    cairo_stroke (pCairoContext);
  }
}


void cd_decorator_register_tooltip (void)
{
	CairoDialogDecorator *pDecorator = g_new (CairoDialogDecorator, 1);
	pDecorator->set_size = cd_decorator_set_frame_size_tooltip;
	pDecorator->render = cd_decorator_draw_decorations_tooltip;
	cairo_dock_register_dialog_decorator (MY_APPLET_DECORATOR_TOOLTIP_NAME, pDecorator);
}
