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

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>

#include "rendering-commons.h"
#include "rendering-3D-plane.h"

extern int iVanishingPointY;

extern CairoDockSpeparatorType my_iDrawSeparator3D;
extern double my_fSeparatorColor[4];

extern cairo_surface_t *my_pFlatSeparatorSurface[2];

#define _define_parameters(hi, h0, H, l, r, gamma, h, w, dw)\
	double hi = myIconsParam.fReflectSize * pDock->container.fRatio + myDocksParam.iFrameMargin;\
	double h0max = (1 + myIconsParam.fAmplitude) * pDock->iMaxIconHeight/** * pDock->container.fRatio*/ + MAX (myIconsParam.iLabelSize, myDocksParam.iFrameMargin + myDocksParam.iDockLineWidth);\
	double h0 = pDock->iMaxIconHeight/** * pDock->container.fRatio*/;\
	double H = iVanishingPointY;\
	double l = myDocksParam.iDockLineWidth;\
	double r = MIN (myDocksParam.iDockRadius, (hi + h0) / 2);\
	double gamma = 0, h, w, dw = 0

static void cd_rendering_calculate_max_dock_size_3D_plane (CairoDock *pDock)
{
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->fFlatDockWidth, pDock->iScrollOffset);
	
	//pDock->iMaxDockHeight = (int) ((1 + myIconsParam.fAmplitude) * pDock->iMaxIconHeight + myIconsParam.fReflectSize * pDock->container.fRatio) + myIconsParam.iLabelSize + myDocksParam.iDockLineWidth + myDocksParam.iFrameMargin;
	
	_define_parameters (hi, h0, H, l, r, gamma, h, w, dw);
	pDock->iMaxDockHeight = (int) (hi + h0max + l);
	// 1ere estimation.
	// w
	w = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., 2 * dw));  // pDock->iMaxDockWidth
	// -> gamma
	gamma = w / 2 / H;
	// -> h
	h = hi + h0 / (1 + gamma);  // en fait, sqrt (1 + gamma * gamma), mais on simplifie pour diminuer l'ordre de 2. pDock->iDecorationsHeight
	// -> dw
	dw = h * gamma + 0*r + (l+(r==0)*2)*sqrt(1+gamma*gamma);  // en fait, h*gamma + r*(1-sin)/cos, or (1-sin)/cos <= 1, on majore pour simplifier. on aurait r + gamma * (h - 2 * r) si on utilisait des cercles au lieu de courbes de Bezier.
	
	r = MIN (myDocksParam.iDockRadius, h / 2);
	dw = r + gamma * (h - r) + (l+(r==0)*2)*sqrt(1+gamma*gamma);  // h-2r si on utilisait des cercles au lieu de courbes de Bezier.
	//g_print ("r:%.1f, h:%.1f => dw=%.1f (%.1f)\n", r, h, dw, h * gamma + 0*r + (l+(r==0)*2)*sqrt(1+gamma*gamma));
	
	/*double Ws = w+2*dw;
	double W = Ws - 2 * (r + (l+(r==0)*2)*sqrt(1+gamma*gamma));
	double a = H + hi;
	double b = H + hi + h0 - W / 2;
	double c = - W / 2;
	double g = (-b + sqrt (b * b - 4 * a * c)) / 2  / a;
	cd_debug ("gamma : %f (=) %f\n", gamma, g);*/
	
	double Ws = cairo_dock_get_max_authorized_dock_width (pDock);
	if (cairo_dock_is_extended_dock (pDock) && w + 2 * dw < Ws)  // alors on etend.
	{
		double extra = Ws - w;
		pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., extra));  // on pourra optimiser, ce qui nous interesse ici c'est les fXMin/fXMax.
		double W = Ws - 2 * (r + (l+(r==0)*2)*sqrt(1+gamma*gamma));
		double a = H + hi;
		double b = H + hi + h0 - W / 2;
		double c = - W / 2;
		gamma = (-b + sqrt (b * b - 4 * a * c)) / 2  / a;
		//g_print ("mode etendu : pDock->iMaxDockWidth : %d, gamma = %f\n", pDock->iMaxDockWidth, gamma);
		h = hi + h0 / (1 + gamma);
	}
	else  // rien d'autre a faire
	{
		pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., 2 * dw));  // on pourra optimiser, ce qui nous interesse ici c'est les fXMin/fXMax.
	}
	pDock->iDecorationsHeight = h;
	//g_print ("h : %.2f -> %d\n", h, pDock->iDecorationsHeight);
	
	pDock->iDecorationsWidth = pDock->iMaxDockWidth;
	
	// taille min.
	pDock->iMinDockHeight = myDocksParam.iDockLineWidth + myDocksParam.iFrameMargin + myIconsParam.fReflectSize * pDock->container.fRatio + pDock->iMaxIconHeight * pDock->container.fRatio;
	
	double gamma_min = pDock->fFlatDockWidth / 2 / H;
	double dw_min = h * gamma_min + r + (l+(r==0)*2)*sqrt(1+gamma_min*gamma_min);
	//cairo_dock_calculate_extra_width_for_trapeze (pDock->iDecorationsHeight, fInclination, myDocksParam.iDockRadius, myDocksParam.iDockLineWidth);
	
	// on charge les separateurs plat.
	//g_print ("%d / %d\n", my_iDrawSeparator3D, myIconsParam.iSeparatorType);
	if (my_iDrawSeparator3D != myIconsParam.iSeparatorType || my_fSeparatorColor[0] != myIconsParam.fSeparatorColor[0] || my_fSeparatorColor[1] != myIconsParam.fSeparatorColor[1] || my_fSeparatorColor[2] != myIconsParam.fSeparatorColor[2] || my_fSeparatorColor[3] != myIconsParam.fSeparatorColor[3])
	{
		my_iDrawSeparator3D = myIconsParam.iSeparatorType;
		memcpy (my_fSeparatorColor, myIconsParam.fSeparatorColor, 4*sizeof(double));
		if (myIconsParam.iSeparatorType == CAIRO_DOCK_FLAT_SEPARATOR)
		{
			cd_rendering_load_flat_separator (CAIRO_CONTAINER (g_pMainDock));
		}
	}
	
	pDock->iMinDockWidth = MAX (1, pDock->fFlatDockWidth);  // fFlatDockWidth peut etre meme negatif avec un dock vide.
}

static void cd_rendering_calculate_construction_parameters_3D_plane (Icon *icon, int iWidth, int iHeight, int iMaxDockWidth, double fReflectionOffsetY)
{
	icon->fDrawX = icon->fX;
	icon->fDrawY = icon->fY + fReflectionOffsetY;
	icon->fWidthFactor = 1.;
	icon->fHeightFactor = 1.;
	///icon->fDeltaYReflection = 0.;
	icon->fOrientation = 0.;
	//if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= iWidth)
	{
		icon->fAlpha = 1;
	}
	/*else
	{
		icon->fAlpha = .25;
	}*/
}


static void cd_rendering_make_3D_separator (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bIncludeEdges, gboolean bBackGround)
{
	double hi = myIconsParam.fReflectSize * pDock->container.fRatio + myDocksParam.iFrameMargin;
	hi = (pDock->container.bDirectionUp ? pDock->container.iHeight - (icon->fDrawY + icon->fHeight * icon->fScale) : icon->fDrawY);
	double fLeftInclination = (icon->fDrawX - pDock->container.iWidth / 2) / iVanishingPointY;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->container.iWidth / 2) / iVanishingPointY;
	
	double fHeight, fBigWidth, fLittleWidth;
	if (bIncludeEdges)
	{
		fHeight = (bBackGround ? pDock->iDecorationsHeight - hi : hi) + 2*myDocksParam.iDockLineWidth;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY : iVanishingPointY + fHeight);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY - fHeight : iVanishingPointY);
	}
	else
	{
		fHeight = pDock->iDecorationsHeight - myDocksParam.iDockLineWidth;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi - fHeight);
	}
	double fDeltaXLeft = fHeight * fLeftInclination;
	double fDeltaXRight = fHeight * fRightInclination;
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	
	double fDockOffsetX, fDockOffsetY;
	if (bIncludeEdges)
	{
		fDockOffsetX = icon->fDrawX - (bBackGround ? fHeight * fLeftInclination : 0);
		fDockOffsetY = pDock->container.iHeight - fHeight - (bBackGround ? myDocksParam.iDockLineWidth + hi : -.5*myDocksParam.iDockLineWidth);
	}
	else
	{
		fDockOffsetX = icon->fDrawX - (fHeight - hi) * fLeftInclination;
		fDockOffsetY = pDock->container.iHeight - fHeight - myDocksParam.iDockLineWidth;
	}	
	
	cairo_translate (pCairoContext, fDockOffsetX, fDockOffsetY);  // coin haut gauche.
	cairo_move_to (pCairoContext, 0, 0);  // coin haut gauche.
	
	cairo_rel_line_to (pCairoContext, fLittleWidth, 0);
	cairo_rel_line_to (pCairoContext, fDeltaXRight, fHeight);
	cairo_rel_line_to (pCairoContext, - fBigWidth, 0);
	cairo_rel_line_to (pCairoContext, - fDeltaXLeft, - fHeight);
	
	if (my_iDrawSeparator3D == CAIRO_DOCK_FLAT_SEPARATOR)
	{
		cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL], MIN (0, (fHeight + hi) * fLeftInclination), 0);
	}
}

static void cd_rendering_draw_3D_separator_edge (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bBackGround)
{
	double hi = myIconsParam.fReflectSize * pDock->container.fRatio + myDocksParam.iFrameMargin;
	hi = (pDock->container.bDirectionUp ? pDock->container.iHeight - (icon->fDrawY + icon->fHeight * icon->fScale) : icon->fDrawY);
	double fLeftInclination = (icon->fDrawX - pDock->container.iWidth / 2) / iVanishingPointY;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->container.iWidth / 2) / iVanishingPointY;
	
	double fHeight, fBigWidth, fLittleWidth;
	fHeight = (bBackGround ? pDock->iDecorationsHeight - hi - 0.5*myDocksParam.iDockLineWidth : hi + 1.5*myDocksParam.iDockLineWidth);
	fBigWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY : iVanishingPointY + fHeight);
	fLittleWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY - fHeight : iVanishingPointY);
	
	double fDeltaXLeft = fHeight * fLeftInclination;
	double fDeltaXRight = fHeight * fRightInclination;
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	
	double fDockOffsetX, fDockOffsetY;
	fDockOffsetY =  (bBackGround ? 2.*myDocksParam.iDockLineWidth : - 1.0*myDocksParam.iDockLineWidth);
	
	fDockOffsetX = (bBackGround ? .5*myDocksParam.iDockLineWidth * fLeftInclination + 1.*fLeftInclination : - 0.5 * myDocksParam.iDockLineWidth * fLeftInclination);
	//fDockOffsetX = -.5*myDocksParam.iDockLineWidth;
	
	
	cairo_translate (pCairoContext, fDockOffsetX, fDockOffsetY);  // coin haut droit.
	
	cairo_move_to (pCairoContext, fLittleWidth, 0);
	cairo_rel_line_to (pCairoContext, fDeltaXRight, fHeight);
	
	cairo_move_to (pCairoContext, 0, 0);
	cairo_rel_line_to (pCairoContext, fDeltaXLeft, fHeight);
}


static void cd_rendering_draw_3D_separator (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bHorizontal, gboolean bBackGround)
{
	if (pDock->container.bIsHorizontal)
	{
		if (! pDock->container.bDirectionUp)
		{
			cairo_translate (pCairoContext, 0., pDock->container.iHeight);
			cairo_scale (pCairoContext, 1., -1.);
		}
	}
	else
	{
		cairo_translate (pCairoContext, pDock->container.iHeight/2, pDock->container.iWidth/2);
		cairo_rotate (pCairoContext, G_PI/2);
		cairo_translate (pCairoContext, -pDock->container.iWidth/2, -pDock->container.iHeight/2);
		if (pDock->container.bDirectionUp)
		{
			cairo_translate (pCairoContext, 0., pDock->container.iHeight);
			cairo_scale (pCairoContext, 1., -1.);
		}
	}
	cd_rendering_make_3D_separator (icon, pCairoContext, pDock, (my_iDrawSeparator3D == CAIRO_DOCK_PHYSICAL_SEPARATOR), bBackGround);
	
	if (my_iDrawSeparator3D == CAIRO_DOCK_PHYSICAL_SEPARATOR)
	{
		cairo_set_operator (pCairoContext, CAIRO_OPERATOR_DEST_OUT);
		cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 1.0);
		cairo_fill (pCairoContext);
		
		if (myDocksParam.iDockLineWidth != 0)
		{
			cd_rendering_draw_3D_separator_edge (icon, pCairoContext, pDock, bBackGround);
			
			cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
			cairo_set_line_width (pCairoContext, myDocksParam.iDockLineWidth);
			cairo_set_source_rgba (pCairoContext, myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
			cairo_stroke (pCairoContext);
		}
	}
	else
	{
		cairo_fill (pCairoContext);
	}
}


static void cd_rendering_render_3D_plane (cairo_t *pCairoContext, CairoDock *pDock)
{
	_define_parameters (hi, h0, H, l, r, gamma, h, w, dw);
	h = pDock->iDecorationsHeight;
	if (h < 2 * r)
		r = h / 2;
	
	//\____________________ On definit la position du cadre.
	double dx, dy;  // position du coin haut gauche du cadre.
	if (cairo_dock_is_extended_dock (pDock))  // mode panel etendu.
	{
		double Ws = pDock->container.iWidth;
		gamma = Ws / 2 / H;
		double W = Ws - 2 * (r + (l+(r==0)*2)*sqrt(1+gamma*gamma));
		double a = H + hi;
		double b = H + hi + h0 - W / 2;
		double c = - W / 2;
		gamma = (-b + sqrt (b * b - 4 * a * c)) / 2  / a;
		h = hi + h0 / (1 + gamma);
		//g_print ("h : %.2f (=) %d\n", h, pDock->iDecorationsHeight);
		w = 2 * H * gamma;
		dw = (Ws - w) / 2;
		//g_print ("dw : %.2f (=) %.2f\n", dw, h * gamma + r + (l+(r==0)*2)*sqrt(1+gamma*gamma));
		dx = dw;
	}
	else
	{
		w = cairo_dock_get_current_dock_width_linear (pDock);
		gamma = w / 2 / H;
		dw = h * gamma + r + (l+(r==0)*2)*sqrt(1+gamma*gamma);
		h = pDock->iDecorationsHeight;
		Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
		dx = (pFirstIcon != NULL ? pFirstIcon->fX - 0*myDocksParam.iFrameMargin : r);
	}
	
	int sens;
	if (pDock->container.bDirectionUp)
	{
		sens = 1;
		dy = pDock->container.iHeight - pDock->iDecorationsHeight - 1.5 * l;
	}
	else
	{
		sens = -1;
		dy = pDock->iDecorationsHeight + 1.5 * l;
	}
	
	//\____________________ On trace le cadre.
	cairo_save (pCairoContext);
	
	double fDeltaXTrapeze = cairo_dock_draw_frame (pCairoContext, r, l, w, pDock->iDecorationsHeight, dx, dy, sens, gamma, pDock->container.bIsHorizontal, myDocksParam.bRoundedBottomCorner);
	
	//\____________________ On dessine les decorations dedans.
	dy = (pDock->container.bDirectionUp ? pDock->container.iHeight - pDock->iDecorationsHeight - l : l);
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, dy, dx-fDeltaXTrapeze, w+2*fDeltaXTrapeze);
	
	//\____________________ On dessine le cadre.
	if (l > 0)
	{
		cairo_set_line_width (pCairoContext, l);
		cairo_set_source_rgba (pCairoContext, myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
		cairo_stroke (pCairoContext);
	}
	else
		cairo_new_path (pCairoContext);
	
	/// donner un effet d'epaisseur => chaud du slip avec les separateurs physiques !
	
	
	cairo_restore (pCairoContext);
	
	//\____________________ On dessine la ficelle qui les joint.
	if (myIconsParam.iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, myIconsParam.iStringLineWidth, FALSE, (my_iDrawSeparator3D == CAIRO_DOCK_FLAT_SEPARATOR || my_iDrawSeparator3D == CAIRO_DOCK_PHYSICAL_SEPARATOR));
	
	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
		
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	if (my_iDrawSeparator3D == CAIRO_DOCK_FLAT_SEPARATOR || my_iDrawSeparator3D == CAIRO_DOCK_PHYSICAL_SEPARATOR)
	{
		cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_SQUARE);
		do
		{
			icon = ic->data;
			
			if (icon->cFileName == NULL && CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
			{
				cairo_save (pCairoContext);
				cd_rendering_draw_3D_separator (icon, pCairoContext, pDock, pDock->container.bIsHorizontal, TRUE);
				cairo_restore (pCairoContext);
			}
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
		
		do
		{
			icon = ic->data;
			
			if (icon->cFileName != NULL || ! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
			{
				cairo_save (pCairoContext);
				cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
				cairo_restore (pCairoContext);
			}
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
		
		if (my_iDrawSeparator3D == CAIRO_DOCK_PHYSICAL_SEPARATOR)
		{
			do
			{
				icon = ic->data;
				
				if (icon->cFileName == NULL && CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
				{
					cairo_save (pCairoContext);
					cd_rendering_draw_3D_separator (icon, pCairoContext, pDock, pDock->container.bIsHorizontal, FALSE);
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
	double hi = myIconsParam.fReflectSize * pDock->container.fRatio + myDocksParam.iFrameMargin;
	hi = (pDock->container.bDirectionUp ? pDock->container.iHeight - (icon->fDrawY + icon->fHeight * icon->fScale) : icon->fDrawY);
	double fLeftInclination = fabs (icon->fDrawX - pDock->container.iWidth / 2) / iVanishingPointY;
	double fRightInclination = fabs (icon->fDrawX + icon->fWidth * icon->fScale - pDock->container.iWidth / 2) / iVanishingPointY;
	
	double fHeight, fBigWidth, fLittleWidth;
	if (bIncludeEdges)
	{
		fHeight = (bBackGround ? pDock->iDecorationsHeight - hi : hi) + (bIncludeEdges ? myDocksParam.iDockLineWidth : 0);
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
	if (pDock->container.bDirectionUp)
	{
		sens = 1;
		if (bIncludeEdges)
			fDockOffsetY =  pDock->container.iHeight - fHeight - (bBackGround ? myDocksParam.iDockLineWidth + hi : 0);
		else
			fDockOffsetY =  pDock->container.iHeight - fHeight;
	}
	else
	{
		sens = -1;
		if (bIncludeEdges)
			fDockOffsetY = fHeight + (bBackGround ? myDocksParam.iDockLineWidth + hi : 0);
		else
			fDockOffsetY = fHeight;
	}
	
	if (bIncludeEdges)
			fDockOffsetX = icon->fDrawX - (bBackGround ? fHeight * fLeftInclination : 0);
		else
			fDockOffsetX = icon->fDrawX - (fHeight - hi) * fLeftInclination;
	double fXLeft, fXRight;
	if (icon->fDrawX + icon->fWidth * icon->fScale / 2 > pDock->container.iWidth / 2)  // on est a droite.
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

static void cd_rendering_render_optimized_3D_plane (cairo_t *pCairoContext, CairoDock *pDock, GdkRectangle *pArea)
{
	//g_print ("%s ((%d;%d) x (%d;%d) / (%dx%d))\n", __func__, pArea->x, pArea->y, pArea->width, pArea->height, pDock->container.iWidth, pDock->container.iHeight);
	double fLineWidth = myDocksParam.iDockLineWidth;
	double fMargin = myDocksParam.iFrameMargin;
	int iWidth = pDock->container.iWidth;
	int iHeight = pDock->container.iHeight;
	
	//\____________________ On dessine les decorations du fond sur la portion de fenetre.
	cairo_save (pCairoContext);
	
	double fDockOffsetX, fDockOffsetY;
	if (pDock->container.bIsHorizontal)
	{
		fDockOffsetX = pArea->x;
		fDockOffsetY = (pDock->container.bDirectionUp ? iHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	}
	else
	{
		fDockOffsetX = (pDock->container.bDirectionUp ? iHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
		fDockOffsetY = pArea->y;
	}
	
	//cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	if (pDock->container.bIsHorizontal)
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pArea->width, pDock->iDecorationsHeight);
	else
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pDock->iDecorationsHeight, pArea->height);
	
	double fRadius = MIN (myDocksParam.iDockRadius, (pDock->iDecorationsHeight + myDocksParam.iDockLineWidth) / 2 - 1);
	double fDeltaXTrapeze=0.;
	double fOffsetX;
	if (cairo_dock_is_extended_dock (pDock))  // mode panel etendu.
	{
		fOffsetX = fRadius + fLineWidth / 2;
	}
	else
	{
		Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
		fOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX - fMargin : fRadius + fLineWidth / 2);
	}
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock);
	if (pDock->backgroundBuffer.pSurface != NULL)
	{
		double fInclinationOnHorizon = (fDockWidth / 2) / iVanishingPointY;
		double fRadius = myDocksParam.iDockRadius;
		if (2*fRadius > pDock->iDecorationsHeight + fLineWidth)
			fRadius = (pDock->iDecorationsHeight + fLineWidth) / 2 - 1;
		double fDeltaXForLoop = fInclinationOnHorizon * (pDock->iDecorationsHeight + fLineWidth - (/**myDocksParam.bRoundedBottomCorner*/TRUE ? 2 : 1) * fRadius);
		
		double cosa = 1. / sqrt (1 + fInclinationOnHorizon * fInclinationOnHorizon);
		fDeltaXTrapeze = fDeltaXForLoop + fRadius * cosa;
		
		double sina = cosa * fInclinationOnHorizon;
		fDeltaXTrapeze = fInclinationOnHorizon * (pDock->iDecorationsHeight - (FALSE ? 2 : 1-sina) * fRadius) + fRadius * (FALSE ? 1 : cosa);
	}
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, pDock->container.bIsHorizontal ? fDockOffsetY : fDockOffsetX, fOffsetX-fDeltaXTrapeze, fDockWidth+2*fDeltaXTrapeze);
	
	
	//\____________________ On dessine la partie du cadre qui va bien.
	cairo_new_path (pCairoContext);
	
	if (pDock->container.bIsHorizontal)
	{
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY - 0.5*fLineWidth);
		cairo_rel_line_to (pCairoContext, pArea->width, 0);
		cairo_set_source_rgba (pCairoContext, myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
		cairo_stroke (pCairoContext);
		
		cairo_new_path (pCairoContext);
		cairo_move_to (pCairoContext, fDockOffsetX, (pDock->container.bDirectionUp ? iHeight - 0.5*fLineWidth : pDock->iDecorationsHeight + 1.5 * fLineWidth));
		cairo_rel_line_to (pCairoContext, pArea->width, 0);
	}
	else
	{
		cairo_move_to (pCairoContext, fDockOffsetX - .5*fLineWidth, fDockOffsetY);
		cairo_rel_line_to (pCairoContext, 0, pArea->height);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
		cairo_stroke (pCairoContext);
		
		cairo_new_path (pCairoContext);
		cairo_move_to (pCairoContext, (pDock->container.bDirectionUp ? iHeight - fLineWidth / 2 : pDock->iDecorationsHeight + 1.5 * fLineWidth), fDockOffsetY);
		cairo_rel_line_to (pCairoContext, 0, pArea->height);
	}
	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_set_source_rgba (pCairoContext, myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
	cairo_stroke (pCairoContext);
	
	cairo_restore (pCairoContext);
	
	//\____________________ On dessine les icones impactees.
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement != NULL)
	{
		double fXMin = (pDock->container.bIsHorizontal ? pArea->x : pArea->y), fXMax = (pDock->container.bIsHorizontal ? pArea->x + pArea->width : pArea->y + pArea->height);
		double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
		double fXLeft, fXRight;
		Icon *icon;
		GList *ic = pFirstDrawnElement;
		
		if (my_iDrawSeparator3D == CAIRO_DOCK_FLAT_SEPARATOR || my_iDrawSeparator3D == CAIRO_DOCK_PHYSICAL_SEPARATOR)
		{
			cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_SQUARE);
			do
			{
				icon = ic->data;
				
				if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon) && icon->cFileName == NULL)
				{
					if (_cd_separator_is_impacted (icon, pDock, fXMin, fXMax, TRUE, (my_iDrawSeparator3D == CAIRO_DOCK_PHYSICAL_SEPARATOR)))
					{
						cairo_save (pCairoContext);
						cd_rendering_draw_3D_separator (icon, pCairoContext, pDock, pDock->container.bIsHorizontal, TRUE);
						cairo_restore (pCairoContext);
					}
				}
				
				ic = cairo_dock_get_next_element (ic, pDock->icons);
			} while (ic != pFirstDrawnElement);
			
			do
			{
				icon = ic->data;
				if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon) || icon->cFileName != NULL)
				{
					fXLeft = icon->fDrawX + icon->fScale + 1;
					fXRight = icon->fDrawX + (icon->fWidth - 1) * icon->fScale * icon->fWidthFactor - 1;
					
					if (fXLeft <= fXMax && floor (fXRight) > fXMin)
					{
						//if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= pDock->container.iWidth)
							icon->fAlpha = 1;
						//else
						//	icon->fAlpha = .25;
						
						cairo_save (pCairoContext);
						cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
						cairo_restore (pCairoContext);
					}
				}
				ic = cairo_dock_get_next_element (ic, pDock->icons);
			} while (ic != pFirstDrawnElement);
			
			if (my_iDrawSeparator3D == CAIRO_DOCK_PHYSICAL_SEPARATOR)
			{
				do
				{
					icon = ic->data;
					
					if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon) && icon->cFileName == NULL)
					{
						if (_cd_separator_is_impacted (icon, pDock, fXMin, fXMax, FALSE, (my_iDrawSeparator3D == CAIRO_DOCK_PHYSICAL_SEPARATOR)))
						{
							cairo_save (pCairoContext);
							cd_rendering_draw_3D_separator (icon, pCairoContext, pDock, pDock->container.bIsHorizontal, FALSE);
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
					//if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= pDock->container.iWidth)
						icon->fAlpha = 1;
					//else
					//	icon->fAlpha = .25;
					
					cairo_save (pCairoContext);
					cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
					cairo_restore (pCairoContext);
				}
				ic = cairo_dock_get_next_element (ic, pDock->icons);
			} while (ic != pFirstDrawnElement);
		}
	}
}


static Icon *cd_rendering_calculate_icons_3D_plane (CairoDock *pDock)
{
	Icon *pPointedIcon = cairo_dock_apply_wave_effect (pDock);
	
	//\____________________ On calcule les position/etirements/alpha des icones.
	double fReflectionOffsetY = (pDock->container.bDirectionUp ? -1 : 1) * myIconsParam.fReflectSize * pDock->container.fRatio;
	Icon* icon;
	GList* ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cd_rendering_calculate_construction_parameters_3D_plane (icon, pDock->container.iWidth, pDock->container.iHeight, pDock->iMaxDockWidth, fReflectionOffsetY);
	}
	
	cairo_dock_check_if_mouse_inside_linear (pDock);
	
	cairo_dock_check_can_drop_linear (pDock);
	
	return pPointedIcon;
}



static void cd_rendering_render_3D_plane_opengl (CairoDock *pDock)
{
	//\_____________ On definit notre cadre.
	_define_parameters (hi, h0, H, l, r, gamma, h, w, dw);
	h = pDock->iDecorationsHeight;
	if (h < 2 * r)
		r = h / 2;
	
	double dx, dy;
	if (cairo_dock_is_extended_dock (pDock))  // mode panel etendu.
	{
		double Ws = pDock->container.iWidth;
		double W = Ws - 2 * r;
		double a = H + hi;
		double b = H + hi + h0 - W / 2;
		double c = - W / 2;
		gamma = (-b + sqrt (b * b - 4 * a * c)) / 2  / a;
		h = hi + h0 / (1 + gamma);
		//g_print ("h : %.2f (=) %d\n", h, pDock->iDecorationsHeight);
		w = 2 * H * gamma;
		dw = (Ws - w) / 2;
		//g_print ("dw : %.2f (=) %.2f\n", dw, h * gamma + r + (l+(r==0)*2)*sqrt(1+gamma*gamma));
		dx = dw;
	}
	else
	{
		w = cairo_dock_get_current_dock_width_linear (pDock);
		gamma = w / 2 / H;
		dw = h * gamma + r + (l+(r==0)*2)*sqrt(1+gamma*gamma);
		h = pDock->iDecorationsHeight;
		Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
		dx = (pFirstIcon != NULL ? pFirstIcon->fX - myDocksParam.iFrameMargin : r);
	}
	
	//\_____________ On genere les coordonnees du contour.
	double fDeltaXTrapeze;
	const CairoDockGLPath *pFramePath = cairo_dock_generate_trapeze_path (w - (/**myDocksParam.bRoundedBottomCorner*/TRUE ? 0 : 2*l/gamma), h+l, r, /**myDocksParam.bRoundedBottomCorner*/TRUE, gamma, &fDeltaXTrapeze);
	dx = dx - fDeltaXTrapeze;
	dy = pDock->iDecorationsHeight + 1.5*l;
	
	//\_____________ On remplit avec le fond.
	glPushMatrix ();
	cairo_dock_set_container_orientation_opengl (CAIRO_CONTAINER (pDock));
	glTranslatef (dx + (w+2*fDeltaXTrapeze)/2,
		dy - h/2,
		0.);
	
	cairo_dock_fill_gl_path (pFramePath, pDock->backgroundBuffer.iTexture);
	
	//\_____________ On trace le contour.
	if (l != 0)
	{
		glLineWidth (l);
		glColor4f (myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
		_cairo_dock_set_blend_alpha ();
		cairo_dock_stroke_gl_path (pFramePath, TRUE);
	}
	glPopMatrix ();
	
	//\____________________ On dessine la ficelle qui les joint.
	if (myIconsParam.iStringLineWidth > 0)
		cairo_dock_draw_string_opengl (pDock, myIconsParam.iStringLineWidth, FALSE, (my_iDrawSeparator3D == CAIRO_DOCK_FLAT_SEPARATOR || my_iDrawSeparator3D == CAIRO_DOCK_PHYSICAL_SEPARATOR));
	
	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
		
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	
	///glLoadIdentity ();
 	if (my_iDrawSeparator3D == CAIRO_DOCK_FLAT_SEPARATOR || my_iDrawSeparator3D == CAIRO_DOCK_PHYSICAL_SEPARATOR)
	{
		do
		{
			icon = ic->data;
			
			if (icon->cFileName == NULL && CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
			{
				glPushMatrix ();
				if (my_iDrawSeparator3D == CAIRO_DOCK_FLAT_SEPARATOR)
					cd_rendering_draw_flat_separator_opengl (icon, pDock);
				else
					cd_rendering_draw_physical_separator_opengl (icon, pDock, TRUE, NULL, NULL);
				glPopMatrix ();
			}
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
		
		do
		{
			icon = ic->data;
			
			if (icon->cFileName != NULL || ! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
			{
				glPushMatrix ();
				cairo_dock_render_one_icon_opengl (icon, pDock, fDockMagnitude, TRUE);
				glPopMatrix ();
			}
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
		
		if (my_iDrawSeparator3D == CAIRO_DOCK_PHYSICAL_SEPARATOR)
		{
			do
			{
				icon = ic->data;
				
				if (icon->cFileName == NULL && CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
				{
					glPushMatrix ();
					cd_rendering_draw_physical_separator_opengl (icon, pDock, FALSE, NULL, NULL);
					glPopMatrix ();
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


void cd_rendering_register_3D_plane_renderer (const gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	// interface
	pRenderer->compute_size = cd_rendering_calculate_max_dock_size_3D_plane;
	pRenderer->calculate_icons = cd_rendering_calculate_icons_3D_plane;
	pRenderer->render = cd_rendering_render_3D_plane;
	pRenderer->render_optimized = cd_rendering_render_optimized_3D_plane;
	pRenderer->render_opengl = cd_rendering_render_3D_plane_opengl;
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;
	// parametres
	pRenderer->bUseReflect = TRUE;
	pRenderer->cDisplayedName = D_ (cRendererName);
	pRenderer->cReadmeFilePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/readme-3D-plane-view");
	pRenderer->cPreviewFilePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/preview-3D-plane.jpg");
	
	cairo_dock_register_renderer (cRendererName, pRenderer);
}
