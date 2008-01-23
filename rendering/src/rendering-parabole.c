/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>

#ifdef HAVE_GLITZ
#include <gdk/gdkx.h>
#include <glitz-glx.h>
#include <cairo-glitz.h>
#endif

#include <rendering-parabole.h>

extern double my_fParaboleCurvature;  // puissance de x (alpha).
extern double my_fParaboleRatio;  // ratio hauteur / largeur fixe => coef de la parabole (lambda).

static double *s_pReferenceParaboleS = NULL;
static double *s_pReferenceParaboleX = NULL;
static double *s_pReferenceParaboleY = NULL;
#define fCurve(x, lambda, alpha) (lambda * pow (x, alpha))
#define fCurve_(x, lambda, alpha) (x != 0 ? lambda * alpha * pow (x, alpha - 1) : (alpha > 1 ? 0 : alpha < 1 ? 1e6 : alpha * lambda))
#define fCurveInv(y, lambda, alpha) pow (y / lambda, 1. / alpha)
#define fCurveInv_(y, lambda, alpha) (y != 0 ? alpha * y * pow (lambda / y, 1. / alpha) : (alpha > 1 ? 0 : alpha < 1 ? 1e6 : alpha * pow (lambda, 1. / alpha)))

void cd_rendering_set_subdock_position_parabole (Icon *pPointedIcon, CairoDock *pDock)
{
	CairoDock *pSubDock = pPointedIcon->pSubDock;
	int iMouseX = pDock->iMouseX;
	//int iX = iMouseX + (-iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2) / 2;
	int iX = iMouseX;
	
	if (pDock->iWindowPositionX + pPointedIcon->fDrawX < g_iScreenWidth[pDock->bHorizontalDock] / 2)
	{
		iX = iMouseX + MIN (0, -iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2);
		g_print ("recalage : %.2f (%d)\n", -iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2, pSubDock->iMaxLabelWidth);
		pSubDock->fAlign = 0;
		pSubDock->iGapY = (pDock->iGapY + pDock->iMaxDockHeight);
		pSubDock->iGapX = iX + pDock->iWindowPositionX - 0*pSubDock->iMaxDockWidth - pSubDock->iMaxLabelWidth;
	}
	else
	{
		iX = iMouseX + MAX (0, -iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2);
		pSubDock->fAlign = 1;
		pSubDock->iGapY = (pDock->iGapY + pDock->iMaxDockHeight);
		pSubDock->iGapX =  pDock->iWindowPositionX + iX - g_iScreenWidth[pDock->bHorizontalDock] + pSubDock->iMaxLabelWidth;
	}
	g_print ("pSubDock->iGapY : %d\n", pSubDock->iGapY);
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


void cd_rendering_calculate_reference_parabole (double alpha)
{
	if (s_pReferenceParaboleS == NULL)
	{
		 s_pReferenceParaboleS = g_new (double, 100);
		s_pReferenceParaboleS[0] = 0;
	}
	
	if (s_pReferenceParaboleX == NULL)
	{
		 s_pReferenceParaboleX = g_new (double, 100);
		s_pReferenceParaboleX[0] = 0;
	}
	
	if (s_pReferenceParaboleY == NULL)
	{
		 s_pReferenceParaboleY = g_new (double, 100);
		s_pReferenceParaboleY[0] = 0;
	}
	double w = g_iScreenHeight[CAIRO_DOCK_HORIZONTAL] / my_fParaboleRatio;
	double lambda = my_fParaboleRatio * pow (w, 1 - alpha);
	double s, ds = 10;
	double x = 0, y = 0, theta = 0;
	double x_ = 0, y_ = 0, theta_ = G_PI/2 - atan (fCurve_(0, lambda, alpha));
	int i;
	for (i = 1; i < 100; i ++)
	{
		cd_rendering_calculate_next_point (x, y, ds, lambda, alpha, &x_, &y_, &theta_);
		//g_print ("ds = %.2f => (%.2f;%.2f), %.2fdeg\n", ds, x_, y_, theta_/G_PI*180);
		
		x = x_;
		y = y_;
		s_pReferenceParaboleS[i] = s;
		s_pReferenceParaboleX[i] = x;
		s_pReferenceParaboleY[i] = y;
		theta = theta_;
		
		s += ds;
	}
}

double cd_rendering_interpol (double x, double lambda, double alpha, double *fXValues, double *fYValues)
{
	double y;
	int i, i_inf=0, i_sup=100-1;
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

double cd_rendering_interpol_curvilign_abscisse(double x, double y, double lambda, double alpha)
{
	if (my_fParaboleRatio < 1)
		return cd_rendering_interpol (x, lambda, alpha, s_pReferenceParaboleX, s_pReferenceParaboleS);
	else
		return cd_rendering_interpol (y, lambda, alpha, s_pReferenceParaboleY, s_pReferenceParaboleS);
}



static void _cd_rendering_calculate_parabole_extent (GList *pIconList, double alpha, double lambda, double *fWidth, double *fHeight)  // donne le bas de la derniere icone.
{
	double ds;
	double x = 0, y = 0, theta = 0;
	double x_ = 0, y_ = 0, theta_ = G_PI/2 - atan (fCurve_(0, lambda, alpha));
	GList *ic;
	Icon *icon, *prev_icon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		
		if (ic->prev != NULL)
		{
			ds = icon->fHeight * (1 - 0) + g_iIconGap;
			cd_rendering_calculate_next_point (x, y, ds, lambda, alpha, &x_, &y_, &theta_);
			//g_print ("ds = %.2f => (%.2f;%.2f), %.2fdeg\n", ds, x_, y_, theta_/G_PI*180);
			
			ds += prev_icon->fWidth / 2 * fabs (tan (theta_ - theta)) * (1 - 0);
			cd_rendering_calculate_next_point (x, y, ds, lambda, alpha, &x_, &y_, &theta_);
			//g_print ("  ds = %.2f => (%.2f;%.2f), %.2fdeg\n", ds, x_, y_, theta_/G_PI*180);
		}
		
		x = x_;
		y = y_;
		theta = theta_;
		prev_icon = icon;
	}
	*fWidth = x;
	*fHeight = y;
}

void cd_rendering_calculate_max_dock_size_parabole (CairoDock *pDock)
{
	pDock->bHorizontalDock = CAIRO_DOCK_VERTICAL;
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->fFlatDockWidth, pDock->iScrollOffset);
	
	//pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., 0));
	GList* ic;
	Icon *icon;
	pDock->iMaxLabelWidth = 0;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->fXMax = icon->fXAtRest + 2*icon->fWidth;
		icon->fXMin = icon->fXAtRest - 2*icon->fWidth;
		pDock->iMaxLabelWidth = MAX (pDock->iMaxLabelWidth, icon->iTextWidth);
	}
	g_print ("> iMaxLabelWidth : %d\n", pDock->iMaxLabelWidth);
	
	double alpha = my_fParaboleCurvature, lambda;
	double h=0, w=0, h_, w_;
	if (my_fParaboleRatio > 1)
	{
		h_ = pDock->fFlatDockWidth;
		do
		{
			h = h_;
			w = h / my_fParaboleRatio;
			lambda = h / pow (w, alpha);
			g_print ("-> %.2fx%.2f , %.2f\n", w, h, lambda);
			
			_cd_rendering_calculate_parabole_extent (pDock->icons, alpha, lambda, &w_, &h_);
		}
		while (fabs (h - h_ >2));
		
		h = h_;
		w = w_;
		lambda = h / pow (w, alpha);
		g_print ("=> %.2fx%.2f , %.2f\n", w, h, lambda);
	}
	
	pDock->iMaxDockHeight = h + pDock->iMaxIconHeight * sqrt (5./4.);
	pDock->iMaxDockWidth = w + pDock->iMaxIconHeight * (.5+sqrt(5./4.));  // ce serait plutot MaxIconWidth mais bon ...
	
	pDock->iMaxDockWidth += pDock->iMaxLabelWidth;  // theta(0) = 0 => texte horizontal.
	double fOrientationMax = G_PI/2 - atan (my_fParaboleRatio * my_fParaboleCurvature);  // fCurve_ (W) se simplifie ici.
	pDock->iMaxDockHeight += pDock->iMaxLabelWidth * sin (fOrientationMax);  // thetaMax est atteint en x=W.
	g_print ("> fOrientationMax : %.2fdeg -> %dx%d\n", fOrientationMax/G_PI*180., pDock->iMaxDockWidth, pDock->iMaxDockHeight);
	
	pDock->iDecorationsWidth = 0;
	pDock->iDecorationsHeight = 0;
	
	pDock->iMinDockWidth = pDock->fFlatDockWidth;
	pDock->iMinDockHeight = pDock->iMaxIconHeight;
}


void cd_rendering_render_parabole (CairoDock *pDock)
{
	//g_print ("pDock->fFoldingFactor : %.2f\n", pDock->fFoldingFactor);
	
	//\____________________ On cree le contexte du dessin.
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pDock);
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	cairo_set_tolerance (pCairoContext, 0.5);  // avec moins que 0.5 on ne voit pas la difference.
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	//\____________________ On trace le cadre.
	
	//\____________________ On dessine les decorations dedans.
	
	//\____________________ On dessine le cadre.
	
	//\____________________ On dessine la ficelle qui les joint.
	if (g_iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, g_iStringLineWidth, FALSE, FALSE);
	
	
	//\____________________ On dessine les etiquettes.
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
	
	double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
	double fDockMagnitude = 1;
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		
		cairo_save (pCairoContext);
		cairo_dock_render_one_icon (icon, pCairoContext, pDock->bHorizontalDock, fRatio, fDockMagnitude, pDock->bUseReflect);
		cairo_restore (pCairoContext);
		
		if (icon->pTextBuffer != NULL)
		{
			cairo_save (pCairoContext);
			cairo_translate (pCairoContext, icon->fDrawX, icon->fDrawY);
			
			cairo_rotate (pCairoContext, icon->fOrientation);
			if (pDock->fAlign == 1)
			{
				cairo_set_source_surface (pCairoContext,
					icon->pTextBuffer,
					icon->fWidth,
					(icon->fHeight - icon->iTextHeight)/2);
			}
			else
			{
				cairo_set_source_surface (pCairoContext,
					icon->pTextBuffer,
					- icon->iTextWidth,
					(icon->fHeight - icon->iTextHeight)/2);
			}
			cairo_paint_with_alpha (pCairoContext, 1 - pDock->fFoldingFactor);
			//cairo_paint (pCairoContext);
			
			cairo_restore (pCairoContext);
		}
		
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
	
	
	//cairo_dock_render_icons_linear (pCairoContext, pDock, fRatio);
	
	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}

CairoDockMousePositionType cd_rendering_check_if_mouse_inside_parabole (CairoDock *pDock)
{
	CairoDockMousePositionType iMousePositionType = (pDock->bInside ? CAIRO_DOCK_MOUSE_INSIDE : CAIRO_DOCK_MOUSE_OUTSIDE);
	
	/// A completer.
	return iMousePositionType;
}


Icon *cairo_dock_apply_no_wave_effect (CairoDock *pDock)
{
	//\_______________ On calcule la position du curseur dans le referentiel du dock a plat.
	int dx = pDock->iMouseX - pDock->iCurrentWidth / 2;  // ecart par rapport au milieu du dock a plat.
	int x_abs = dx + pDock->fFlatDockWidth / 2;  // ecart par rapport a la gauche du dock minimal  plat.
	
	//\_______________ On calcule l'ensemble des parametres des icones.
	double fMagnitude = 0.;
	Icon *pPointedIcon = cairo_dock_calculate_wave_with_position_linear (pDock->icons, pDock->pFirstDrawnElement, x_abs, fMagnitude, pDock->fFlatDockWidth, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->fAlign, pDock->fFoldingFactor);
	return pPointedIcon;
}

static double cd_rendering_project_cursor_on_curve_x (double x0, double y0, double lambda, double alpha)
{
	g_print ("%s (%.2f;%.2f)\n", __func__, x0, y0);
	if (y0 < 0)
		return 0;
	double xM, yM;  // M se balade sur la courbe.
	double x_inf = 0, x_max = fCurveInv (y0, lambda, alpha), x_sup = x_max;  // bornes de l'intervalle de xM.
	double nx, ny=1;  // verteur normal a la courbe.
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
	g_print ("%s (%.2f;%.2f)\n", __func__, x0, y0);
	if (y0 < 0)
		return 0;
	double xM, yM;  // M se balade sur la courbe.
	double y_inf = 0, y_sup = y0;  // bornes de l'intervalle de xM.
	double nx, ny=1;  // verteur normal a la courbe.
	double k;  // parametre de la droite normale a la courbe.
	double y_;
	//g_print ("  yM € [%.2f ; %.2f]\n", y_inf, y_sup);
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
static double cd_rendering_project_cursor_on_curve (double x0, double y0, double lambda, double alpha, double *fXOnCurve, double *fYOnCurve)
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


Icon *cd_rendering_calculate_icons_parabole (CairoDock *pDock)
{
	if (pDock->icons == NULL)
		return NULL;
	
	//\____________________ On calcule la projection du curseur sur la courbe.
	double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
	double w = MAX (1, pDock->iCurrentWidth - pDock->iMaxLabelWidth - pDock->iMaxIconHeight * (.5+sqrt(5./4.)));
	double h = my_fParaboleRatio * w;
	double alpha = my_fParaboleCurvature, lambda =h / pow (w, alpha);
	//g_print ("> lambda = %.2f\n", lambda);
	double fXOnCurve, fYOnCurve;
	fXOnCurve = (pDock->fAlign == 0 ? pDock->iMouseX - pDock->iMaxLabelWidth - .5*pDock->iMaxIconHeight : pDock->iCurrentWidth - (pDock->iMouseX - pDock->iMaxLabelWidth - .5*pDock->iMaxIconHeight));
	fYOnCurve = pDock->iCurrentHeight - pDock->iMouseY;
	cd_rendering_project_cursor_on_curve (fXOnCurve, fYOnCurve, lambda, alpha, &fXOnCurve, &fYOnCurve);
	g_print (" on curve : %.2f;%.2f\n", fXOnCurve, fYOnCurve);
	
	double fCurvilignAbscisse = cd_rendering_interpol_curvilign_abscisse (fXOnCurve, fYOnCurve, lambda, alpha);
	g_print ("  fCurvilignAbscisse : %.2f\n", fCurvilignAbscisse);
	
	if (pDock->fAlign == 0)
	{
		fXOnCurve = fXOnCurve + pDock->iMaxLabelWidth + .5*pDock->iMaxIconHeight;
	}
	else
	{
		fXOnCurve = pDock->iCurrentWidth - (fXOnCurve - pDock->iMaxLabelWidth - .5*pDock->iMaxIconHeight);
	}
	fYOnCurve = pDock->iCurrentHeight - fYOnCurve;
	g_print (" => %.2f;%.2f (%d ; %d)\n", fXOnCurve, fYOnCurve, pDock->iMouseX, pDock->iMouseY);
	
	//\____________________ On en deduit l'icone pointee.
	Icon *pPointedIcon = NULL;
	///pPointedIcon = cairo_dock_apply_wave_effect (pDock);
	
	CairoDockMousePositionType iMousePositionType = cd_rendering_check_if_mouse_inside_parabole (pDock);
	
	cairo_dock_manage_mouse_position (pDock, iMousePositionType);
	
	///cairo_dock_mark_avoiding_mouse_icons_linear (pDock);
	
	
	//\____________________ On calcule les position/etirements/alpha des icones.
	Icon* icon, *prev_icon;
	GList* ic;
	double ds;
	double x = 0, y = 0, theta = 0;
	double x_ = 0, y_ = 0, theta_ = G_PI/2 - atan (fCurve_(0, lambda, alpha));
	if (pDock->fAlign == 1)
		theta_ = -theta_;
	//g_print ("theta_ : %.2fdeg\n", theta_);
	
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	ic = pFirstDrawnElement;
	//for (ic = pDock->icons; ic != NULL; ic = ic->next)
	double fOrientationMax = G_PI/2 - atan (my_fParaboleRatio * my_fParaboleCurvature);
	double fTopMargin = pDock->iMaxLabelWidth * sin (fOrientationMax);
	do
	{
		icon = ic->data;
		
		if (ic != pFirstDrawnElement)
		{
			ds = icon->fHeight * (1 - pDock->fFoldingFactor) + g_iIconGap;
			cd_rendering_calculate_next_point (x, y, ds, lambda, alpha, &x_, &y_, &theta_);
			g_print ("ds = %.2f => (%.2f;%.2f), %.2fdeg\n", ds, x_, y_, theta_/G_PI*180);
			
			ds += prev_icon->fWidth / 2 * fabs (tan (theta_ - theta)) * (1 - pDock->fFoldingFactor);
			cd_rendering_calculate_next_point (x, y, ds, lambda, alpha, &x_, &y_, &theta_);
			g_print ("  ds = %.2f => (%.2f;%.2f), %.2fdeg\n", ds, x_, y_, theta_/G_PI*180);
		}
		
		icon->fDrawY = pDock->iCurrentHeight - y_ - icon->fHeight;
		if (pDock->fAlign == 1)
		{
			icon->fDrawX = pDock->iCurrentWidth - pDock->iMaxLabelWidth - x_ - icon->fWidth;
			icon->fOrientation = - theta_;
		}
		else
		{
			icon->fDrawX = x_ + pDock->iMaxLabelWidth + pDock->iMaxIconHeight/2 - icon->fWidth/2;
			icon->fOrientation = theta_;
		}
		icon->fScale = 1.;
		icon->fAlpha = 1;
		icon->fWidthFactor = 1.;
		icon->fHeightFactor = 1.;
		
		x = x_;
		y = y_;
		theta = theta_;
		prev_icon = icon;
		
		//g_print (" * %.2f;%.2f\n", icon->fDrawX, icon->fDrawY);
		if (fXOnCurve > icon->fDrawX && fXOnCurve < icon->fDrawX + icon->fWidth && fYOnCurve > icon->fDrawY && fYOnCurve < icon->fDrawY + icon->fHeight)
		{
			icon->bPointed = TRUE;
			pPointedIcon = icon;
		}
		else
			icon->bPointed = FALSE;
		
		cairo_dock_manage_animations (icon, pDock);
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
	
	return (iMousePositionType == CAIRO_DOCK_MOUSE_INSIDE ? pPointedIcon : NULL);
}


void cd_rendering_register_parabole_renderer (void)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	pRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-parabolic-view", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->calculate_max_dock_size = cd_rendering_calculate_max_dock_size_parabole;
	pRenderer->calculate_icons = cd_rendering_calculate_icons_parabole;  // cairo_dock_apply_no_wave_effect;
	pRenderer->render = cd_rendering_render_parabole;
	pRenderer->render_optimized = NULL;
	pRenderer->set_subdock_position = cd_rendering_set_subdock_position_parabole;
	
	cd_rendering_calculate_reference_parabole (my_fParaboleCurvature);
	
	cairo_dock_register_renderer (MY_APPLET_PARABOLIC_VIEW_NAME, pRenderer);
}
