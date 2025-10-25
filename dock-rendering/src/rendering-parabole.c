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

/*********************************************************************************
On definit une courbe de la forme y = f(x); on decide de prendre
    f : x |--> lambda * x ^ alpha.
On se ramene a un dock lineaire en passant aux coordonnees curvilignes par la formule :
    s = int (0, x, sqrt (1 + f'(x)^2)dx),
ce qui nous donne un systeme de 2 equations a 2 inconnnues (x et y)
En approximant l'integrale par une somme discrete, on se rend compte qu'on peut re-ecrire le probleme sous la forme d'une equa-diff du 1er ordre.
On utilise alors une méthode de Runge-Kutta d'ordre 4, ce qui pour un nombre d'icones N peu eleve donne un tres bon resultat.
Pour optimiser le calcul, on remarque par un changement de variable idoine que l'on a la relation :
    s_lambda(x) = s_lambda'(c*x) / c, avec c = (lambda / lambda')^(1 / (alpha - 1))
ce qui nous permet de calculer une parabole par interpolation lineaire sur une parabole de reference.
La vague se rajoute dans un 2eme temps. On cherche a ancrer la 1ere icone en (0,0).
Pour se faire, on calcule la position de la crete de la vague par une suite convergente :
    sn+1 = vague (sn)
La position sur la vague est deduite de la position du curseur par projection normale sur la courbe; on remarque qu'on peut borner s dans un intervalle de largeur moyenne egale a L/2. On procede alors par dichotomie sur ce segment en prolongeant en chaque point suivant grad(f) jusqu'a tomber sur le curseur.
Voila pour la petite explication :-)
*********************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>

#ifdef HAVE_GLITZ
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include "rendering-commons.h"
#include "rendering-parabole.h"

extern double my_fParaboleCurvature;  // puissance de x (alpha).
extern double my_fParaboleRatio;  // ratio hauteur / largeur fixe => coef de la parabole (lambda).
extern double my_fParaboleMagnitude;
extern int my_iParaboleTextGap;
extern gboolean my_bDrawTextWhileUnfolding;
extern gboolean my_bParaboleCurveOutside;

static double *s_pReferenceParaboleS = NULL;
static double *s_pReferenceParaboleX = NULL;
static double *s_pReferenceParaboleY = NULL;
static double *s_pReferenceParaboleT = NULL;

#define fCurve(x, lambda, alpha) (lambda * pow (x, alpha))
#define fCurve_(x, lambda, alpha) (x != 0 ? lambda * alpha * pow (x, alpha - 1) : (alpha > 1 ? 0 : alpha < 1 ? 1e6 : alpha * lambda))
#define fCurveInv(y, lambda, alpha) pow (y / lambda, 1. / alpha)
#define fCurveInv_(y, lambda, alpha) (y != 0 ? alpha * y * pow (lambda / y, 1. / alpha) : (alpha > 1 ? 0 : alpha < 1 ? 1e6 : alpha * pow (lambda, 1. / alpha)))

#define PARABOLE_DISTANCE_OUT2 4900
#define PARABOLE_DISTANCE_EDGE2 2500

static void cd_rendering_set_subdock_position_parabole (Icon *pPointedIcon, CairoDock *pDock)
{
	CairoDock *pSubDock = pPointedIcon->pSubDock;
	int iMouseX = pDock->container.iMouseX;
	//int iX = iMouseX + (-iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2) / 2;
	int iX = iMouseX;
	
	int W = gldi_dock_get_screen_width (pDock);
	int iScreenOffsetX = gldi_dock_get_screen_offset_x (pDock);
	if ((pDock->container.iWindowPositionX - iScreenOffsetX + pPointedIcon->fDrawX < W / 2) ^ my_bParaboleCurveOutside)
	{
		///iX = iMouseX + MIN (0, -iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2);
		iX = pPointedIcon->fDrawX;
		if (pSubDock->icons != NULL)
		{
			Icon *icon = pSubDock->icons->data;
			iX += (pPointedIcon->fWidth * pPointedIcon->fScale - icon->fWidth) / 2;
		}
		//cd_debug ("recalage : %.2f (%d)", -iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2, pSubDock->iMaxLabelWidth);
		pSubDock->fAlign = 0;
		pSubDock->iGapY = (pDock->iGapY + pDock->iActiveHeight);
		pSubDock->iGapX = iX + pDock->container.iWindowPositionX - iScreenOffsetX - pSubDock->iMaxLabelWidth;
	}
	else
	{
		///iX = iMouseX + MAX (0, -iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2);
		iX = pPointedIcon->fDrawX;
		if (pSubDock->icons != NULL)
		{
			Icon *icon = pSubDock->icons->data;
			iX += icon->fWidth + (pPointedIcon->fWidth * pPointedIcon->fScale - icon->fWidth) / 2;
		}
		pSubDock->fAlign = 1;
		pSubDock->iGapY = (pDock->iGapY + pDock->iActiveHeight);
		pSubDock->iGapX =  pDock->container.iWindowPositionX - iScreenOffsetX + iX - W + pSubDock->iMaxLabelWidth;
		
	}
	//cd_debug ("pSubDock->iGapY : %d", pSubDock->iGapY);
}


static void cd_rendering_calculate_next_point (double xn, double yn, double ds, double lambda, double alpha, double *fXNext, double *fYNext, double *fOrientation)
{
	//g_print ("%s ((%.2f;%.2f), +%.2f\n", __func__, xn, yn, ds);
	if (ds <= 0)
	{
		return ;
	}
	double k1, k2, k3, k4, k;
	double Txn, Tyn, Tn;
	double Txnplus1_2, Tynplus1_2, Tnplus1_2, xnplus1_2, ynplus1_2;
	double  xnplus1, ynplus1;
	
	// k1.
	k1 = fCurve_ (xn, lambda, alpha);
	//g_print ("k1 : %.2f ; ", k1);
	
	// k2.
	Tn = sqrt (1 + k1 * k1);
	Txn = 1. / Tn;
	Tyn = k1 / Tn;
	*fOrientation = atan (Txn / Tyn);
	if (k1 > 1)
	{
		ynplus1_2 = yn + ds/2 * Tyn;
		k2 = fCurveInv_ (ynplus1_2, lambda, alpha);
	}
	else
	{
		xnplus1_2 = xn + ds/2 * Txn;
		k2 =fCurve_ (xnplus1_2, lambda, alpha);
	}
	//g_print ("k2 : %.2f ; ", k2);
	
	// k3.
	Tnplus1_2 = sqrt (1 + k2 * k2);
	if (k2 > 1)
	{
		Tynplus1_2 = k2 / Tnplus1_2;
		ynplus1_2 = yn + ds/2 * Tynplus1_2;
		k3 = fCurveInv_ (ynplus1_2, lambda, alpha);
	}
	else
	{
		Txnplus1_2 = 1. / Tnplus1_2;
		xnplus1_2 = xn + ds/2 * Txnplus1_2;
		k3 = fCurve_ (xnplus1_2, lambda, alpha);
	}
	//g_print ("k3 : %.2f ; ", k3);
	
	// k4.
	Tnplus1_2 = sqrt (1 + k3 * k3);
	if (k3 > 1)
	{
		Tynplus1_2 = k3 / Tnplus1_2;
		ynplus1 = yn + ds * Tynplus1_2;
		k4 = fCurveInv_ (ynplus1, lambda, alpha);
	}
	else
	{
		Txnplus1_2 = 1. / Tnplus1_2;
		xnplus1 = xn + ds * Txnplus1_2;
		k4 = fCurve_ (xnplus1, lambda, alpha);
	}
	//g_print ("k4 : %.2f \n", k4);
	
	// k.
	k = 1/(1/k1 + 2 / k2 + 2 / k3 + 1/k4) * 6;
	
	// (xn+1, yn+1).
	Tn = sqrt (1 + k * k);
	Txn = 1. / Tn;
	Tyn = k / Tn;
	*fXNext = xn+ ds * Txn;
	*fYNext = yn+ ds * Tyn;
	*fOrientation = atan (Txn / Tyn);
}


static void cd_rendering_calculate_reference_parabole (double alpha)
{
	if (s_pReferenceParaboleS == NULL)
	{
		s_pReferenceParaboleS = g_new (double, RENDERING_INTERPOLATION_NB_PTS);
		s_pReferenceParaboleS[0] = 0;
	}
	
	if (s_pReferenceParaboleX == NULL)
	{
		s_pReferenceParaboleX = g_new (double, RENDERING_INTERPOLATION_NB_PTS);
		s_pReferenceParaboleX[0] = 0;
	}
	
	if (s_pReferenceParaboleY == NULL)
	{
		s_pReferenceParaboleY = g_new (double, RENDERING_INTERPOLATION_NB_PTS);
		s_pReferenceParaboleY[0] = 0;
	}
	
	if (s_pReferenceParaboleT == NULL)
	{
		s_pReferenceParaboleT = g_new (double, RENDERING_INTERPOLATION_NB_PTS);
		s_pReferenceParaboleT[0] = 0;
	}
	
	double w = g_desktopGeometry.Xscreen.height / my_fParaboleRatio;
	double lambda = my_fParaboleRatio * pow (w, 1 - alpha);
	double s=0, ds = 1;
	double x = 0, y = 0, theta = 0;
	double x_ = 0, y_ = 0, theta_ = G_PI/2 - atan (fCurve_(0, lambda, alpha));
	int i;
	for (i = 1; i < RENDERING_INTERPOLATION_NB_PTS; i ++)
	{
		s += ds;
		cd_rendering_calculate_next_point (x, y, ds, lambda, alpha, &x_, &y_, &theta_);
		//g_print ("ds = %.2f => (%.2f;%.2f), %.2fdeg\n", ds, x_, y_, theta_/G_PI*180);
		
		x = x_;
		y = y_;
		theta = theta_;
		s_pReferenceParaboleS[i] = s;
		s_pReferenceParaboleX[i] = x;
		s_pReferenceParaboleY[i] = y;
		s_pReferenceParaboleT[i] = theta;
		
		///s += ds;
	}
}


double cd_rendering_interpol_curvilign_abscisse (double x, double y, double lambda, double alpha)
{
	double w = g_desktopGeometry.Xscreen.height / my_fParaboleRatio;  // aie, au changement de resolution ...
	double lambda_reference = my_fParaboleRatio * pow (w, 1 - alpha);
	//cd_debug ("%s (%.2f / %.2f)", __func__, lambda, lambda_reference);
	if (my_fParaboleRatio < 1)
	{
		double coef = pow (lambda / lambda_reference, 1. / (alpha - 1));
		//cd_debug (" xcoef : %.2f", coef);
		return cd_rendering_interpol (x * coef, s_pReferenceParaboleX, s_pReferenceParaboleS) / coef;
	}
	else
	{
		double coef = pow (lambda / lambda_reference, - 1. / alpha);
		//cd_debug (" ycoef : %.2f", coef);
		return cd_rendering_interpol (y * coef, s_pReferenceParaboleY, s_pReferenceParaboleS) / coef;
	}
}


static void cd_rendering_calculate_max_dock_size_parabole (CairoDock *pDock)
{
	static double fParaboleCurvature= 0;
	if (s_pReferenceParaboleS == NULL || my_fParaboleCurvature != fParaboleCurvature)
	{
		fParaboleCurvature = my_fParaboleCurvature;
		cd_rendering_calculate_reference_parabole (my_fParaboleCurvature);
	}
	
	pDock->fMagnitudeMax = my_fParaboleMagnitude;
	cairo_dock_calculate_icons_positions_at_rest_linear (pDock);
	
	int Ws = gldi_dock_get_screen_width (pDock) - 2;  // let 1px on each edge, so that we can leave the dock even if it gets huge.
	int Hs = gldi_dock_get_screen_height (pDock) - 2;
	int iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->fFlatDockWidth, 1., 0));
	GList* ic;
	Icon *icon;
	pDock->iMaxLabelWidth = 0;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		//cd_debug ("  fXAtRest : %.2f; [%.2f;%.2f]", icon->fXAtRest, icon->fXMin, icon->fXMax);
		icon->fXMax = icon->fXAtRest + 1e4;
		icon->fXMin = icon->fXAtRest - 1e4;
		pDock->iMaxLabelWidth = MAX (pDock->iMaxLabelWidth, icon->label.iWidth);
	}
	//cd_debug ("> iMaxLabelWidth : %d+%d", pDock->iMaxLabelWidth, my_iParaboleTextGap);
	pDock->iMaxLabelWidth += my_iParaboleTextGap;
	
	//double alpha = my_fParaboleCurvature;
	double h=0, w=0, h_, w_;
	if (my_fParaboleRatio > 1)
	{
		h_ = iMaxDockWidth;
		do
		{
			h = h_;
			w = h / my_fParaboleRatio;
			//lambda = h / pow (w, alpha);
			//cd_debug ("-> %.2fx%.2f , %.2f", w, h, lambda);
			
			h_ = cd_rendering_interpol (iMaxDockWidth, s_pReferenceParaboleS, s_pReferenceParaboleY);
			w_ = h_ / my_fParaboleRatio;
		}
		while (fabs (h - h_ >2));
		
		h = h_;
		w = w_;
		//lambda = h / pow (w, alpha);
		//cd_debug ("=> %.2fx%.2f , %.2f", w, h, lambda);
	}
	
	pDock->iMaxDockHeight = h + pDock->iMaxIconHeight * sqrt (5./4.) * (1 + my_fParaboleMagnitude * myIconsParam.fAmplitude);
	pDock->iMaxDockWidth = w + pDock->iMaxIconHeight * (.5+sqrt(5./4.)) * (1 + my_fParaboleMagnitude * myIconsParam.fAmplitude);  // ce serait plutot MaxIconWidth mais bon ...
	
	pDock->iMaxDockWidth += pDock->iMaxLabelWidth;  // theta(0) = 0 => texte horizontal.
	double fOrientationMax = G_PI/2 - atan (my_fParaboleRatio * my_fParaboleCurvature);  // fCurve_ (W) se simplifie ici.
	pDock->iMaxDockHeight += pDock->iMaxLabelWidth * sin (fOrientationMax);  // thetaMax est atteint en x=W.
	//cd_debug ("> fOrientationMax : %.2fdeg -> %dx%d", fOrientationMax/G_PI*180., pDock->iMaxDockWidth, pDock->iMaxDockHeight);
	
	if (pDock->iMaxDockHeight > Hs)
		pDock->iMaxDockHeight = Hs;
	if (pDock->iMaxDockWidth > Ws)
		pDock->iMaxDockWidth = Ws;
	
	pDock->iDecorationsWidth = 0;
	pDock->iDecorationsHeight = 0;
	
	pDock->iMinDockWidth = pDock->iMaxDockWidth;
	pDock->iMinDockHeight = pDock->iMaxIconHeight;
	
	pDock->iActiveWidth = pDock->iMaxDockWidth;
	pDock->iActiveHeight = pDock->iMaxDockHeight;
}


static void cd_rendering_render_parabole (cairo_t *pCairoContext, CairoDock *pDock)
{
	//g_print ("pDock->fFoldingFactor : %.2f\n", pDock->fFoldingFactor);
	
	//\____________________ On trace le cadre.
	
	//\____________________ On dessine les decorations dedans.
	
	//\____________________ On dessine le cadre.
	
	//\____________________ On dessine la ficelle qui les joint.
	if (myIconsParam.iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, myIconsParam.iStringLineWidth, FALSE, FALSE);
	
	
	//\____________________ On dessine les icones et leurs etiquettes, mais separement.
	GList *pFirstDrawnElement = pDock->icons;
	if (pFirstDrawnElement == NULL)
		return;
	
	double fDockMagnitude = 1;  // pour le rendu des icones, on utilise la magnitude max.
	double fAlpha = (1 - pDock->fFoldingFactor) * (1 - pDock->fFoldingFactor);
	gboolean bHorizontal = pDock->container.bIsHorizontal;
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		
		cairo_save (pCairoContext);
		cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, FALSE);
		cairo_restore (pCairoContext);
		
		if (icon->label.pSurface != NULL && (my_bDrawTextWhileUnfolding || pDock->fFoldingFactor == 0))
		{
			cairo_save (pCairoContext);
			if (bHorizontal)
				cairo_translate (pCairoContext, icon->fDrawX, icon->fDrawY);
			else
				cairo_translate (pCairoContext, icon->fDrawY, icon->fDrawX);
			cairo_rotate (pCairoContext, icon->fOrientation);
			if (pDock->fAlign == 1)
			{
				if (bHorizontal)
				{
					/**cairo_set_source_surface (pCairoContext,
						icon->pTextBuffer,
						icon->fWidth * icon->fScale + my_iParaboleTextGap,
						(icon->fHeight * icon->fScale - icon->label.iHeight)/2);*/
					cairo_dock_apply_image_buffer_surface_with_offset (&icon->label, pCairoContext,
						icon->fWidth * icon->fScale + my_iParaboleTextGap,
						(icon->fHeight * icon->fScale - icon->label.iHeight)/2,
						fAlpha);
				}
				else
				{
					cairo_rotate (pCairoContext, G_PI/2);
					/**cairo_set_source_surface (pCairoContext,
						icon->pTextBuffer,
						icon->fWidth * icon->fScale + my_iParaboleTextGap,
						(-icon->fHeight * icon->fScale - icon->label.iHeight)/2);*/
					cairo_dock_apply_image_buffer_surface_with_offset (&icon->label, pCairoContext,
						icon->fWidth * icon->fScale + my_iParaboleTextGap,
						(-icon->fHeight * icon->fScale - icon->label.iHeight)/2,
						fAlpha);
				}
			}
			else
			{
				if (bHorizontal)
				{
					/**cairo_set_source_surface (pCairoContext,
						icon->pTextBuffer,
						- (icon->label.iWidth + my_iParaboleTextGap),
						(icon->fHeight * icon->fScale - icon->label.iHeight)/2);*/
					cairo_dock_apply_image_buffer_surface_with_offset (&icon->label, pCairoContext,
						- (icon->label.iWidth + my_iParaboleTextGap),
						(icon->fHeight * icon->fScale - icon->label.iHeight)/2,
						fAlpha);
				}
				else
				{
					cairo_rotate (pCairoContext, G_PI/2);
					/**cairo_set_source_surface (pCairoContext,
						icon->pTextBuffer,
						- (icon->label.iWidth + my_iParaboleTextGap),
						(-icon->fHeight * icon->fScale - icon->label.iHeight)/2);*/
					cairo_dock_apply_image_buffer_surface_with_offset (&icon->label, pCairoContext,
						- (icon->label.iWidth + my_iParaboleTextGap),
						(-icon->fHeight * icon->fScale - icon->label.iHeight)/2,
						fAlpha);
				}
			}
			/**if (pDock->fFoldingFactor != 0)
				cairo_paint_with_alpha (pCairoContext, (1 - pDock->fFoldingFactor) * (1 - pDock->fFoldingFactor));
			else
				cairo_paint (pCairoContext);*/
			
			cairo_restore (pCairoContext);
		}
		
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
}


static double cd_rendering_project_cursor_on_curve_x (double x0, double y0, double lambda, double alpha)
{
	//cd_debug ("%s (%.2f;%.2f)", __func__, x0, y0);
	if (y0 < 0)
		return 0;
	double xM, yM;  // M se balade sur la courbe.
	double x_inf = 0, x_max = fCurveInv (y0, lambda, alpha), x_sup = x_max;  // bornes de l'intervalle de xM.
	double nx;  // vecteur normal a la courbe.
	double k;  // parametre de la droite normale a la courbe.
	double y_;
	do
	{
		xM = (x_inf + x_sup) / 2;
		
		yM = fCurve (xM, lambda, alpha);
		nx = - fCurve_ (xM, lambda, alpha);
		
		k = (x0 - xM) / nx;
		y_ = yM + k;
		
		if (y_ < y0)
		{
			x_inf = xM;
		}
		else
		{
			x_sup = xM;
		}
	}
	while (x_sup - x_inf > x_max/200);
	return (x_inf + x_sup) / 2;
}
static double cd_rendering_project_cursor_on_curve_y (double x0, double y0, double lambda, double alpha)
{
	//cd_debug ("%s (%.2f;%.2f ; %.2f ; %.2f)", __func__, x0, y0, lambda, alpha);
	if (y0 < 0)
		return 0;
	double xM, yM;  // M se balade sur la courbe.
	double y_inf = 0, y_sup = y0;  // bornes de l'intervalle de xM.
	xM = fCurveInv (y0, lambda, alpha);
	if (xM < x0)
	{
		y_inf = y0;
		y_sup = fCurve (x0, lambda, alpha);
	}
	else
	{
		y_sup = y0;
		y_inf = (x0 > 0 ? fCurve (x0, lambda, alpha) : 0);
	}
	double nx;  // verteur normal a la courbe.
	double k;  // parametre de la droite normale a la courbe.
	double y_;
	//cd_debug ("  yM € [%.2f ; %.2f]", y_inf, y_sup);
	do
	{
		yM = (y_inf + y_sup) / 2;
		//g_print ("  yM <- %.2f\n", yM);
		
		xM = fCurveInv (yM, lambda, alpha);
		nx = - fCurve_ (xM, lambda, alpha);
		
		k = (x0 - xM) / nx;
		y_ = yM + k;
		
		if (y_ < y0)
		{
			y_inf = yM;
		}
		else
		{
			y_sup = yM;
		}
	}
	while (y_sup - y_inf > 1);
	return (y_inf + y_sup) / 2;
}
static void cd_rendering_project_cursor_on_curve (double x0, double y0, double lambda, double alpha, double *fXOnCurve, double *fYOnCurve)
{
	double xM, yM;
	if (my_fParaboleRatio > 1)
	{
		yM = cd_rendering_project_cursor_on_curve_y (x0, y0, lambda, alpha);
		xM = fCurveInv (yM, lambda, alpha);
	}
	else
	{
		xM = cd_rendering_project_cursor_on_curve_x (x0, y0, lambda, alpha);
		yM = fCurve (xM, lambda, alpha);
	}
	*fXOnCurve = xM;
	*fYOnCurve = yM;
}

static double cd_rendering_calculate_wave_position (CairoDock *pDock, double fCurvilignAbscisse, double fMagnitude)
{
	//cd_debug ("%s (%.2f)", __func__, fCurvilignAbscisse);
	
	Icon *pFirstIcon = cairo_dock_get_first_icon (pDock->icons);
	if (pFirstIcon == NULL || fCurvilignAbscisse <= 0)
		return 0;
	double fWaveOffset, fWaveExtrema;
	double x_abs = fCurvilignAbscisse;
	int nb_iter = 0;
	
	do
	{
		//cd_debug ("  x_abs : %.2f / %.2f", x_abs, pDock->fFlatDockWidth);
		cairo_dock_calculate_wave_with_position_linear (pDock->icons, x_abs, fMagnitude, pDock->fFlatDockWidth, pDock->fFlatDockWidth, pDock->container.iHeight, 0*pDock->fAlign, pDock->fFoldingFactor, pDock->container.bDirectionUp);
		fWaveOffset = - pFirstIcon->fX;
		
		fWaveExtrema = fWaveOffset + x_abs;
		x_abs += (fCurvilignAbscisse - fWaveExtrema) / 2;
		if (x_abs > (int) pDock->fFlatDockWidth + fWaveOffset)
		{
			x_abs = (int) pDock->fFlatDockWidth;
			break ;
		}
		//cd_debug ("  -> fWaveExtrema : %.2f", fWaveExtrema);
		
		nb_iter ++;
	}
	while (fabs (fWaveExtrema - fCurvilignAbscisse) > 1 && nb_iter < 15);
	
	return x_abs;
}

Icon *cd_rendering_calculate_icons_parabole (CairoDock *pDock)
{
	if (pDock->icons == NULL)
		return NULL;
	
	//\____________________ On calcule la projection du curseur sur la courbe.
	double fMaxScale =  1. + my_fParaboleMagnitude * myIconsParam.fAmplitude;
	double w = MAX (1, pDock->container.iWidth - pDock->iMaxLabelWidth - pDock->iMaxIconHeight * (.5+sqrt(5./4.)) * fMaxScale);
	double h = my_fParaboleRatio * w;
	double alpha = my_fParaboleCurvature, lambda = h / pow (w, alpha);
	//g_print ("> lambda = %.2f\n", lambda);
	double fXOnCurve, fYOnCurve;
	fXOnCurve = (pDock->fAlign == 0 ? pDock->container.iMouseX - pDock->iMaxLabelWidth - .5*pDock->iMaxIconHeight * fMaxScale : pDock->container.iWidth - pDock->container.iMouseX - pDock->iMaxLabelWidth - .5*pDock->iMaxIconHeight * fMaxScale);
	fYOnCurve = (pDock->container.bDirectionUp ? pDock->container.iHeight - pDock->container.iMouseY : pDock->container.iMouseY);
	//cd_debug (" mouse : %d;%d", pDock->container.iMouseX, pDock->container.iMouseY);
	cd_rendering_project_cursor_on_curve (fXOnCurve, fYOnCurve, lambda, alpha, &fXOnCurve, &fYOnCurve);
	//cd_debug (" on curve : %.2f;%.2f", fXOnCurve, fYOnCurve);
	
	double fCurvilignAbscisse = cd_rendering_interpol_curvilign_abscisse (fXOnCurve, fYOnCurve, lambda, alpha);
	//cd_debug ("  fCurvilignAbscisse : %.2f", fCurvilignAbscisse);
	
	if (pDock->fAlign == 0)
	{
		fXOnCurve = fXOnCurve + pDock->iMaxLabelWidth + .5*pDock->iMaxIconHeight * fMaxScale;
	}
	else
	{
		fXOnCurve = pDock->container.iWidth - fXOnCurve - pDock->iMaxLabelWidth - .5*pDock->iMaxIconHeight * fMaxScale;
	}
	fYOnCurve = pDock->container.iHeight - fYOnCurve;
	//cd_debug (" => %.2f;%.2f (%d ; %d)", fXOnCurve, fYOnCurve, pDock->container.iMouseX, pDock->container.iMouseY);
	
	//\____________________ On en deduit ou appliquer la vague pour que la crete soit a la position du curseur sur la courbe.
	Icon *pPointedIcon = NULL;
	double fMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex) * pDock->fMagnitudeMax;
	//int x_abs = fCurvilignAbscisse;  // ecart par rapport a la gauche du dock minimal  plat.
	int x_abs = (int) round (cd_rendering_calculate_wave_position (pDock, fCurvilignAbscisse, fMagnitude));
	//cd_debug (" => x_abs : %d (fMagnitude:%.2f ; fFoldingFactor:%.2f)", x_abs, fMagnitude, pDock->fFoldingFactor);
	
	//\_______________ On en deduit l'ensemble des parametres des icones.
	pPointedIcon = cairo_dock_calculate_wave_with_position_linear (pDock->icons, x_abs, fMagnitude, (int) pDock->fFlatDockWidth, (int) pDock->fFlatDockWidth, pDock->container.iHeight, 0*pDock->fAlign, pDock->fFoldingFactor, pDock->container.bDirectionUp);
	//cd_debug (" => pPointedIcon : %s; %.2f", pPointedIcon->cName, pPointedIcon->fX);
	
	
	//\____________________ On en deduit les position/etirements/alpha des icones.
	Icon* icon;
	GList* ic;
	double s = 0;
	double x_ = 0, y_ = 0, theta_ = G_PI/2 - atan (fCurve_(0, lambda, alpha));
	if (pDock->fAlign == 1)
		theta_ = -theta_;
	//g_print ("theta_ : %.2fdeg\n", theta_);
	
	GList *pFirstDrawnElement = pDock->icons;
	ic = pFirstDrawnElement;
	Icon *pFirstIcon = pFirstDrawnElement->data;
	// double fOrientationMax = G_PI/2 - atan (my_fParaboleRatio * my_fParaboleCurvature);
	// double fTopMargin = pDock->iMaxLabelWidth * sin (fOrientationMax);
	gboolean bHorizontal = pDock->container.bIsHorizontal;
	do
	{
		icon = ic->data;
		
		if (ic != pFirstDrawnElement)  // on ancre la 1ere icone a l'origine.
		{
			s = icon->fX - pFirstIcon->fX;
			//g_print (" %s : %.2f + %.2f (x %.2f)\n", icon->cName, s, ds, icon->fScale);
			y_ = cd_rendering_interpol (s, s_pReferenceParaboleS, s_pReferenceParaboleY);
			x_ = fCurveInv (y_, lambda, alpha);
			theta_ = G_PI/2 - atan (fCurve_(x_, lambda, alpha));
		}
		
		icon->fDrawY = (pDock->container.bDirectionUp ? pDock->container.iHeight - (y_ + icon->fHeight * icon->fScale) * (1 - pDock->fFoldingFactor) : y_ * (1 - pDock->fFoldingFactor));
		if (pDock->fAlign == 1)
		{
			icon->fDrawX = pDock->container.iWidth - (x_ + pDock->iMaxLabelWidth + .5 * pDock->iMaxIconHeight * fMaxScale + icon->fWidth * icon->fScale/2);
			if (bHorizontal)
				icon->fOrientation = (pDock->container.bDirectionUp ? - theta_ : theta_);
			else
				icon->fOrientation = (pDock->container.bDirectionUp ? theta_ : - theta_);
		}
		else
		{
			icon->fDrawX = x_ + pDock->iMaxLabelWidth + .5 * pDock->iMaxIconHeight * fMaxScale - icon->fWidth * icon->fScale/2;
			if (bHorizontal)
				icon->fOrientation = (pDock->container.bDirectionUp ? theta_ : - theta_);
			else
				icon->fOrientation = (pDock->container.bDirectionUp ? - theta_ : theta_);
		}
		icon->fAlpha = 1;
		icon->fWidthFactor = 1.;
		icon->fHeightFactor = 1.;
		
		//g_print (" %s : (%.2f ; %.2f)\n", icon->cName, x, y);
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
	
	//cd_debug ("  derniere icone : %.2f (s:%.2f)", icon->fX + icon->fHeight * icon->fScale - pFirstIcon->fX, fCurvilignAbscisse);
	if (! pDock->container.bInside || fCurvilignAbscisse > icon->fX + icon->fHeight * icon->fScale - pFirstIcon->fX - (pDock->fFoldingFactor > 0 ? 20 : -30) || fCurvilignAbscisse <= 0)  // on laisse 30 pixels de rab pour ne pas etre trop sensible a l'extremite du dock.
	{
		//cd_debug ("<<< on sort de la parabole >>>");
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_OUTSIDE;
	}
	else if ((pDock->fAlign == 0 && pDock->container.iMouseX > fXOnCurve) || (pDock->fAlign == 1 && pDock->container.iMouseX < fXOnCurve))
	{
		int iMouseY = (pDock->container.bDirectionUp ? pDock->container.iMouseY : pDock->container.iHeight - pDock->container.iMouseY);
		double fDistanceToCurve2 = (fXOnCurve - pDock->container.iMouseX) * (fXOnCurve - pDock->container.iMouseX) + (fYOnCurve - iMouseY) * (fYOnCurve - iMouseY);
		if (fDistanceToCurve2 > PARABOLE_DISTANCE_OUT2)
			pDock->iMousePositionType = CAIRO_DOCK_MOUSE_OUTSIDE;
		else if (fDistanceToCurve2 < PARABOLE_DISTANCE_EDGE2)
			pDock->iMousePositionType = CAIRO_DOCK_MOUSE_INSIDE;
		else
			pDock->iMousePositionType = CAIRO_DOCK_MOUSE_ON_THE_EDGE;
	}
	else
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_INSIDE;
	}
	
	cairo_dock_check_can_drop_linear (pDock);  /// marche ?...
	
	return pPointedIcon;
}


static void cd_rendering_render_parabole_opengl (CairoDock *pDock)
{
	//g_print ("pDock->fFoldingFactor : %.2f\n", pDock->fFoldingFactor);
	
	//\____________________ On trace le cadre.
	
	//\____________________ On dessine les decorations dedans.
	
	//\____________________ On dessine le cadre.
	
	//\____________________ On dessine la ficelle qui les joint.
	if (myIconsParam.iStringLineWidth > 0)
		cairo_dock_draw_string_opengl (pDock, myIconsParam.iStringLineWidth, FALSE, FALSE);
	
	//\____________________ On dessine les icones et leurs etiquettes, mais separement.
	GList *pFirstDrawnElement = pDock->icons;
	if (pFirstDrawnElement == NULL)
		return;
	
	glPushMatrix ();
	double fDockMagnitude = 1;  // pour le rendu des icones, on utilise la magnitude max.
	double fAlpha = (1 - pDock->fFoldingFactor) * (1 - pDock->fFoldingFactor);
	gboolean bHorizontal = pDock->container.bIsHorizontal;
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		
		cairo_dock_render_one_icon_opengl (icon, pDock, fDockMagnitude, FALSE);
		
		if (icon->label.iTexture != 0)  // en opengl on dessine les etiquettes meme pendant le depliage.
		{
			glPushMatrix ();
			cairo_dock_translate_on_icon_opengl (icon, CAIRO_CONTAINER (pDock), 1.);
			
			glTranslatef (-icon->fWidth * icon->fScale/2, icon->fHeight * icon->fScale/2, 0.);
			glRotatef (-icon->fOrientation/G_PI*180., 0., 0., 1.);
			glTranslatef (icon->fWidth * icon->fScale/2, -icon->fHeight * icon->fScale/2, 0.);
			
			_cairo_dock_enable_texture ();
			///_cairo_dock_set_blend_over ();
			_cairo_dock_set_blend_alpha ();
			_cairo_dock_set_alpha (fAlpha);
			
			if (pDock->fAlign == 1)
			{
				if (bHorizontal)
				{
					glTranslatef (icon->fWidth * icon->fScale/2 + my_iParaboleTextGap + icon->label.iWidth/2,
						0.,
						0.);
				}
				else
				{
					glRotatef (-90., 0., 0., 1.);
					glTranslatef (icon->fWidth * icon->fScale/2 + my_iParaboleTextGap + icon->label.iWidth/2,
						0.,
						0.);
				}
			}
			else
			{
				if (bHorizontal)
				{
					glTranslatef (- (icon->fWidth * icon->fScale/2 + my_iParaboleTextGap + icon->label.iWidth/2),
						0.,
						0.);
				}
				else
				{
					glRotatef (-90., 0., 0., 1.);
					glTranslatef (- (icon->fWidth * icon->fScale/2 + my_iParaboleTextGap + icon->label.iWidth/2),
						0.,
						0.);
				}
			}
			/**cairo_dock_apply_texture_at_size (icon->iLabelTexture,
				icon->label.iWidth,
				icon->label.iHeight);*/
			cairo_dock_apply_image_buffer_texture (&icon->label);
			
			_cairo_dock_disable_texture ();
			glPopMatrix ();
		}
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
	glPopMatrix ();
}


void cd_rendering_register_parabole_renderer (const gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	// interface
	pRenderer->compute_size = cd_rendering_calculate_max_dock_size_parabole;
	pRenderer->calculate_icons = cd_rendering_calculate_icons_parabole;
	pRenderer->render = cd_rendering_render_parabole;
	pRenderer->render_optimized = NULL;
	pRenderer->render_opengl = cd_rendering_render_parabole_opengl;
	pRenderer->set_subdock_position = cd_rendering_set_subdock_position_parabole;
	// parametres
	pRenderer->cDisplayedName = D_ (cRendererName);
	pRenderer->cReadmeFilePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/readme-parabolic-view");
	pRenderer->cPreviewFilePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/preview-parabolic.jpg");
	
	cairo_dock_register_renderer (cRendererName, pRenderer);
}
