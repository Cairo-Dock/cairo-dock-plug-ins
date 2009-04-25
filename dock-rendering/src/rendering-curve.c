/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

This rendering has been written by parAdOxxx_ZeRo (http://paradoxxx.zero.free.fr)
 and Fabounet.

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>

#include "rendering-commons.h"
#include "rendering-curve.h"
#include "rendering-3D-plane.h"


static double *s_pReferenceCurveS = NULL;
static double *s_pReferenceCurveX = NULL;
static double *s_pReferenceCurveY = NULL;
int iVanishingPointY = 200;

extern cairo_surface_t *my_pFlatSeparatorSurface[2];

extern CDSpeparatorType my_curve_iDrawSeparator3D;

extern gdouble my_fCurveCurvature;
extern gint my_iCurveAmplitude;

//const guint curveOffsetX = 75;
// OM(t) = sum ([k=0..n] Bn,k(t)*OAk)
// Bn,k(x) = Cn,k*x^k*(1-x)^(n-k)

#define xCurve(a, t) (t * (t * t + 1.5 * (1 - t) * (1 - a + 2 * a * t)))
#define yCurve(t) (3 * t * (1 - t))
#define XCurve(W, a, t) (W * xCurve (a, t))
#define YCurve(h, t) (h * yCurve (t))

void cd_rendering_calculate_max_dock_size_curve (CairoDock *pDock)
{
	static double fCurveCurvature= 0;
	if (s_pReferenceCurveS == NULL || my_fCurveCurvature != fCurveCurvature)
	{
		fCurveCurvature = my_fCurveCurvature;
		cd_rendering_calculate_reference_curve (my_fCurveCurvature);
	}
	
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->fFlatDockWidth, pDock->iScrollOffset);
	
	pDock->iDecorationsHeight = myBackground.iFrameMargin + my_iCurveAmplitude + .5 * pDock->iMaxIconHeight;  // de bas en haut.
	
	pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., 0.));  // etendue max des icones, sans le cadre.
	//g_print ("iMaxDockWidth : %d\n", pDock->iMaxDockWidth);
	
	double h = 4./3 * (pDock->iDecorationsHeight + myBackground.iDockLineWidth);  // hauteur de controle de la courbe de Bezier, de telle facon qu'elle atteigne 'iDecorationsHeight'.
	double hi = .5 * pDock->iMaxIconHeight + myBackground.iFrameMargin - 1;  // hauteur de la courbe a la 1ere icone.
	double ti = .5 * (1. - sqrt (MAX (1. - 4./3 * hi / h, 0.01)));
	double xi = xCurve (my_fCurveCurvature, ti);
	double fDeltaX = pDock->iMaxDockWidth * xi / (1 - 2 * xi);  // abscisse de la 1ere icone pour satisfaire a la contrainte y=hi.
	//g_print ("ti = %.2f => xi = %.2f => fDeltaX = %.2f\n", ti, xi, fDeltaX);
	//g_print ("my_fCurveCurvature:%.2f\n", my_fCurveCurvature);
	pDock->iMaxDockWidth += 2*fDeltaX;
	double tan_theta = (my_fCurveCurvature != 1 ? h / ((1 - my_fCurveCurvature) * pDock->iMaxDockWidth / 2) : 1e6);  // la tangente a une courbe de Bezier en son origine est la droite reliant les deux premiers points de controle.
	double fDeltaTip = .5 * myBackground.iDockLineWidth * sqrt (1 + tan_theta * tan_theta) / tan_theta;  // prolongement de la pointe.
	pDock->iMaxDockWidth += 2 * fDeltaTip;
	pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., 2*(fDeltaX+fDeltaTip)));
	//g_print ("fDeltaTip : %.2f\n", fDeltaTip);
	
	pDock->iMaxDockHeight = myBackground.iDockLineWidth + myBackground.iFrameMargin + my_iCurveAmplitude + (1 + g_fAmplitude) * pDock->iMaxIconHeight + myLabels.iconTextDescription.iSize;  // de bas en haut.
	
	pDock->iDecorationsWidth = pDock->iMaxDockWidth - 4 * fDeltaTip;
	
	pDock->iMinDockWidth = pDock->fFlatDockWidth / (1 - 2 * xi) + 2 * fDeltaTip;
	//g_print ("pDock->fFlatDockWidth = %.2f => pDock->iMinDockWidth = %d\n", pDock->fFlatDockWidth, pDock->iMinDockWidth);
	
	pDock->iMinDockHeight = myBackground.iDockLineWidth + myBackground.iFrameMargin + my_iCurveAmplitude + pDock->iMaxIconHeight;  // de bas en haut.
	
	if (my_pFlatSeparatorSurface[0] == NULL && (my_curve_iDrawSeparator3D == CD_FLAT_SEPARATOR || my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR))
		cd_rendering_load_flat_separator (CAIRO_CONTAINER (g_pMainDock));
}


static void cd_rendering_make_3D_curve_separator (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bIncludeEdges, gboolean bBackGround)
{
	double fLineWidth = myBackground.iDockLineWidth;
	double fMargin = myBackground.iFrameMargin;
	double hi;
	
	Icon *pPrevIcon = cairo_dock_get_previous_icon (pDock->icons, icon);
	if (pPrevIcon == NULL)
		pPrevIcon = icon;
	Icon *pNextIcon = cairo_dock_get_next_icon (pDock->icons, icon);
	if (pNextIcon == NULL)
		pNextIcon = icon;
	
	double fVanishingDistanceLeft, fVanishingDistanceRight;
	double fDeltaInterIconLeft, fDeltaInterIconRight;
	if (pDock->bDirectionUp)
	{
		hi = pDock->iCurrentHeight - (icon->fDrawY + icon->fHeight * icon->fScale);
		fVanishingDistanceLeft = iVanishingPointY + pPrevIcon->fDrawY + pPrevIcon->fHeight * pPrevIcon->fScale;
		fVanishingDistanceRight = iVanishingPointY + pNextIcon->fDrawY + pNextIcon->fHeight * pNextIcon->fScale;
		
		fDeltaInterIconLeft = (pPrevIcon->fDrawY + pPrevIcon->fHeight * pPrevIcon->fScale) - (icon->fDrawY + icon->fHeight * icon->fScale);
		fDeltaInterIconRight = (icon->fDrawY + icon->fHeight * icon->fScale) - (pNextIcon->fDrawY + pNextIcon->fHeight * pNextIcon->fScale);
	}
	else
	{
		hi = icon->fDrawY;
		fVanishingDistanceLeft = iVanishingPointY + pDock->iCurrentHeight - pPrevIcon->fDrawY;
		fVanishingDistanceRight = iVanishingPointY + pDock->iCurrentHeight - pNextIcon->fDrawY;
		
		fDeltaInterIconLeft = (pPrevIcon->fDrawY) - (icon->fDrawY);
		fDeltaInterIconRight = (icon->fDrawY) - (pNextIcon->fDrawY);
	}
	double fLeftInclination = (icon->fDrawX - pDock->iCurrentWidth / 2) / fVanishingDistanceLeft;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->iCurrentWidth / 2) / fVanishingDistanceRight;
	
	if (bBackGround || ! bIncludeEdges)  // pour s'arreter sur la courbe, on realise un clippage.
	{
		//\________________ On se ramene au cas du dessin optimise.
		double x0, y0, xf, yf, w0, h0;
		if (pDock->bDirectionUp)
		{
			x0 = icon->fDrawX - MAX (0, fLeftInclination * (pPrevIcon->fDrawY + pPrevIcon->fHeight * pPrevIcon->fScale));
			xf = icon->fDrawX + icon->fWidth * icon->fScale - MIN (0, fRightInclination * (pNextIcon->fDrawY + pNextIcon->fHeight * pNextIcon->fScale));
		}
		else
		{
			x0 = icon->fDrawX - MAX (0, fLeftInclination * (pDock->iCurrentHeight - (pPrevIcon->fDrawY)));
			xf = icon->fDrawX + icon->fWidth * icon->fScale - MIN (0, fRightInclination * (pDock->iCurrentHeight - (pNextIcon->fDrawY)));
		}
		if (! bIncludeEdges)  // on prolonge jusqu'en bas.
		{
			if (pDock->bDirectionUp)
			{
				x0 += MIN (0, fLeftInclination * (pDock->iCurrentHeight - icon->fDrawY - icon->fHeight * icon->fScale));
				xf += MAX (0, fRightInclination * (pDock->iCurrentHeight - icon->fDrawY - icon->fHeight * icon->fScale));
			}
			else
			{
				x0 += MIN (0, fLeftInclination * (pPrevIcon->fDrawY));
				xf += MAX (0, fRightInclination * (pNextIcon->fDrawY));
			}
		}
		//g_print ("x0:%.2f -> xf:%.2f\n", x0, xf);
		y0 = 0;
		yf = icon->fDrawY;
		w0 = xf - x0;
		h0 = yf - y0;
		
		int sens;
		double fDockOffsetY;  // Offset du coin haut gauche du cadre.
		if (pDock->bDirectionUp)
		{
			sens = 1;
			fDockOffsetY = pDock->iCurrentHeight - .5 * fLineWidth;
		}
		else
		{
			sens = -1;
			fDockOffsetY = .5 * fLineWidth;
		}
		
		double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock) - 2 * myBackground.iFrameMargin;
		
		double h = 4./3 * (pDock->iDecorationsHeight + myBackground.iDockLineWidth);
		double hi_ = .5 * pDock->iMaxIconHeight + myBackground.iFrameMargin - 1;
		double ti = .5 * (1. - sqrt (MAX (1. - 4./3 * hi_ / h, 0)));
		double xi = xCurve (my_fCurveCurvature, ti);
		double curveOffsetX = fDockWidth * xi / (1 - 2 * xi);
		
		Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
		double fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX - curveOffsetX : fLineWidth / 2);
		
		
		//\________________ On approche le morceau de courbe de Bezier par des trapezes.
		double x = (x0 - fDockOffsetX) / (fDockWidth + 2 * curveOffsetX);
		double s = cd_rendering_interpol_curve_parameter (x);
		double y = yCurve (s);
		double x_ = (x0 + w0 - fDockOffsetX) / (fDockWidth + 2 * curveOffsetX);
		double s_ = cd_rendering_interpol_curve_parameter (x_);
		double y_ = yCurve (s_);
		int i, iNbMidPoints = MAX (0, w0 / 20 - 1);  // nombre de points intermediaires a calculer.
		double *pMidPointCoord = g_new (double, 2 * (iNbMidPoints+2));
		pMidPointCoord[0] = x0 - fDockOffsetX;
		pMidPointCoord[1] = y * h;
		pMidPointCoord[2*(iNbMidPoints+1)] = x0 + w0 - fDockOffsetX;
		pMidPointCoord[2*(iNbMidPoints+1)+1] = y_ * h;
		double si=s, ds = (s_ - s) / (iNbMidPoints+1);
		for (i = 1; i < iNbMidPoints+1; i ++)
		{
			si += ds;
			pMidPointCoord[2*i] = (fDockWidth + 2 * curveOffsetX) * xCurve (my_fCurveCurvature, si);
			pMidPointCoord[2*i+1] = h * yCurve (si);
		}
		
		cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_BUTT);
		cairo_save (pCairoContext);
		double fDeltaLineWidth = 0.;
		if (bIncludeEdges)
		{
			double tan_theta = MAX (fabs (pMidPointCoord[1] - pMidPointCoord[3]) / (pMidPointCoord[2] - pMidPointCoord[0]), fabs (pMidPointCoord[2*iNbMidPoints+1] - pMidPointCoord[2*iNbMidPoints+3]) / (pMidPointCoord[2*iNbMidPoints+2] - pMidPointCoord[2*iNbMidPoints]));
			fDeltaLineWidth = (fLineWidth / 2 + .1) * sqrt (1. + tan_theta*tan_theta);
		}
		if (pDock->bHorizontalDock)
		{
			cairo_move_to (pCairoContext, x0, fDockOffsetY - sens * (y * h + fDeltaLineWidth));
			for (i = 0; i < iNbMidPoints+1; i ++)
				cairo_rel_line_to (pCairoContext, pMidPointCoord[2*(i+1)] - pMidPointCoord[2*i], sens * (pMidPointCoord[2*i+1] - pMidPointCoord[2*i+3]));
			cairo_rel_line_to (pCairoContext, 0, sens * (y_ * h + fDeltaLineWidth));
			cairo_rel_line_to (pCairoContext, - w0, 0);
			cairo_rel_line_to (pCairoContext, 0, - sens * (y * h + fDeltaLineWidth));
		}
		else
		{
			cairo_move_to (pCairoContext, fDockOffsetY - sens * (y * h + fDeltaLineWidth), x0);
			for (i = 0; i < iNbMidPoints+1; i ++)
				cairo_rel_line_to (pCairoContext, sens * (pMidPointCoord[2*i+1] - pMidPointCoord[2*i+3]), pMidPointCoord[2*(i+1)] - pMidPointCoord[2*i]);
			cairo_rel_line_to (pCairoContext, sens * (y_ * h + fDeltaLineWidth), 0);
			cairo_rel_line_to (pCairoContext, 0, - w0);
			cairo_rel_line_to (pCairoContext, - sens * (y * h + fDeltaLineWidth), 0);
		}
		
		g_free (pMidPointCoord);
		cairo_clip (pCairoContext);
	}
	
	
	double fHeight, fBigWidth, fLittleWidth;
	if (bIncludeEdges)
	{
		fHeight = (bBackGround ? pDock->iDecorationsHeight - hi : hi) + myBackground.iDockLineWidth;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY : iVanishingPointY + fHeight);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY - fHeight : iVanishingPointY);
	}
	else
	{
		fHeight = pDock->iDecorationsHeight - myBackground.iDockLineWidth;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi - fHeight);
	}
	double fDeltaXLeft = fHeight * fLeftInclination;
	double fDeltaXRight = fHeight * fRightInclination;
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	
	int sens;
	double fDockOffsetX, fDockOffsetY;
	if (pDock->bDirectionUp)
	{
		sens = 1;
		if (bIncludeEdges)
			fDockOffsetY = pDock->iCurrentHeight - fHeight - (bBackGround ? myBackground.iDockLineWidth + hi : 0);
		else
			fDockOffsetY = pDock->iCurrentHeight - fHeight - myBackground.iDockLineWidth;
	}
	else
	{
		sens = -1;
		if (bIncludeEdges)
			fDockOffsetY = fHeight + (bBackGround ? myBackground.iDockLineWidth + hi : 0);
		else
			fDockOffsetY = fHeight + myBackground.iDockLineWidth;
	}
	
	if (bIncludeEdges)
		fDockOffsetX = icon->fDrawX - (bBackGround ? fHeight * fLeftInclination : 0);
	else
		fDockOffsetX = (pDock->bDirectionUp ? icon->fDrawX - (fHeight - hi) * fLeftInclination : icon->fDrawX - (fHeight - hi) * fLeftInclination);
	fDockOffsetX -= fDeltaInterIconLeft * fLeftInclination*sens;
	
	if (pDock->bHorizontalDock)
	{
		cairo_translate (pCairoContext, fDockOffsetX, fDockOffsetY);  // coin haut gauche.
		cairo_move_to (pCairoContext, 0, 0);  // coin haut gauche.
		
		cairo_rel_line_to (pCairoContext, fLittleWidth, 0);
		cairo_rel_line_to (pCairoContext, fDeltaXRight, sens * fHeight);
		cairo_rel_line_to (pCairoContext, - fBigWidth, 0);
		cairo_rel_line_to (pCairoContext, - fDeltaXLeft, - sens * fHeight);
		
		if (my_curve_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, 1, -1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL], MIN (0, (fHeight + hi) * fLeftInclination), 0);
		}
	}
	else
	{
		cairo_translate (pCairoContext, fDockOffsetY, fDockOffsetX);  // coin haut gauche.
		cairo_move_to (pCairoContext, 0, 0);  // coin haut gauche.
		
		cairo_rel_line_to (pCairoContext, 0, fLittleWidth);
		cairo_rel_line_to (pCairoContext, sens * fHeight, fDeltaXRight);
		cairo_rel_line_to (pCairoContext, 0, - fBigWidth);
		cairo_rel_line_to (pCairoContext, - sens * fHeight, - fDeltaXLeft);
		
		if (my_curve_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, -1, 1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL], 0, MIN (0, (fHeight + hi) * fLeftInclination));
		}
	}
}

static void cd_rendering_draw_3D_curve_separator_edge (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bBackGround)
{
	Icon *pPrevIcon = cairo_dock_get_previous_icon (pDock->icons, icon);
	if (pPrevIcon == NULL)
		pPrevIcon = icon;
	Icon *pNextIcon = cairo_dock_get_next_icon (pDock->icons, icon);
	if (pNextIcon == NULL)
		pNextIcon = icon;
	
	double hi, fVanishingDistanceLeft, fVanishingDistanceRight;
	if (pDock->bDirectionUp)
	{
		hi = pDock->iCurrentHeight - (icon->fDrawY + icon->fHeight * icon->fScale);
		fVanishingDistanceLeft = iVanishingPointY + pPrevIcon->fDrawY + pPrevIcon->fHeight * pPrevIcon->fScale;
		fVanishingDistanceRight = iVanishingPointY + pNextIcon->fDrawY + pNextIcon->fHeight * pNextIcon->fScale;
	}
	else
	{
		hi = icon->fDrawY;
		fVanishingDistanceLeft = iVanishingPointY + pDock->iCurrentHeight - pPrevIcon->fDrawY;
		fVanishingDistanceRight = iVanishingPointY + pDock->iCurrentHeight - pNextIcon->fDrawY;
	}
	double fLeftInclination = (icon->fDrawX - pDock->iCurrentWidth / 2) / fVanishingDistanceLeft;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->iCurrentWidth / 2) / fVanishingDistanceRight;
	
	
	double fHeight, fBigWidth, fLittleWidth;
	fHeight = (bBackGround ? pDock->iDecorationsHeight - hi - 0.5*myBackground.iDockLineWidth : hi + 1.5*myBackground.iDockLineWidth);
	fBigWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY : iVanishingPointY + fHeight);
	fLittleWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY - fHeight : iVanishingPointY);
	
	double fDeltaXLeft = fHeight * fLeftInclination;
	double fDeltaXRight = fHeight * fRightInclination;
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	
	int sens;
	double fDockOffsetX, fDockOffsetY;
	if (pDock->bDirectionUp)
	{
		sens = 1;
		fDockOffsetY =  (bBackGround ? 0.5*myBackground.iDockLineWidth : - 1.*myBackground.iDockLineWidth);
	}
	else
	{
		sens = -1;
		fDockOffsetY =  (bBackGround ? - 0.5*myBackground.iDockLineWidth : 1.*myBackground.iDockLineWidth);
	}
	fDockOffsetX = (bBackGround ? .5*myBackground.iDockLineWidth * fLeftInclination + 1.*fLeftInclination : - 0.5 * myBackground.iDockLineWidth * fLeftInclination);
	//fDockOffsetX = -.5*myBackground.iDockLineWidth;
	
	if (pDock->bHorizontalDock)
	{
		cairo_translate (pCairoContext, fDockOffsetX, fDockOffsetY);  // coin haut droit.
		
		cairo_move_to (pCairoContext, fLittleWidth, 0);
		cairo_rel_line_to (pCairoContext, fDeltaXRight, sens * fHeight);
		
		cairo_move_to (pCairoContext, 0, 0);
		cairo_rel_line_to (pCairoContext, fDeltaXLeft, sens * fHeight);
	}
	else
	{
		cairo_translate (pCairoContext, fDockOffsetY, fDockOffsetX);  // coin haut droit.
		
		cairo_move_to (pCairoContext, 0, fLittleWidth);
		cairo_rel_line_to (pCairoContext, sens * fHeight, fDeltaXRight);
		
		cairo_move_to (pCairoContext, 0, 0);
		cairo_rel_line_to (pCairoContext, sens * fHeight, fDeltaXLeft);
	}
}


static void cd_rendering_draw_3D_curve_separator (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bHorizontal, gboolean bBackGround)
{
	cd_rendering_make_3D_curve_separator (icon, pCairoContext, pDock, (my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR), bBackGround);
	
	if (my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
	{
		cairo_set_operator (pCairoContext, CAIRO_OPERATOR_DEST_OUT);
		cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 1.0);
		cairo_fill (pCairoContext);
		
		cd_rendering_draw_3D_curve_separator_edge (icon, pCairoContext, pDock, bBackGround);
		
		cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
		cairo_set_line_width (pCairoContext, myBackground.iDockLineWidth);
		cairo_set_source_rgba (pCairoContext, myBackground.fLineColor[0], myBackground.fLineColor[1], myBackground.fLineColor[2], myBackground.fLineColor[3]);
		cairo_stroke (pCairoContext);
	}
	else
	{
		cairo_fill (pCairoContext);
	}
}



void cd_rendering_render_curve (cairo_t *pCairoContext, CairoDock *pDock)
{
	//\____________________ On trace le cadre.
	double fLineWidth = myBackground.iDockLineWidth;
	double fMargin = myBackground.iFrameMargin;
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock) - 2 * myBackground.iFrameMargin;
	
	double h = 4./3 * (pDock->iDecorationsHeight + myBackground.iDockLineWidth);
	double hi = .5 * pDock->iMaxIconHeight + myBackground.iFrameMargin - 1;
	double ti = .5 * (1. - sqrt (MAX (1. - 4./3 * hi / h, 0)));
	double xi = xCurve (my_fCurveCurvature, ti);
	double curveOffsetX = fDockWidth * xi / (1 - 2 * xi);
	
	int sens;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX - curveOffsetX : fLineWidth / 2);
	if (pDock->bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = pDock->iCurrentHeight - .5 * fLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = .5 * fLineWidth;
	}
	
	cairo_save (pCairoContext);
	
	cairo_dock_draw_curved_frame (pCairoContext, fDockWidth + 2 * curveOffsetX, h, fDockOffsetX, fDockOffsetY, pDock->bHorizontalDock, sens);
	
	//\____________________ On dessine les decorations dedans.
	fDockOffsetY = (pDock->bDirectionUp ? pDock->iCurrentHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY, fDockOffsetX, fDockWidth + 2 * curveOffsetX);
	
	//\____________________ On dessine le cadre.
	if (fLineWidth > 0)
	{
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, myBackground.fLineColor[0], myBackground.fLineColor[1], myBackground.fLineColor[2], myBackground.fLineColor[3]);
		cairo_stroke (pCairoContext);
	}
	else
		cairo_new_path (pCairoContext);
	cairo_restore (pCairoContext);
	
	//\____________________ On dessine la ficelle qui les joint.
	if (myIcons.iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, myIcons.iStringLineWidth, FALSE, (my_curve_iDrawSeparator3D == CD_FLAT_SEPARATOR || my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR));
	
	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
	
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	
	if (my_curve_iDrawSeparator3D == CD_FLAT_SEPARATOR || my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
	{
		cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_BUTT);
		do
		{
			icon = ic->data;
			
			if (icon->acFileName == NULL && CAIRO_DOCK_IS_SEPARATOR (icon))
			{
				cairo_save (pCairoContext);
				cd_rendering_draw_3D_curve_separator (icon, pCairoContext, pDock, pDock->bHorizontalDock, TRUE);
				cairo_restore (pCairoContext);
			}
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
		
		do
		{
			icon = ic->data;
			
			if (icon->acFileName != NULL || ! CAIRO_DOCK_IS_SEPARATOR (icon))
			{
				cairo_save (pCairoContext);
				cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
				cairo_restore (pCairoContext);
			}
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
		
		if (my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
		{
			do
			{
				icon = ic->data;
				
				if (icon->acFileName == NULL && CAIRO_DOCK_IS_SEPARATOR (icon))
				{
					cairo_save (pCairoContext);
					cd_rendering_draw_3D_curve_separator (icon, pCairoContext, pDock, pDock->bHorizontalDock, FALSE);
					cairo_restore (pCairoContext);
				}
				
				ic = cairo_dock_get_next_element (ic, pDock->icons);
			} while (ic != pFirstDrawnElement);
		}
	}
	else
	{
		do
		{
			icon = ic->data;
			
			cairo_save (pCairoContext);
			cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
			cairo_restore (pCairoContext);
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
	}
}



static gboolean _cd_separator_is_impacted (Icon *icon, CairoDock *pDock, double fXMin, double fXMax, gboolean bBackGround, gboolean bIncludeEdges)
{
	double hi = .5 * pDock->iMaxIconHeight + myBackground.iFrameMargin - 1;
	double fLeftInclination = fabs (icon->fDrawX - pDock->iCurrentWidth / 2) / iVanishingPointY;
	double fRightInclination = fabs (icon->fDrawX + icon->fWidth * icon->fScale - pDock->iCurrentWidth / 2) / iVanishingPointY;
	
	double fHeight, fBigWidth, fLittleWidth;
	if (bIncludeEdges)
	{
		fHeight = (bBackGround ? pDock->iDecorationsHeight - hi : hi) + (bIncludeEdges ? myBackground.iDockLineWidth : 0);
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY : iVanishingPointY + fHeight);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY - fHeight : iVanishingPointY);
	}
	else
	{
		fHeight = pDock->iDecorationsHeight;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + fHeight);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY - fHeight);
	}
	double fDeltaXLeft = fHeight * fLeftInclination;
	double fDeltaXRight = fHeight * fRightInclination;
	double fDeltaX = MAX (fDeltaXLeft, fDeltaXRight);
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	
	int sens;
	double fDockOffsetX, fDockOffsetY;
	if (pDock->bDirectionUp)
	{
		sens = 1;
		if (bIncludeEdges)
			fDockOffsetY =  pDock->iCurrentHeight - fHeight - (bBackGround ? myBackground.iDockLineWidth + hi : 0);
		else
			fDockOffsetY =  pDock->iCurrentHeight - fHeight;
	}
	else
	{
		sens = -1;
		if (bIncludeEdges)
			fDockOffsetY = fHeight + (bBackGround ? myBackground.iDockLineWidth + hi : 0);
		else
			fDockOffsetY = fHeight;
	}
	
	if (bIncludeEdges)
			fDockOffsetX = icon->fDrawX - (bBackGround ? fHeight * fLeftInclination : 0);
		else
			fDockOffsetX = icon->fDrawX - (fHeight - hi) * fLeftInclination;
	double fXLeft, fXRight;
	if (icon->fDrawX + icon->fWidth * icon->fScale / 2 > pDock->iCurrentWidth / 2)  // on est a droite.
	{
		if (bIncludeEdges)
		{
			if (bBackGround)
			{
				fXLeft = icon->fDrawX - fHeight * fLeftInclination;
				fXRight = icon->fDrawX + icon->fWidth * icon->fScale;
			}
			else
			{
				fXLeft = icon->fDrawX;
				fXRight = icon->fDrawX + icon->fWidth * icon->fScale + fHeight * fRightInclination;
			}
		}
		else
		{
			fXLeft = icon->fDrawX - (fHeight - hi) * fLeftInclination;
			fXRight = icon->fDrawX + icon->fWidth * icon->fScale + hi * fRightInclination;
		}
	}
	else  // a gauche.
	{
		if (bIncludeEdges)
		{
			if (bBackGround)
			{
				fXLeft = icon->fDrawX;
				fXRight = icon->fDrawX + icon->fWidth * icon->fScale + fHeight * fRightInclination;
			}
			else
			{
				fXLeft = icon->fDrawX - fHeight * fLeftInclination;
				fXRight = icon->fDrawX + icon->fWidth * icon->fScale;
			}
		}
		else
		{
			fXLeft = icon->fDrawX - hi * fLeftInclination;
			fXRight = icon->fDrawX + icon->fWidth * icon->fScale +(fHeight - hi) * fRightInclination;
		}
	}
	
	return (fXLeft <= fXMax && floor (fXRight) > fXMin);
}

void cd_rendering_render_optimized_curve (cairo_t *pCairoContext, CairoDock *pDock, GdkRectangle *pArea)
{
	//\____________________ On trace le cadre.
	double fLineWidth = myBackground.iDockLineWidth;
	double fMargin = myBackground.iFrameMargin;
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock) - 2 * myBackground.iFrameMargin;
	
	double h = 4./3 * (pDock->iDecorationsHeight + myBackground.iDockLineWidth);
	double hi = .5 * pDock->iMaxIconHeight + myBackground.iFrameMargin - 1;
	double ti = .5 * (1. - sqrt (MAX (1. - 4./3 * hi / h, 0)));
	double xi = xCurve (my_fCurveCurvature, ti);
	double curveOffsetX = fDockWidth * xi / (1 - 2 * xi);
	
	int sens;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX - curveOffsetX : fLineWidth / 2);
	int x0, y0, w0, h0;
	if (pDock->bHorizontalDock)
	{
		x0 = pArea->x;
		y0 = pArea->y;
		w0 = pArea->width;
		h0 = pArea->height;
	}
	else
	{
		x0 = pArea->y;
		y0 = pArea->x;
		w0 = pArea->height;
		h0 = pArea->width;
	}
	
	if (pDock->bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = pDock->iCurrentHeight - .5 * fLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = .5 * fLineWidth;
	}
	
	//\________________ On approche le morceau de courbe de Bezier par des trapezes.
	double x = (x0 - fDockOffsetX) / (fDockWidth + 2 * curveOffsetX);
	double s = cd_rendering_interpol_curve_parameter (x);
	double y = yCurve (s);
	double x_ = (x0 + w0 - fDockOffsetX) / (fDockWidth + 2 * curveOffsetX);
	double s_ = cd_rendering_interpol_curve_parameter (x_);
	double y_ = yCurve (s_);
	int i, iNbMidPoints = MAX (0, w0 / 20 - 1);  // nombre de points intermediaires a calculer.
	double *pMidPointCoord = g_new (double, 2 * (iNbMidPoints+2));
	pMidPointCoord[0] = x0 - fDockOffsetX;
	pMidPointCoord[1] = y * h;
	pMidPointCoord[2*(iNbMidPoints+1)] = x0 + w0 - fDockOffsetX;
	pMidPointCoord[2*(iNbMidPoints+1)+1] = y_ * h;
	double si=s, ds = (s_ - s) / (iNbMidPoints+1);
	for (i = 1; i < iNbMidPoints+1; i ++)
	{
		si += ds;
		pMidPointCoord[2*i] = (fDockWidth + 2 * curveOffsetX) * xCurve (my_fCurveCurvature, si);
		pMidPointCoord[2*i+1] = h * yCurve (si);
	}
	
	cairo_save (pCairoContext);
	if (pDock->bHorizontalDock)
	{
		cairo_move_to (pCairoContext, pArea->x, fDockOffsetY - sens * y * h);
		for (i = 0; i < iNbMidPoints+1; i ++)
			cairo_rel_line_to (pCairoContext, pMidPointCoord[2*(i+1)] - pMidPointCoord[2*i], sens * (pMidPointCoord[2*i+1] - pMidPointCoord[2*i+3]));
		cairo_rel_line_to (pCairoContext, 0, sens * y_ * h);
		cairo_rel_line_to (pCairoContext, - pArea->width, 0);
		cairo_rel_line_to (pCairoContext, 0, - sens * y * h);
	}
	else
	{
		cairo_move_to (pCairoContext, fDockOffsetY - sens * y * h, pArea->y);
		for (i = 0; i < iNbMidPoints+1; i ++)
			cairo_rel_line_to (pCairoContext, sens * (pMidPointCoord[2*i+1] - pMidPointCoord[2*i+3]), pMidPointCoord[2*(i+1)] - pMidPointCoord[2*i]);
		cairo_rel_line_to (pCairoContext, sens * y_ * h, 0);
		cairo_rel_line_to (pCairoContext, 0, - pArea->height);
		cairo_rel_line_to (pCairoContext, - sens * y * h, 0);
	}
	//\____________________ On dessine les decorations dedans.
	fDockOffsetY = (pDock->bDirectionUp ? pDock->iCurrentHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY, fDockOffsetX, fDockWidth + 2 * curveOffsetX);
	
	//\____________________ On dessine le cadre.
	cairo_new_path (pCairoContext);
	if (fLineWidth > 0)
	{
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, myBackground.fLineColor[0], myBackground.fLineColor[1], myBackground.fLineColor[2], myBackground.fLineColor[3]);
		
		fDockOffsetY = (pDock->bDirectionUp ? pDock->iCurrentHeight - .5 * fLineWidth : .5 * fLineWidth);
		
		if (pDock->bHorizontalDock)
		{
			cairo_move_to (pCairoContext, pArea->x, fDockOffsetY - sens * y * h);
			for (i = 0; i < iNbMidPoints+1; i ++)
				cairo_rel_line_to (pCairoContext, pMidPointCoord[2*(i+1)] - pMidPointCoord[2*i], sens * (pMidPointCoord[2*i+1] - pMidPointCoord[2*i+3]));
			cairo_stroke (pCairoContext);
			
			cairo_new_path (pCairoContext);
			cairo_move_to (pCairoContext, pArea->x, fDockOffsetY);
			cairo_rel_line_to (pCairoContext, pArea->width, 0);
		}
		else
		{
			cairo_move_to (pCairoContext, fDockOffsetY - sens * y * h, pArea->y);
			for (i = 0; i < iNbMidPoints+1; i ++)
				cairo_rel_line_to (pCairoContext, sens * (pMidPointCoord[2*i+1] - pMidPointCoord[2*i+3]), pMidPointCoord[2*(i+1)] - pMidPointCoord[2*i]);
			cairo_stroke (pCairoContext);
			
			cairo_new_path (pCairoContext);
			cairo_move_to (pCairoContext, fDockOffsetY, pArea->y);
			cairo_rel_line_to (pCairoContext, 0, pArea->height);
		}
		cairo_stroke (pCairoContext);
	}
	g_free (pMidPointCoord);
	
	cairo_restore (pCairoContext);
	
	/*double x = (fDockOffsetX - pFirstIcon->fX + curveOffsetX) / (fDockWidth + 2 * curveOffsetX);
	double a = - 1. / 3;
	double b = (1 - my_fCurveCurvature) / 3 / (1 - 3 * my_fCurveCurvature);
	double c = - x / (1 - 3 * my_fCurveCurvature);
	double a_ = a*a - b;
	double b_ = a*b - c;
	double c_ = b*b - a*c;
	double u = (-b_ + sqrt (b_*b_ - 4 * a_ * c_)) / (2 * a_);
	double v = (-b_ - sqrt (b_*b_ - 4 * a_ * c_)) / (2 * a_);
	double lambda = - (a + u) / (a + v);
	double t = v + (u - v) / (1 + pow (lambda, 1./3));
	//g_print ("x : %.2f => t : %.2f\n", x, t);*/
	
	//\____________________ On dessine les icones impactees.
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement != NULL)
	{
		double fXMin = (pDock->bHorizontalDock ? pArea->x : pArea->y), fXMax = (pDock->bHorizontalDock ? pArea->x + pArea->width : pArea->y + pArea->height);
		double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
		double fXLeft, fXRight;
		Icon *icon;
		GList *ic = pFirstDrawnElement;
		
		if (my_curve_iDrawSeparator3D == CD_FLAT_SEPARATOR || my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
		{
			cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_BUTT);
			do
			{
				icon = ic->data;
				
				if (CAIRO_DOCK_IS_SEPARATOR (icon) && icon->acFileName == NULL)
				{
					if (_cd_separator_is_impacted (icon, pDock, fXMin, fXMax, TRUE, (my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)))
					{
						cairo_save (pCairoContext);
						cd_rendering_draw_3D_curve_separator (icon, pCairoContext, pDock, pDock->bHorizontalDock, TRUE);
						cairo_restore (pCairoContext);
					}
				}
				
				ic = cairo_dock_get_next_element (ic, pDock->icons);
			} while (ic != pFirstDrawnElement);
			
			do
			{
				icon = ic->data;
				if (! CAIRO_DOCK_IS_SEPARATOR (icon) || icon->acFileName != NULL)
				{
					fXLeft = icon->fDrawX + icon->fScale + 1;
					fXRight = icon->fDrawX + (icon->fWidth - 1) * icon->fScale * icon->fWidthFactor - 1;
					
					if (fXLeft <= fXMax && floor (fXRight) > fXMin)
					{
						if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= pDock->iCurrentWidth)
							icon->fAlpha = 1;
						else
							icon->fAlpha = .25;
						
						cairo_save (pCairoContext);
						
						cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
						
						cairo_restore (pCairoContext);
					}
				}
				ic = cairo_dock_get_next_element (ic, pDock->icons);
			} while (ic != pFirstDrawnElement);
			
			if (my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
			{
				do
				{
					icon = ic->data;
					
					if (CAIRO_DOCK_IS_SEPARATOR (icon) && icon->acFileName == NULL)
					{
						if (_cd_separator_is_impacted (icon, pDock, fXMin, fXMax, FALSE, (my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)))
						{
							cairo_save (pCairoContext);
							cd_rendering_draw_3D_curve_separator (icon, pCairoContext, pDock, pDock->bHorizontalDock, FALSE);
							cairo_restore (pCairoContext);
						}
					}
					
					ic = cairo_dock_get_next_element (ic, pDock->icons);
				} while (ic != pFirstDrawnElement);
			}
		}
		else
		{
			do
			{
				icon = ic->data;
				fXLeft = icon->fDrawX + icon->fScale + 1;
				fXRight = icon->fDrawX + (icon->fWidth - 1) * icon->fScale * icon->fWidthFactor - 1;
				
				if (fXLeft <= fXMax && floor (fXRight) > fXMin)
				{
					if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= pDock->iCurrentWidth)
						icon->fAlpha = 1;
					else
						icon->fAlpha = .25;
					
					cairo_save (pCairoContext);
					
					cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
					
					cairo_restore (pCairoContext);
				}
				ic = cairo_dock_get_next_element (ic, pDock->icons);
			} while (ic != pFirstDrawnElement);
		}
	}
}


Icon *cd_rendering_calculate_icons_curve (CairoDock *pDock)
{
	Icon *pPointedIcon = cairo_dock_apply_wave_effect (pDock);
	
	cairo_dock_check_if_mouse_inside_linear (pDock);
	
	//\____________________ On calcule les position/etirements/alpha des icones.
	double h = 4./3 * (pDock->iDecorationsHeight + myBackground.iDockLineWidth);
	double hi = .5 * pDock->iMaxIconHeight + myBackground.iFrameMargin - 1;
	double ti = .5 * (1. - sqrt (MAX (1. - 4./3 * hi / h, 0)));
	double xi = xCurve (my_fCurveCurvature, ti);
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock) - 2 * myBackground.iFrameMargin;
	double curveOffsetX = fDockWidth * xi;  // pDock->iMaxDockWidth
	
	gint sens, fDockOffsetY;
	sens = pDock->bDirectionUp ? 1 : -1;
	double fReflectionOffsetY = (pDock->bDirectionUp ? -1 : 1) * myIcons.fReflectSize;
	// On va calculer une parabole pour approcher la courbe de bézier : 
	// Soient A: (xa,ya) B: (xb,yb) C: (xc,yc) trois points qui appartiennent à la parabole. xa, xb et xc sont distincts.
	// P1(x)=(x-xb)(x-xc)
	// P2(x)=(x-xa)(x-xc)
	// P3(x)=(x-xa)(x-xb)
	// p(x)=k1p1(x)+k2p2(x)+k3p3(x).
	// avec k1=ya/((xa-xb)(xa-xc)) car p(xa)=ya p2(xa)=p3(xa)=0
	
	// on calcule donc k1, k2, k3 : 
	gdouble xa, xb, xc, ya, yb, yc, k1, k2, k3;
	if(pDock->icons  == NULL)
		return NULL;
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	Icon *pLastIcon = cairo_dock_get_last_drawn_icon (pDock);
	xa = 0 ; // icone de gauche
	ya = 0;
	xc = pLastIcon->fX - pFirstIcon->fX;  // icone de droite
	yc = 0;
	xb = (xc-xa)/2;
	yb = -my_iCurveAmplitude;
	
	if (xa != xb && xa != xc && xb != xc)
	{
		k1 = ya/((xa-xb)*(xa-xc));
		k2 = yb/((xb-xa)*(xb-xc));
		k3 = yc/((xc-xa)*(xc-xb));
	}
	else
	{
		k1 = k2 = k3 = 0;
	}
	
	
	Icon* icon;
	GList* ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		double x = icon->fX - pFirstIcon->fX;
		double y = k1*(x-xb)*(x-xc) + k2*(x-xa)*(x-xc) + k3*(x-xa)*(x-xb);
		
		icon->fDrawX = icon->fX;
		icon->fDrawY = icon->fY + sens * y;
		//g_print ("y : %.2f -> fDrawY = %.2f\n", y, icon->fDrawY);
		icon->fWidthFactor = 1.;
		icon->fHeightFactor = 1.;
		///icon->fDeltaYReflection = 0.;
		icon->fOrientation = 0.;
		if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= pDock->iCurrentWidth)
			icon->fAlpha = 1;
		else
			icon->fAlpha = .25;
		//g_print ("fDrawX:%.2f / %d (%.2f)\n", icon->fDrawX, pDock->iCurrentWidth, icon->fAlpha);
	}
	
	cairo_dock_check_can_drop_linear (pDock);
	
	return pPointedIcon;
}

void cd_rendering_register_curve_renderer (const gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	pRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-curve-view", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->cPreviewFilePath = g_strdup_printf ("%s/preview-curve.png", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->calculate_max_dock_size = cd_rendering_calculate_max_dock_size_curve;
	pRenderer->calculate_icons = cd_rendering_calculate_icons_curve;
	pRenderer->render = cd_rendering_render_curve;
	pRenderer->render_optimized = cd_rendering_render_optimized_curve;
	pRenderer->render_opengl = cd_rendering_render_curve_opengl;
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;
	pRenderer->bUseReflect = TRUE;
	pRenderer->bUseStencil = TRUE;
	
	cairo_dock_register_renderer (cRendererName, pRenderer);
}


static void cairo_dock_draw_curved_frame_horizontal (cairo_t *pCairoContext, double fFrameWidth, double fControlHeight, double fDockOffsetX, double fDockOffsetY, int sens)
{
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	cairo_rel_curve_to (pCairoContext,
		(1 - my_fCurveCurvature) * fFrameWidth / 2, -sens * fControlHeight,
		(1 + my_fCurveCurvature) * fFrameWidth / 2, -sens * fControlHeight,
		fFrameWidth, 0);
	
	// on trace la ligne du bas
	cairo_rel_line_to (pCairoContext, -fFrameWidth, 0);
}
static void cairo_dock_draw_curved_frame_vertical (cairo_t *pCairoContext, double fFrameWidth, double fControlHeight, double fDockOffsetX, double fDockOffsetY, int sens)
{
       cairo_move_to (pCairoContext, fDockOffsetY, fDockOffsetX);
	cairo_rel_curve_to (pCairoContext,
		-sens * fControlHeight, (1 - my_fCurveCurvature) * fFrameWidth / 2,
		-sens * fControlHeight, (1 + my_fCurveCurvature) * fFrameWidth / 2,
		0, fFrameWidth);
	
	// on trace la ligne du bas
	cairo_rel_line_to (pCairoContext, 0, - fFrameWidth);
}
void cairo_dock_draw_curved_frame (cairo_t *pCairoContext, double fFrameWidth, double fControlHeight, double fDockOffsetX, double fDockOffsetY, gboolean bHorizontal, int sens)
{
	if (bHorizontal)
		cairo_dock_draw_curved_frame_horizontal (pCairoContext, fFrameWidth, fControlHeight, fDockOffsetX, fDockOffsetY, sens);
	else
		cairo_dock_draw_curved_frame_vertical (pCairoContext, fFrameWidth, fControlHeight, fDockOffsetX, fDockOffsetY, sens);
}


void cd_rendering_calculate_reference_curve (double alpha)
{
	if (s_pReferenceCurveS == NULL)
	{
		s_pReferenceCurveS = g_new (double, RENDERING_INTERPOLATION_NB_PTS+1);
	}
	
	if (s_pReferenceCurveX == NULL)
	{
		s_pReferenceCurveX = g_new (double, RENDERING_INTERPOLATION_NB_PTS+1);
	}
	
	if (s_pReferenceCurveY == NULL)
	{
		s_pReferenceCurveY = g_new (double, RENDERING_INTERPOLATION_NB_PTS+1);
	}
	
	double s, x, y;
	int i;
	for (i = 0; i < RENDERING_INTERPOLATION_NB_PTS+1; i ++)
	{
		s = (double) i / RENDERING_INTERPOLATION_NB_PTS;
		
		s_pReferenceCurveS[i] = s;
		s_pReferenceCurveX[i] = xCurve (my_fCurveCurvature, s);
		s_pReferenceCurveY[i] = yCurve (s);
	}
}

double cd_rendering_interpol_curve_parameter (double x)
{
	return cd_rendering_interpol (x, s_pReferenceCurveX, s_pReferenceCurveS);
}

double cd_rendering_interpol_curve_height (double x)
{
	return cd_rendering_interpol (x, s_pReferenceCurveX, s_pReferenceCurveY);
}






void cd_rendering_render_curve_opengl (CairoDock *pDock)
{
	//\____________________ On genere le cadre.
	double fLineWidth = myBackground.iDockLineWidth;
	double fMargin = myBackground.iFrameMargin;
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock) - 2 * myBackground.iFrameMargin;
	
	double h = 4./3 * (pDock->iDecorationsHeight + myBackground.iDockLineWidth);
	double hi = .5 * pDock->iMaxIconHeight + myBackground.iFrameMargin - 1;
	double ti = .5 * (1. - sqrt (MAX (1. - 4./3 * hi / h, 0)));
	double xi = xCurve (my_fCurveCurvature, ti);
	double curveOffsetX = fDockWidth * xi / (1 - 2 * xi);
	
	double fFrameHeight = pDock->iDecorationsHeight + fLineWidth;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX - curveOffsetX : fLineWidth / 2);
	if ((pDock->bHorizontalDock && ! pDock->bDirectionUp) || (! pDock->bHorizontalDock && pDock->bDirectionUp))
		fDockOffsetY = pDock->iCurrentHeight - .5 * fLineWidth;
	else
		fDockOffsetY = pDock->iDecorationsHeight + 1.5 * fLineWidth;
	if (! pDock->bHorizontalDock)
		fDockOffsetX = pDock->iCurrentWidth - fDockOffsetX + 0;
	else
		fDockOffsetX = fDockOffsetX-0;
	
	int iNbVertex;
	GLfloat *pVertexTab = cairo_dock_generate_curve_path (4./3, &iNbVertex);
	
	//\________________ On met en place le clipping.
	glDisable (GL_DEPTH_TEST);
	glEnable (GL_STENCIL_TEST);  // active le tampon 'stencil'.
	glClear (GL_STENCIL_BUFFER_BIT);  // on le remplit de 0.
	glStencilFunc (GL_ALWAYS, 1, 1);  // valeur de reference = 1, et on ecrira un 1 sans condition.
	glStencilOp (GL_KEEP, GL_KEEP, GL_REPLACE);  // on remplace tout ce qui est dedans.
	glColorMask (FALSE, FALSE, FALSE, FALSE);  // desactive l'ecriture dans toutes les composantes du Tampon Chromatique.
	
	double fEpsilon = (my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR ? 2. : 0);  // erreur d'arrondi quand tu nous tiens.
	glPushMatrix ();
	cairo_dock_draw_frame_background_opengl (0, fDockWidth+2*curveOffsetX, fFrameHeight+fLineWidth+fEpsilon, fDockOffsetX, fDockOffsetY+(fLineWidth+fEpsilon)/2, pVertexTab, iNbVertex, pDock->bHorizontalDock, pDock->bDirectionUp, pDock->fDecorationsOffsetX);  // le cadre est trace au milieu de la ligne, donc on augmente de l la hauteur (et donc de l/2 pixels la hauteur du cadre, car [-.5, .5]), et pour compenser on se translate de l/2.
	glPopMatrix ();
	
	glColorMask (TRUE, TRUE, TRUE, TRUE);
	glStencilFunc (GL_EQUAL, 1, 1); /* draw if ==1 */
	glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);  // read only.
	glDisable (GL_STENCIL_TEST);
	
	//\____________________ On dessine les decorations dedans.
	glPushMatrix ();
	cairo_dock_draw_frame_background_opengl (g_iBackgroundTexture, fDockWidth+2*curveOffsetX, fFrameHeight, fDockOffsetX, fDockOffsetY, pVertexTab, iNbVertex, pDock->bHorizontalDock, pDock->bDirectionUp, pDock->fDecorationsOffsetX);
	
	//\____________________ On dessine le cadre.
	if (fLineWidth > 0)
		cairo_dock_draw_current_path_opengl (fLineWidth, myBackground.fLineColor, iNbVertex);
	
	glPopMatrix ();
	
	//\____________________ On dessine la ficelle qui les joint.
	if (myIcons.iStringLineWidth > 0)
		cairo_dock_draw_string_opengl (pDock, myIcons.iStringLineWidth, FALSE, (my_curve_iDrawSeparator3D == CD_FLAT_SEPARATOR || my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR));
	
	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
	
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	
	if (my_curve_iDrawSeparator3D == CD_FLAT_SEPARATOR || my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
	{
		do
		{
			icon = ic->data;
			
			if (icon->acFileName == NULL && CAIRO_DOCK_IS_SEPARATOR (icon))
			{
				glEnable (GL_STENCIL_TEST);
				glStencilFunc (GL_EQUAL, 1, 1);
				glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
				
				glPushMatrix ();
				if (my_curve_iDrawSeparator3D == CD_FLAT_SEPARATOR)
					cd_rendering_draw_flat_separator_opengl (icon, pDock);
				else
					cd_rendering_draw_physical_separator_opengl (icon, pDock, TRUE, (ic->prev ? ic->prev->data : NULL), (ic->next ? ic->next : NULL));
				glPopMatrix ();
				
				glDisable (GL_STENCIL_TEST);
			}
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
		
		do
		{
			icon = ic->data;
			
			if (icon->acFileName != NULL || ! CAIRO_DOCK_IS_SEPARATOR (icon))
			{
				glPushMatrix ();
				cairo_dock_render_one_icon_opengl (icon, pDock, fDockMagnitude, TRUE);
				glPopMatrix ();
			}
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
		
		if (my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
		{
			do
			{
				icon = ic->data;
				
				if (icon->acFileName == NULL && CAIRO_DOCK_IS_SEPARATOR (icon))
				{
					glEnable (GL_STENCIL_TEST);
					glStencilFunc (GL_EQUAL, 1, 1);
					glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
					
					glPushMatrix ();
					cd_rendering_draw_physical_separator_opengl (icon, pDock, FALSE, (ic->prev ? ic->prev->data : NULL), (ic->next ? ic->next : NULL));
					glPopMatrix ();
					
					glDisable (GL_STENCIL_TEST);
				}
				
				ic = cairo_dock_get_next_element (ic, pDock->icons);
			} while (ic != pFirstDrawnElement);
		}
	}
	else
	{
		do
		{
			icon = ic->data;
			
			glPushMatrix ();
			cairo_dock_render_one_icon_opengl (icon, pDock, fDockMagnitude, TRUE);
			glPopMatrix ();
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
	}
}


#define DELTA_ROUND_DEGREE 1
#define RADIAN (G_PI / 180.0)  // Conversion Radian/Degres
#define P(t,p,q,r,s) (1-t) * (1-t) * (1-t) * p + 3 * t * (1-t) * (1 - t) * q + 3 * t * t * (1-t) * r + t * t * t * s
GLfloat *cairo_dock_generate_curve_path (double fRelativeControlHeight, int *iNbPoints)
{
	//static GLfloat pVertexTab[((180/DELTA_ROUND_DEGREE+1)+1)*3];
	_cairo_dock_define_static_vertex_tab ((180/DELTA_ROUND_DEGREE+1)+1);
	
	double w = 1. / 2;
	double h = 1. / 2;
	double xp = -w, xq = - (1 - my_fCurveCurvature) * w, xr = - xq, xs = - xp;
	double yp = 0., yq = fRelativeControlHeight, yr = yq, ys = yp;
	
	//g_print ("%s (%.2f)\n", __func__, yq);
	int iPrecision = DELTA_ROUND_DEGREE;
	int i=0, t;
	for (t = 0; t <= 180; t += iPrecision, i++)
	{
		_cairo_dock_set_vertex_xy (i,
			P (t/180., xp, xq, xr, xs),
			P (t/180., yp, yq, yr, ys) - h);
		//vx(i) = P (t/180., xp, xq, xr, xs);
		//vy(i) = P (t/180., yp, yq, yr, ys) - h;
	}
	
	// on trace la ligne du bas.
	_cairo_dock_close_path (i);  // on boucle.
	//vx(i) = vx(0);  // on boucle.
	//vy(i) = vy(0);
	
	*iNbPoints = i+1;
	_cairo_dock_return_vertex_tab ();
}








/*static void cd_rendering_draw_curve_flat_separator_opengl (Icon *icon, CairoDock *pDock)
{
	double fLineWidth = myBackground.iDockLineWidth;
	double fMargin = myBackground.iFrameMargin;
	double hi;
	
	Icon *pPrevIcon = cairo_dock_get_previous_icon (pDock->icons, icon);
	if (pPrevIcon == NULL)
		pPrevIcon = icon;
	Icon *pNextIcon = cairo_dock_get_next_icon (pDock->icons, icon);
	if (pNextIcon == NULL)
		pNextIcon = icon;
	
	double fVanishingDistanceLeft, fVanishingDistanceRight;
	double fDeltaInterIconLeft, fDeltaInterIconRight;
	if (pDock->bDirectionUp)
	{
		hi = pDock->iCurrentHeight - (icon->fDrawY + icon->fHeight * icon->fScale);
		fVanishingDistanceLeft = iVanishingPointY + pPrevIcon->fDrawY + pPrevIcon->fHeight * pPrevIcon->fScale;
		fVanishingDistanceRight = iVanishingPointY + pNextIcon->fDrawY + pNextIcon->fHeight * pNextIcon->fScale;
		
		fDeltaInterIconLeft = (pPrevIcon->fDrawY + pPrevIcon->fHeight * pPrevIcon->fScale) - (icon->fDrawY + icon->fHeight * icon->fScale);
		fDeltaInterIconRight = (icon->fDrawY + icon->fHeight * icon->fScale) - (pNextIcon->fDrawY + pNextIcon->fHeight * pNextIcon->fScale);
	}
	else
	{
		hi = icon->fDrawY;
		fVanishingDistanceLeft = iVanishingPointY + pDock->iCurrentHeight - pPrevIcon->fDrawY;
		fVanishingDistanceRight = iVanishingPointY + pDock->iCurrentHeight - pNextIcon->fDrawY;
		
		fDeltaInterIconLeft = (pPrevIcon->fDrawY) - (icon->fDrawY);
		fDeltaInterIconRight = (icon->fDrawY) - (pNextIcon->fDrawY);
	}
	double fLeftInclination = (icon->fDrawX - pDock->iCurrentWidth / 2) / fVanishingDistanceLeft;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->iCurrentWidth / 2) / fVanishingDistanceRight;
	
	if (bBackGround || ! bIncludeEdges)  // pour s'arreter sur la courbe, on realise un clippage.
	{
		//\________________ On se ramene au cas du dessin optimise.
		double x0, y0, xf, yf, w0, h0;
		if (pDock->bDirectionUp)
		{
			x0 = icon->fDrawX - MAX (0, fLeftInclination * (pPrevIcon->fDrawY + pPrevIcon->fHeight * pPrevIcon->fScale));
			xf = icon->fDrawX + icon->fWidth * icon->fScale - MIN (0, fRightInclination * (pNextIcon->fDrawY + pNextIcon->fHeight * pNextIcon->fScale));
		}
		else
		{
			x0 = icon->fDrawX - MAX (0, fLeftInclination * (pDock->iCurrentHeight - (pPrevIcon->fDrawY)));
			xf = icon->fDrawX + icon->fWidth * icon->fScale - MIN (0, fRightInclination * (pDock->iCurrentHeight - (pNextIcon->fDrawY)));
		}
		if (! bIncludeEdges)  // on prolonge jusqu'en bas.
		{
			if (pDock->bDirectionUp)
			{
				x0 += MIN (0, fLeftInclination * (pDock->iCurrentHeight - icon->fDrawY - icon->fHeight * icon->fScale));
				xf += MAX (0, fRightInclination * (pDock->iCurrentHeight - icon->fDrawY - icon->fHeight * icon->fScale));
			}
			else
			{
				x0 += MIN (0, fLeftInclination * (pPrevIcon->fDrawY));
				xf += MAX (0, fRightInclination * (pNextIcon->fDrawY));
			}
		}
		//g_print ("x0:%.2f -> xf:%.2f\n", x0, xf);
		y0 = 0;
		yf = icon->fDrawY;
		w0 = xf - x0;
		h0 = yf - y0;
		
		int sens;
		double fDockOffsetY;  // Offset du coin haut gauche du cadre.
		if (pDock->bDirectionUp)
		{
			sens = 1;
			fDockOffsetY = pDock->iCurrentHeight - .5 * fLineWidth;
		}
		else
		{
			sens = -1;
			fDockOffsetY = .5 * fLineWidth;
		}
		
		double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock) - 2 * myBackground.iFrameMargin;
		
		double h = 4./3 * (pDock->iDecorationsHeight + myBackground.iDockLineWidth);
		double hi_ = .5 * pDock->iMaxIconHeight + myBackground.iFrameMargin - 1;
		double ti = .5 * (1. - sqrt (MAX (1. - 4./3 * hi_ / h, 0)));
		double xi = xCurve (my_fCurveCurvature, ti);
		double curveOffsetX = fDockWidth * xi / (1 - 2 * xi);
		
		Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
		double fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX - curveOffsetX : fLineWidth / 2);
		
		
		//\________________ On approche le morceau de courbe de Bezier par des trapezes.
		double x = (x0 - fDockOffsetX) / (fDockWidth + 2 * curveOffsetX);
		double s = cd_rendering_interpol_curve_parameter (x);
		double y = yCurve (s);
		double x_ = (x0 + w0 - fDockOffsetX) / (fDockWidth + 2 * curveOffsetX);
		double s_ = cd_rendering_interpol_curve_parameter (x_);
		double y_ = yCurve (s_);
		int i, iNbMidPoints = MAX (0, w0 / 20 - 1);  // nombre de points intermediaires a calculer.
		double *pMidPointCoord = g_new (double, 2 * (iNbMidPoints+2));
		pMidPointCoord[0] = x0 - fDockOffsetX;
		pMidPointCoord[1] = y * h;
		pMidPointCoord[2*(iNbMidPoints+1)] = x0 + w0 - fDockOffsetX;
		pMidPointCoord[2*(iNbMidPoints+1)+1] = y_ * h;
		double si=s, ds = (s_ - s) / (iNbMidPoints+1);
		for (i = 1; i < iNbMidPoints+1; i ++)
		{
			si += ds;
			pMidPointCoord[2*i] = (fDockWidth + 2 * curveOffsetX) * xCurve (my_fCurveCurvature, si);
			pMidPointCoord[2*i+1] = h * yCurve (si);
		}
		
		cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_BUTT);
		cairo_save (pCairoContext);
		double fDeltaLineWidth = 0.;
		if (bIncludeEdges)
		{
			double tan_theta = MAX (fabs (pMidPointCoord[1] - pMidPointCoord[3]) / (pMidPointCoord[2] - pMidPointCoord[0]), fabs (pMidPointCoord[2*iNbMidPoints+1] - pMidPointCoord[2*iNbMidPoints+3]) / (pMidPointCoord[2*iNbMidPoints+2] - pMidPointCoord[2*iNbMidPoints]));
			fDeltaLineWidth = (fLineWidth / 2 + .1) * sqrt (1. + tan_theta*tan_theta);
		}
		if (pDock->bHorizontalDock)
		{
			cairo_move_to (pCairoContext, x0, fDockOffsetY - sens * (y * h + fDeltaLineWidth));
			for (i = 0; i < iNbMidPoints+1; i ++)
				cairo_rel_line_to (pCairoContext, pMidPointCoord[2*(i+1)] - pMidPointCoord[2*i], sens * (pMidPointCoord[2*i+1] - pMidPointCoord[2*i+3]));
			cairo_rel_line_to (pCairoContext, 0, sens * (y_ * h + fDeltaLineWidth));
			cairo_rel_line_to (pCairoContext, - w0, 0);
			cairo_rel_line_to (pCairoContext, 0, - sens * (y * h + fDeltaLineWidth));
		}
		else
		{
			cairo_move_to (pCairoContext, fDockOffsetY - sens * (y * h + fDeltaLineWidth), x0);
			for (i = 0; i < iNbMidPoints+1; i ++)
				cairo_rel_line_to (pCairoContext, sens * (pMidPointCoord[2*i+1] - pMidPointCoord[2*i+3]), pMidPointCoord[2*(i+1)] - pMidPointCoord[2*i]);
			cairo_rel_line_to (pCairoContext, sens * (y_ * h + fDeltaLineWidth), 0);
			cairo_rel_line_to (pCairoContext, 0, - w0);
			cairo_rel_line_to (pCairoContext, - sens * (y * h + fDeltaLineWidth), 0);
		}
		
		g_free (pMidPointCoord);
		cairo_clip (pCairoContext);
	}
	
	
	double fHeight, fBigWidth, fLittleWidth;
	if (bIncludeEdges)
	{
		fHeight = (bBackGround ? pDock->iDecorationsHeight - hi : hi) + myBackground.iDockLineWidth;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY : iVanishingPointY + fHeight);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY - fHeight : iVanishingPointY);
	}
	else
	{
		fHeight = pDock->iDecorationsHeight - myBackground.iDockLineWidth;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi - fHeight);
	}
	double fDeltaXLeft = fHeight * fLeftInclination;
	double fDeltaXRight = fHeight * fRightInclination;
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	
	int sens;
	double fDockOffsetX, fDockOffsetY;
	if (pDock->bDirectionUp)
	{
		sens = 1;
		if (bIncludeEdges)
			fDockOffsetY = pDock->iCurrentHeight - fHeight - (bBackGround ? myBackground.iDockLineWidth + hi : 0);
		else
			fDockOffsetY = pDock->iCurrentHeight - fHeight - myBackground.iDockLineWidth;
	}
	else
	{
		sens = -1;
		if (bIncludeEdges)
			fDockOffsetY = fHeight + (bBackGround ? myBackground.iDockLineWidth + hi : 0);
		else
			fDockOffsetY = fHeight + myBackground.iDockLineWidth;
	}
	
	if (bIncludeEdges)
		fDockOffsetX = icon->fDrawX - (bBackGround ? fHeight * fLeftInclination : 0);
	else
		fDockOffsetX = (pDock->bDirectionUp ? icon->fDrawX - (fHeight - hi) * fLeftInclination : icon->fDrawX - (fHeight - hi) * fLeftInclination);
	fDockOffsetX -= fDeltaInterIconLeft * fLeftInclination*sens;
	
	if (pDock->bHorizontalDock)
	{
		cairo_translate (pCairoContext, fDockOffsetX, fDockOffsetY);  // coin haut gauche.
		cairo_move_to (pCairoContext, 0, 0);  // coin haut gauche.
		
		cairo_rel_line_to (pCairoContext, fLittleWidth, 0);
		cairo_rel_line_to (pCairoContext, fDeltaXRight, sens * fHeight);
		cairo_rel_line_to (pCairoContext, - fBigWidth, 0);
		cairo_rel_line_to (pCairoContext, - fDeltaXLeft, - sens * fHeight);
		
		if (my_curve_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, 1, -1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL], MIN (0, (fHeight + hi) * fLeftInclination), 0);
		}
	}
	else
	{
		cairo_translate (pCairoContext, fDockOffsetY, fDockOffsetX);  // coin haut gauche.
		cairo_move_to (pCairoContext, 0, 0);  // coin haut gauche.
		
		cairo_rel_line_to (pCairoContext, 0, fLittleWidth);
		cairo_rel_line_to (pCairoContext, sens * fHeight, fDeltaXRight);
		cairo_rel_line_to (pCairoContext, 0, - fBigWidth);
		cairo_rel_line_to (pCairoContext, - sens * fHeight, - fDeltaXLeft);
		
		if (my_curve_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, -1, 1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL], 0, MIN (0, (fHeight + hi) * fLeftInclination));
		}
	}
}*/
