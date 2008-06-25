/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

This rendering is (was) written by parAdOxxx_ZeRo, co mah blog : http://paradoxxx.zero.free.fr/ :D

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>

#include "rendering-3D-plane.h"
#include "rendering-curve.h"

#define REFERENCE_CURVE_NB_POINTS 1000

static double *s_pReferenceCurveS = NULL;
static double *s_pReferenceCurveX = NULL;
static double *s_pReferenceCurveY = NULL;
int iVanishingPointY = 200;

extern cairo_surface_t *my_pFlatSeparatorSurface[2];

extern CDSpeparatorType my_curve_iDrawSeparator3D;
extern double my_curve_fSeparatorColor[4];

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
	
	pDock->iDecorationsHeight = g_iFrameMargin + my_iCurveAmplitude + .5 * pDock->iMaxIconHeight;  // de bas en haut.
	
	pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., 0.));  // etendue max des icones, sans le cadre.
	g_print ("iMaxDockWidth : %d\n", pDock->iMaxDockWidth);
	
	double h = 4./3 * (pDock->iDecorationsHeight + g_iDockLineWidth);  // hauteur de controle de la courbe de Bezier, de telle facon qu'elle atteigne 'iDecorationsHeight'.
	double hi = .5 * pDock->iMaxIconHeight + g_iFrameMargin - 1;  // hauteur de la courbe a la 1ere icone.
	double ti = .5 * (1. - sqrt (MAX (1. - 4./3 * hi / h, 0.01)));
	double xi = xCurve (my_fCurveCurvature, ti);
	double fDeltaX = pDock->iMaxDockWidth * xi / (1 - 2 * xi);  // abscisse de la 1ere icone pour satisfaire a la contrainte y=hi.
	g_print ("ti = %.2f => xi = %.2f => fDeltaX = %.2f\n", ti, xi, fDeltaX);
	
	pDock->iMaxDockWidth += 2*fDeltaX;
	double tan_theta = (my_fCurveCurvature != 1 ? h / ((1 - my_fCurveCurvature) * pDock->iMaxDockWidth / 2) : 1e6);  // la tangente a une courbe de Bezier en son origine est la droite reliant les deux premiers points de controle.
	double fDeltaTip = .5 * g_iDockLineWidth * sqrt (1 + tan_theta * tan_theta) / tan_theta;  // prolongement de la pointe.
	pDock->iMaxDockWidth += 2 * fDeltaTip;
	pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., 2*(fDeltaX+fDeltaTip)));
	g_print ("fDeltaTip : %.2f\n", fDeltaTip);
	
	pDock->iMaxDockHeight = g_iDockLineWidth + g_iFrameMargin + my_iCurveAmplitude + (1 + g_fAmplitude) * pDock->iMaxIconHeight + g_iconTextDescription.iSize;  // de bas en haut.
	
	pDock->iDecorationsWidth = pDock->iMaxDockWidth - 4 * fDeltaTip;
	
	pDock->iMinDockWidth = pDock->fFlatDockWidth / (1 - 2 * xi) + 2 * fDeltaTip;
	g_print ("pDock->fFlatDockWidth = %.2f => pDock->iMinDockWidth = %d\n", pDock->fFlatDockWidth, pDock->iMinDockWidth);
	
	pDock->iMinDockHeight = g_iDockLineWidth + g_iFrameMargin + my_iCurveAmplitude + pDock->iMaxIconHeight;  // de bas en haut.
}

void cd_rendering_calculate_construction_parameters_curve (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, double fReflectionOffsetY, double fCurveY)
{
	icon->fDrawX = icon->fX;
	icon->fDrawY = icon->fY + fReflectionOffsetY + fCurveY;
	icon->fWidthFactor = 1.;
	icon->fHeightFactor = 1.;
	icon->fDeltaYReflection = 0.;
	icon->fOrientation = 0.;
	if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= iCurrentWidth)
	{
		icon->fAlpha = 1;
	}
	else
	{
		icon->fAlpha = .25;
	}
}

static void cd_rendering_make_3D_curve_separator (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bIncludeEdges, gboolean bBackGround)
{
	double fLineWidth = g_iDockLineWidth;
	double fMargin = g_iFrameMargin;
	double hi = g_fReflectSize + g_iFrameMargin;
	double fLeftInclination = (icon->fDrawX - pDock->iCurrentWidth / 2) / iVanishingPointY;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->iCurrentWidth / 2) / iVanishingPointY;
	//g_print ("fLeftInclination : %.2f ; fRightInclination : %.2f\n", fLeftInclination, fRightInclination);
	
	Icon *pPrevIcon = cairo_dock_get_first_drawn_icon (pDock);
	if (pPrevIcon == NULL)
		pPrevIcon = icon;
	Icon *pNextIcon = cairo_dock_get_first_drawn_icon (pDock);
	if (pNextIcon == NULL)
		pNextIcon = icon;
	
	if (bBackGround)  // pour s'arreter sur la courbe, on realise un clippage.
	{
		//\________________ On se ramene au cas du dessin optimise.
		double x0, y0, xf, yf, w0, h0;
		double fDeltaInterIconLeft, fDeltaInterIconRight;
		if (pDock->bDirectionUp)
		{
			fDeltaInterIconLeft = (pPrevIcon->fDrawY + pPrevIcon->fHeight * pPrevIcon->fScale) - (icon->fDrawY + icon->fHeight * icon->fScale);
			fDeltaInterIconRight = (icon->fDrawY + icon->fHeight * icon->fScale) - (pNextIcon->fDrawY + pNextIcon->fHeight * pNextIcon->fScale);
			
			x0 = icon->fDrawX + fDeltaInterIconLeft * fLeftInclination - MAX (0, fLeftInclination * (pPrevIcon->fDrawY + pPrevIcon->fHeight * pPrevIcon->fScale));
			xf = icon->fDrawX - fDeltaInterIconRight * fRightInclination + icon->fWidth * icon->fScale - MIN (0, fRightInclination * (pNextIcon->fDrawY + pNextIcon->fHeight * pNextIcon->fScale));
		}
		else
		{
			fDeltaInterIconLeft = (pPrevIcon->fDrawY) - (icon->fDrawY);
			fDeltaInterIconRight = (icon->fDrawY) - (pNextIcon->fDrawY);
			x0 = icon->fDrawX - MAX (0, fLeftInclination * (pDock->iCurrentHeight - (pPrevIcon->fDrawY + pPrevIcon->fHeight * pPrevIcon->fScale)));
			xf = icon->fDrawX + icon->fWidth * icon->fScale - MIN (0, fRightInclination * (pDock->iCurrentHeight - (pNextIcon->fDrawY + pNextIcon->fHeight * pNextIcon->fScale)));
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
		
		double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock) - 2 * g_iFrameMargin;
		
		double h = 4./3 * (pDock->iDecorationsHeight + g_iDockLineWidth);
		double hi = .5 * pDock->iMaxIconHeight + g_iFrameMargin - 1;
		double ti = .5 * (1. - sqrt (MAX (1. - 4./3 * hi / h, 0)));
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
		
		cairo_save (pCairoContext);
		if (pDock->bHorizontalDock)
		{
			cairo_move_to (pCairoContext, x0, fDockOffsetY - sens * (y * h + (bIncludeEdges ? fLineWidth : 0)));
			for (i = 0; i < iNbMidPoints+1; i ++)
				cairo_rel_line_to (pCairoContext, pMidPointCoord[2*(i+1)] - pMidPointCoord[2*i], sens * (pMidPointCoord[2*i+1] - pMidPointCoord[2*i+3]));
			cairo_rel_line_to (pCairoContext, 0, sens * (y_ * h + (bIncludeEdges ? fLineWidth : 0)));
			cairo_rel_line_to (pCairoContext, - w0, 0);
			cairo_rel_line_to (pCairoContext, 0, - sens * (y * h + (bIncludeEdges ? fLineWidth : 0)));
		}
		else
		{
			cairo_move_to (pCairoContext, fDockOffsetY - sens * y * h, x0);
			for (i = 0; i < iNbMidPoints+1; i ++)
				cairo_rel_line_to (pCairoContext, sens * (pMidPointCoord[2*i+1] - pMidPointCoord[2*i+3]), pMidPointCoord[2*(i+1)] - pMidPointCoord[2*i]);
			cairo_rel_line_to (pCairoContext, sens * y_ * h, 0);
			cairo_rel_line_to (pCairoContext, 0, - w0);
			cairo_rel_line_to (pCairoContext, - sens * y * h, 0);
		}
		g_free (pMidPointCoord);
		cairo_clip (pCairoContext);
	}
	
	
	double fHeight, fBigWidth, fLittleWidth;
	if (bIncludeEdges)
	{
		fHeight = (bBackGround ? pDock->iDecorationsHeight - hi : hi) + g_iDockLineWidth;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY : iVanishingPointY + fHeight);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY - fHeight : iVanishingPointY);
	}
	else
	{
		fHeight = pDock->iDecorationsHeight - g_iDockLineWidth;
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
			fDockOffsetY = pDock->iCurrentHeight - fHeight - (bBackGround ? g_iDockLineWidth + hi : 0);
		else
			fDockOffsetY = pDock->iCurrentHeight - fHeight - g_iDockLineWidth;
	}
	else
	{
		sens = -1;
		if (bIncludeEdges)
			fDockOffsetY = fHeight + (bBackGround ? g_iDockLineWidth + hi : 0);
		else
			fDockOffsetY = fHeight + g_iDockLineWidth;
	}
	if (bIncludeEdges)
		fDockOffsetX = icon->fDrawX - (bBackGround ? fHeight * fLeftInclination : 0);
	else
		fDockOffsetX = icon->fDrawX - (fHeight - hi) * fLeftInclination;
	
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
	double hi = g_fReflectSize + g_iFrameMargin;
	double fLeftInclination = (icon->fDrawX - pDock->iCurrentWidth / 2) / iVanishingPointY;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->iCurrentWidth / 2) / iVanishingPointY;
	
	double fHeight, fBigWidth, fLittleWidth;
	fHeight = (bBackGround ? pDock->iDecorationsHeight - hi - 0.5*g_iDockLineWidth : hi + 1.5*g_iDockLineWidth);
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
		fDockOffsetY =  (bBackGround ? 0.5*g_iDockLineWidth : - 1.*g_iDockLineWidth);
	}
	else
	{
		sens = -1;
		fDockOffsetY =  (bBackGround ? - 0.5*g_iDockLineWidth : 1.*g_iDockLineWidth);
	}
	fDockOffsetX = (bBackGround ? .5*g_iDockLineWidth * fLeftInclination + 1.*fLeftInclination : - 0.5 * g_iDockLineWidth * fLeftInclination);
	//fDockOffsetX = -.5*g_iDockLineWidth;
	
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
		cairo_set_line_width (pCairoContext, g_iDockLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
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
	double fLineWidth = g_iDockLineWidth;
	double fMargin = g_iFrameMargin;
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock) - 2 * g_iFrameMargin;
	
	double h = 4./3 * (pDock->iDecorationsHeight + g_iDockLineWidth);
	double hi = .5 * pDock->iMaxIconHeight + g_iFrameMargin - 1;
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
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY);
	
	//\____________________ On dessine le cadre.
	if (fLineWidth > 0)
	{
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		cairo_stroke (pCairoContext);
	}
	else
		cairo_new_path (pCairoContext);
	cairo_restore (pCairoContext);
	
	//\____________________ On dessine la ficelle qui les joint.
	if (g_iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, g_iStringLineWidth, FALSE, (my_pFlatSeparatorSurface != NULL));
	
	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
	fRatio = pDock->fRatio;
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement == NULL)
		return ;
		
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	
	if (my_pFlatSeparatorSurface[0] != NULL || my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
	{
		cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_SQUARE);
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
				cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio, fDockMagnitude, pDock->bUseReflect, TRUE, pDock->iCurrentWidth, pDock->bDirectionUp);
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
			cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio, fDockMagnitude, pDock->bUseReflect, TRUE, pDock->iCurrentWidth, pDock->bDirectionUp);
			cairo_restore (pCairoContext);
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
	}
}



static gboolean _cd_separator_is_impacted (Icon *icon, CairoDock *pDock, double fXMin, double fXMax, gboolean bBackGround, gboolean bIncludeEdges)
{
	double hi = g_fReflectSize + g_iFrameMargin;
	double fLeftInclination = fabs (icon->fDrawX - pDock->iCurrentWidth / 2) / iVanishingPointY;
	double fRightInclination = fabs (icon->fDrawX + icon->fWidth * icon->fScale - pDock->iCurrentWidth / 2) / iVanishingPointY;
	
	double fHeight, fBigWidth, fLittleWidth;
	if (bIncludeEdges)
	{
		fHeight = (bBackGround ? pDock->iDecorationsHeight - hi : hi) + (bIncludeEdges ? g_iDockLineWidth : 0);
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
			fDockOffsetY =  pDock->iCurrentHeight - fHeight - (bBackGround ? g_iDockLineWidth + hi : 0);
		else
			fDockOffsetY =  pDock->iCurrentHeight - fHeight;
	}
	else
	{
		sens = -1;
		if (bIncludeEdges)
			fDockOffsetY = fHeight + (bBackGround ? g_iDockLineWidth + hi : 0);
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
	double fLineWidth = g_iDockLineWidth;
	double fMargin = g_iFrameMargin;
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock) - 2 * g_iFrameMargin;
	
	double h = 4./3 * (pDock->iDecorationsHeight + g_iDockLineWidth);
	double hi = .5 * pDock->iMaxIconHeight + g_iFrameMargin - 1;
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
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY);
	
	//\____________________ On dessine le cadre.
	cairo_new_path (pCairoContext);
	if (fLineWidth > 0)
	{
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		
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
	g_print ("x : %.2f => t : %.2f\n", x, t);*/
	
	//\____________________ On dessine les icones impactees.
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement != NULL)
	{
		double fXMin = (pDock->bHorizontalDock ? pArea->x : pArea->y), fXMax = (pDock->bHorizontalDock ? pArea->x + pArea->width : pArea->y + pArea->height);
		double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
		double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
		fRatio = pDock->fRatio;
		double fXLeft, fXRight;
		Icon *icon;
		GList *ic = pFirstDrawnElement;
		
		if (my_pFlatSeparatorSurface[0] != NULL || my_curve_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
		{
			cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_SQUARE);
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
						if (icon->iAnimationType == CAIRO_DOCK_AVOID_MOUSE)
							icon->fAlpha = 0.4;
						else if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= pDock->iCurrentWidth)
							icon->fAlpha = 1;
						else
							icon->fAlpha = .25;
						
						cairo_save (pCairoContext);
						
						cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio, fDockMagnitude, pDock->bUseReflect, TRUE, pDock->iCurrentWidth, pDock->bDirectionUp);
						
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
					if (icon->iAnimationType == CAIRO_DOCK_AVOID_MOUSE)
						icon->fAlpha = 0.4;
					else if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= pDock->iCurrentWidth)
						icon->fAlpha = 1;
					else
						icon->fAlpha = .25;
					
					cairo_save (pCairoContext);
					
					cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio, fDockMagnitude, pDock->bUseReflect, TRUE, pDock->iCurrentWidth, pDock->bDirectionUp);
					
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
	
	CairoDockMousePositionType iMousePositionType = cairo_dock_check_if_mouse_inside_linear (pDock);
	
	cairo_dock_manage_mouse_position (pDock, iMousePositionType);
	
	//\____________________ On calcule les position/etirements/alpha des icones.
	cairo_dock_mark_avoiding_mouse_icons_linear (pDock);
	
	double h = 4./3 * (pDock->iDecorationsHeight + g_iDockLineWidth);
	double hi = .5 * pDock->iMaxIconHeight + g_iFrameMargin - 1;
	double ti = .5 * (1. - sqrt (MAX (1. - 4./3 * hi / h, 0)));
	double xi = xCurve (my_fCurveCurvature, ti);
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock) - 2 * g_iFrameMargin;
	double curveOffsetX = fDockWidth * xi;  // pDock->iMaxDockWidth
	
	gint sens, fDockOffsetY;
	sens = pDock->bDirectionUp ? 1 : -1;
	double fReflectionOffsetY = (pDock->bDirectionUp ? -1 : 1) * g_fReflectSize;
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
	
	k1 = ya/((xa-xb)*(xa-xc));
	k2 = yb/((xb-xa)*(xb-xc));
	k3 = yc/((xc-xa)*(xc-xb));
	
	
	Icon* icon;
	GList* ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		double x = icon->fX - pFirstIcon->fX;
		double y = k1*(x-xb)*(x-xc) + k2*(x-xa)*(x-xc) + k3*(x-xa)*(x-xb);
		cd_rendering_calculate_construction_parameters_curve (icon, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iMaxDockWidth, fReflectionOffsetY,
		      sens * y);
		
		icon->fDrawX = icon->fX;
		icon->fDrawY = icon->fY + sens * y;
		
		cairo_dock_manage_animations (icon, pDock);
	}
	
	return (iMousePositionType == CAIRO_DOCK_MOUSE_INSIDE ? pPointedIcon : NULL);
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
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;
	pRenderer->bUseReflect = TRUE;
	
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
		s_pReferenceCurveS = g_new (double, REFERENCE_CURVE_NB_POINTS+1);
	}
	
	if (s_pReferenceCurveX == NULL)
	{
		s_pReferenceCurveX = g_new (double, REFERENCE_CURVE_NB_POINTS+1);
	}
	
	if (s_pReferenceCurveY == NULL)
	{
		s_pReferenceCurveY = g_new (double, REFERENCE_CURVE_NB_POINTS+1);
	}
	
	double s, x, y;
	int i;
	for (i = 0; i < REFERENCE_CURVE_NB_POINTS+1; i ++)
	{
		s = (double) i / REFERENCE_CURVE_NB_POINTS;
		
		s_pReferenceCurveS[i] = s;
		s_pReferenceCurveX[i] = xCurve (my_fCurveCurvature, s);
		s_pReferenceCurveY[i] = yCurve (s);
	}
}

double cd_rendering_interpol_curve (double x, double *fXValues, double *fYValues)
{
	double y;
	int i, i_inf=0, i_sup=REFERENCE_CURVE_NB_POINTS-1;
	do
	{
		i = (i_inf + i_sup) / 2;
		if (fXValues[i] < x)
			i_inf = i;
		else
			i_sup = i;
	}
	while (i_sup - i_inf > 1);
	
	double x_inf = fXValues[i_inf];
	double x_sup = fXValues[i_sup];
	return (x_sup != x_inf ? ((x - x_inf) * fYValues[i_sup] + (x_sup - x) * fYValues[i_inf]) / (x_sup - x_inf) : fYValues[i_inf]);
}

double cd_rendering_interpol_curve_parameter (double x)
{
	return cd_rendering_interpol_curve (x, s_pReferenceCurveX, s_pReferenceCurveS);
}

double cd_rendering_interpol_curve_height (double x)
{
	return cd_rendering_interpol_curve (x, s_pReferenceCurveX, s_pReferenceCurveY);
}

