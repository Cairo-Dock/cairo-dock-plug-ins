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

extern CDSpeparatorType my_iDrawSeparator3D;
extern double my_fSeparatorColor[4];

extern cairo_surface_t *my_pFlatSeparatorSurface[2];
extern GLuint my_iFlatSeparatorTexture;

#define _define_parameters(hi, h0, H, l, r, gamma, h, w, dw)\
	double hi = myIcons.fReflectSize * pDock->container.fRatio + myBackground.iFrameMargin;\
	double h0max = (1 + g_fAmplitude) * pDock->iMaxIconHeight/** * pDock->container.fRatio*/ + MAX (myLabels.iLabelSize, myBackground.iFrameMargin + myBackground.iDockLineWidth);\
	double h0 = pDock->iMaxIconHeight/** * pDock->container.fRatio*/;\
	double H = iVanishingPointY;\
	double l = myBackground.iDockLineWidth;\
	double r = MIN (myBackground.iDockRadius, (hi + h0) / 2);\
	double gamma = 0, h, w, dw = 0

void cd_rendering_calculate_max_dock_size_3D_plane (CairoDock *pDock)
{
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->fFlatDockWidth, pDock->iScrollOffset);
	
	//pDock->iMaxDockHeight = (int) ((1 + g_fAmplitude) * pDock->iMaxIconHeight + myIcons.fReflectSize * pDock->container.fRatio) + myLabels.iLabelSize + myBackground.iDockLineWidth + myBackground.iFrameMargin;
	
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
	
	r = MIN (myBackground.iDockRadius, h / 2);
	dw = r + gamma * (h - r) + (l+(r==0)*2)*sqrt(1+gamma*gamma);  // h-2r si on utilisait des cercles au lieu de courbes de Bezier.
	//g_print ("r:%.1f, h:%.1f => dw=%.1f (%.1f)\n", r, h, dw, h * gamma + 0*r + (l+(r==0)*2)*sqrt(1+gamma*gamma));
	
	/*double Ws = w+2*dw;
	double W = Ws - 2 * (r + (l+(r==0)*2)*sqrt(1+gamma*gamma));
	double a = H + hi;
	double b = H + hi + h0 - W / 2;
	double c = - W / 2;
	double g = (-b + sqrt (b * b - 4 * a * c)) / 2  / a;
	g_print ("gamma : %f (=) %f\n", gamma, g);*/
	
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
	pDock->iMinDockHeight = myBackground.iDockLineWidth + myBackground.iFrameMargin + myIcons.fReflectSize * pDock->container.fRatio + pDock->iMaxIconHeight * pDock->container.fRatio;
	
	double gamma_min = pDock->fFlatDockWidth / 2 / H;
	double dw_min = h * gamma_min + r + (l+(r==0)*2)*sqrt(1+gamma_min*gamma_min);
	//cairo_dock_calculate_extra_width_for_trapeze (pDock->iDecorationsHeight, fInclination, myBackground.iDockRadius, myBackground.iDockLineWidth);
	
	// on charge les separateurs plat.
	//g_print ("%d / %d\n", my_iDrawSeparator3D, myIcons.iSeparatorType);
	if (my_iDrawSeparator3D != myIcons.iSeparatorType || my_fSeparatorColor[0] != myIcons.fSeparatorColor[0] || my_fSeparatorColor[1] != myIcons.fSeparatorColor[1] || my_fSeparatorColor[2] != myIcons.fSeparatorColor[2] || my_fSeparatorColor[3] != myIcons.fSeparatorColor[3])
	{
		my_iDrawSeparator3D = myIcons.iSeparatorType;
		memcpy (my_fSeparatorColor, myIcons.fSeparatorColor, 4*sizeof(double));
		if (myIcons.iSeparatorType == CD_FLAT_SEPARATOR)
		{
			cd_rendering_load_flat_separator (CAIRO_CONTAINER (g_pMainDock));
		}
	}
	
	pDock->iMinDockWidth = pDock->fFlatDockWidth;
}

void cd_rendering_calculate_construction_parameters_3D_plane (Icon *icon, int iWidth, int iHeight, int iMaxDockWidth, double fReflectionOffsetY)
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
	gboolean bDirectionUp = pDock->container.bDirectionUp;
	gboolean bIsHorizontal = pDock->container.bIsHorizontal;
	bDirectionUp = TRUE;
	bIsHorizontal = TRUE;
	double hi = myIcons.fReflectSize * pDock->container.fRatio + myBackground.iFrameMargin;
	hi = (pDock->container.bDirectionUp ? pDock->container.iHeight - (icon->fDrawY + icon->fHeight * icon->fScale) : icon->fDrawY);
	double fLeftInclination = (icon->fDrawX - pDock->container.iWidth / 2) / iVanishingPointY;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->container.iWidth / 2) / iVanishingPointY;
	
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
	if (bDirectionUp)
	{
		sens = 1;
		if (bIncludeEdges)
			fDockOffsetY = pDock->container.iHeight - fHeight - (bBackGround ? myBackground.iDockLineWidth + hi : 0);
		else
			fDockOffsetY = pDock->container.iHeight - fHeight - myBackground.iDockLineWidth;
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
		fDockOffsetX = icon->fDrawX - (fHeight - hi) * fLeftInclination;
	
	if (bIsHorizontal)
	{
		cairo_translate (pCairoContext, fDockOffsetX, fDockOffsetY);  // coin haut gauche.
		cairo_move_to (pCairoContext, 0, 0);  // coin haut gauche.
		
		cairo_rel_line_to (pCairoContext, fLittleWidth, 0);
		cairo_rel_line_to (pCairoContext, fDeltaXRight, sens * fHeight);
		cairo_rel_line_to (pCairoContext, - fBigWidth, 0);
		cairo_rel_line_to (pCairoContext, - fDeltaXLeft, - sens * fHeight);
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! bDirectionUp)
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
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! bDirectionUp)
				cairo_scale (pCairoContext, -1, 1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL], 0, MIN (0, (fHeight + hi) * fLeftInclination));
		}
	}
}

static void cd_rendering_draw_3D_separator_edge (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bBackGround)
{
	gboolean bDirectionUp = pDock->container.bDirectionUp;
	gboolean bIsHorizontal = pDock->container.bIsHorizontal;
	bDirectionUp = TRUE;
	bIsHorizontal = TRUE;
	double hi = myIcons.fReflectSize * pDock->container.fRatio + myBackground.iFrameMargin;
	hi = (pDock->container.bDirectionUp ? pDock->container.iHeight - (icon->fDrawY + icon->fHeight * icon->fScale) : icon->fDrawY);
	double fLeftInclination = (icon->fDrawX - pDock->container.iWidth / 2) / iVanishingPointY;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->container.iWidth / 2) / iVanishingPointY;
	
	double fHeight, fBigWidth, fLittleWidth;
	fHeight = (bBackGround ? pDock->iDecorationsHeight - hi - 0.5*myBackground.iDockLineWidth : hi + 1.5*myBackground.iDockLineWidth);
	fBigWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY : iVanishingPointY + fHeight);
	fLittleWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY - fHeight : iVanishingPointY);
	
	double fDeltaXLeft = fHeight * fLeftInclination;
	double fDeltaXRight = fHeight * fRightInclination;
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	
	int sens;
	double fDockOffsetX, fDockOffsetY;
	if (bDirectionUp)
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
	
	if (bIsHorizontal)
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
		if (pDock->container.bDirectionUp)
		{
			
		}
		else
		{
			
		}
	}
	cd_rendering_make_3D_separator (icon, pCairoContext, pDock, (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR), bBackGround);
	
	if (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
	{
		cairo_set_operator (pCairoContext, CAIRO_OPERATOR_DEST_OUT);
		cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 1.0);
		cairo_fill (pCairoContext);
		
		if (myBackground.iDockLineWidth != 0)
		{
			cd_rendering_draw_3D_separator_edge (icon, pCairoContext, pDock, bBackGround);
			
			cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
			cairo_set_line_width (pCairoContext, myBackground.iDockLineWidth);
			cairo_set_source_rgba (pCairoContext, myBackground.fLineColor[0], myBackground.fLineColor[1], myBackground.fLineColor[2], myBackground.fLineColor[3]);
			cairo_stroke (pCairoContext);
		}
	}
	else
	{
		cairo_fill (pCairoContext);
	}
}


void cd_rendering_render_3D_plane (cairo_t *pCairoContext, CairoDock *pDock)
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
		dx = (pFirstIcon != NULL ? pFirstIcon->fX - 0*myBackground.iFrameMargin : r);
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
	
	double fDeltaXTrapeze = cairo_dock_draw_frame (pCairoContext, r, l, w, pDock->iDecorationsHeight, dx, dy, sens, gamma, pDock->container.bIsHorizontal);
	
	//\____________________ On dessine les decorations dedans.
	dy = (pDock->container.bDirectionUp ? pDock->container.iHeight - pDock->iDecorationsHeight - l : l);
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, dy, dx-fDeltaXTrapeze, w+2*fDeltaXTrapeze);
	
	//\____________________ On dessine le cadre.
	if (l > 0)
	{
		cairo_set_line_width (pCairoContext, l);
		cairo_set_source_rgba (pCairoContext, myBackground.fLineColor[0], myBackground.fLineColor[1], myBackground.fLineColor[2], myBackground.fLineColor[3]);
		cairo_stroke (pCairoContext);
	}
	else
		cairo_new_path (pCairoContext);
	
	/// donner un effet d'epaisseur => chaud du slip avec les separateurs physiques !
	
	
	cairo_restore (pCairoContext);
	
	//\____________________ On dessine la ficelle qui les joint.
	if (myIcons.iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, myIcons.iStringLineWidth, FALSE, (my_iDrawSeparator3D == CD_FLAT_SEPARATOR || my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR));
	
	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
		
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR || my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
	{
		cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_SQUARE);
		do
		{
			icon = ic->data;
			
			if (icon->cFileName == NULL && CAIRO_DOCK_IS_SEPARATOR (icon))
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
			
			if (icon->cFileName != NULL || ! CAIRO_DOCK_IS_SEPARATOR (icon))
			{
				cairo_save (pCairoContext);
				cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
				cairo_restore (pCairoContext);
			}
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
		
		if (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
		{
			do
			{
				icon = ic->data;
				
				if (icon->cFileName == NULL && CAIRO_DOCK_IS_SEPARATOR (icon))
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
	double hi = myIcons.fReflectSize * pDock->container.fRatio + myBackground.iFrameMargin;
	hi = (pDock->container.bDirectionUp ? pDock->container.iHeight - (icon->fDrawY + icon->fHeight * icon->fScale) : icon->fDrawY);
	double fLeftInclination = fabs (icon->fDrawX - pDock->container.iWidth / 2) / iVanishingPointY;
	double fRightInclination = fabs (icon->fDrawX + icon->fWidth * icon->fScale - pDock->container.iWidth / 2) / iVanishingPointY;
	
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
	if (pDock->container.bDirectionUp)
	{
		sens = 1;
		if (bIncludeEdges)
			fDockOffsetY =  pDock->container.iHeight - fHeight - (bBackGround ? myBackground.iDockLineWidth + hi : 0);
		else
			fDockOffsetY =  pDock->container.iHeight - fHeight;
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

void cd_rendering_render_optimized_3D_plane (cairo_t *pCairoContext, CairoDock *pDock, GdkRectangle *pArea)
{
	//g_print ("%s ((%d;%d) x (%d;%d) / (%dx%d))\n", __func__, pArea->x, pArea->y, pArea->width, pArea->height, pDock->container.iWidth, pDock->container.iHeight);
	double fLineWidth = myBackground.iDockLineWidth;
	double fMargin = myBackground.iFrameMargin;
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
	
	double fRadius = MIN (myBackground.iDockRadius, (pDock->iDecorationsHeight + myBackground.iDockLineWidth) / 2 - 1);
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
	if (g_pBackgroundSurface != NULL)
	{
		double fInclinationOnHorizon = (fDockWidth / 2) / iVanishingPointY;
		double fRadius = myBackground.iDockRadius;
		if (2*fRadius > pDock->iDecorationsHeight + fLineWidth)
			fRadius = (pDock->iDecorationsHeight + fLineWidth) / 2 - 1;
		double fDeltaXForLoop = fInclinationOnHorizon * (pDock->iDecorationsHeight + fLineWidth - (/**myBackground.bRoundedBottomCorner*/TRUE ? 2 : 1) * fRadius);
		
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
		cairo_set_source_rgba (pCairoContext, myBackground.fLineColor[0], myBackground.fLineColor[1], myBackground.fLineColor[2], myBackground.fLineColor[3]);
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
		cairo_set_source_rgba (pCairoContext, myBackground.fLineColor[0], myBackground.fLineColor[1], myBackground.fLineColor[2], myBackground.fLineColor[3]);
		cairo_stroke (pCairoContext);
		
		cairo_new_path (pCairoContext);
		cairo_move_to (pCairoContext, (pDock->container.bDirectionUp ? iHeight - fLineWidth / 2 : pDock->iDecorationsHeight + 1.5 * fLineWidth), fDockOffsetY);
		cairo_rel_line_to (pCairoContext, 0, pArea->height);
	}
	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_set_source_rgba (pCairoContext, myBackground.fLineColor[0], myBackground.fLineColor[1], myBackground.fLineColor[2], myBackground.fLineColor[3]);
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
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR || my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
		{
			cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_SQUARE);
			do
			{
				icon = ic->data;
				
				if (CAIRO_DOCK_IS_SEPARATOR (icon) && icon->cFileName == NULL)
				{
					if (_cd_separator_is_impacted (icon, pDock, fXMin, fXMax, TRUE, (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)))
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
				if (! CAIRO_DOCK_IS_SEPARATOR (icon) || icon->cFileName != NULL)
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
			
			if (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
			{
				do
				{
					icon = ic->data;
					
					if (CAIRO_DOCK_IS_SEPARATOR (icon) && icon->cFileName == NULL)
					{
						if (_cd_separator_is_impacted (icon, pDock, fXMin, fXMax, FALSE, (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)))
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


Icon *cd_rendering_calculate_icons_3D_plane (CairoDock *pDock)
{
	Icon *pPointedIcon = cairo_dock_apply_wave_effect (pDock);
	
	//\____________________ On calcule les position/etirements/alpha des icones.
	double fReflectionOffsetY = (pDock->container.bDirectionUp ? -1 : 1) * myIcons.fReflectSize * pDock->container.fRatio;
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


void cd_rendering_render_3D_plane_opengl (CairoDock *pDock)
{
	//\____________________ On genere le cadre.
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
		dx = (pFirstIcon != NULL ? pFirstIcon->fX - myBackground.iFrameMargin : r);
	}
	
	//\____________________ On trace le cadre.
	int sens;
	if ((pDock->container.bDirectionUp && pDock->container.bIsHorizontal) || (!pDock->container.bDirectionUp && !pDock->container.bIsHorizontal))
	{
		sens = 1;
		//dy = pDock->container.iHeight - pDock->iDecorationsHeight - 1.5 * l;
		dy = pDock->iDecorationsHeight + 1.5*l;
	}
	else
	{
		sens = -1;
		//dy = pDock->iDecorationsHeight + 1.5 * l;
		dy = pDock->container.iHeight - .5 * l;
	}
	
	int iNbVertex;
	double fDeltaXTrapeze;
	GLfloat *pVertexTab = cairo_dock_generate_trapeze_path (w - (/**myBackground.bRoundedBottomCorner*/TRUE ? 0 : 2*l/gamma), h+l, r, /**myBackground.bRoundedBottomCorner*/TRUE, gamma, &fDeltaXTrapeze, &iNbVertex);
	
	if (! pDock->container.bIsHorizontal)
		dx = pDock->container.iWidth - dx + fDeltaXTrapeze;
	else
		dx = dx - fDeltaXTrapeze;
	
	//\____________________ On dessine les decorations dedans.
	//fDockOffsetY = (!pDock->container.bDirectionUp ? pDock->container.iHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	glPushMatrix ();
	cairo_dock_draw_frame_background_opengl (g_iBackgroundTexture, w+2*fDeltaXTrapeze, h+l, dx, dy, pVertexTab, iNbVertex, pDock->container.bIsHorizontal, pDock->container.bDirectionUp, pDock->fDecorationsOffsetX);
	
	//\____________________ On dessine le cadre.
	if (l != 0)
		cairo_dock_draw_current_path_opengl (l, myBackground.fLineColor, iNbVertex);
	glPopMatrix ();
	
	/// donner un effet d'epaisseur => chaud du slip avec les separateurs physiques !
	
	
	//\____________________ On dessine la ficelle qui les joint.
	if (myIcons.iStringLineWidth > 0)
		cairo_dock_draw_string_opengl (pDock, myIcons.iStringLineWidth, FALSE, (my_iDrawSeparator3D == CD_FLAT_SEPARATOR || my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR));
	
	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
		
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	
	///glLoadIdentity ();
 	if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR || my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
	{
		do
		{
			icon = ic->data;
			
			if (icon->cFileName == NULL && CAIRO_DOCK_IS_SEPARATOR (icon))
			{
				glPushMatrix ();
				if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
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
			
			if (icon->cFileName != NULL || ! CAIRO_DOCK_IS_SEPARATOR (icon))
			{
				glPushMatrix ();
				cairo_dock_render_one_icon_opengl (icon, pDock, fDockMagnitude, TRUE);
				glPopMatrix ();
			}
			
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
		
		if (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
		{
			do
			{
				icon = ic->data;
				
				if (icon->cFileName == NULL && CAIRO_DOCK_IS_SEPARATOR (icon))
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


void cd_rendering_draw_flat_separator_opengl (Icon *icon, CairoDock *pDock)
{
	double hi = myIcons.fReflectSize * pDock->container.fRatio + myBackground.iFrameMargin;
	double fLeftInclination = (icon->fDrawX - pDock->container.iWidth / 2) / iVanishingPointY;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->container.iWidth / 2) / iVanishingPointY;
	
	double fHeight = pDock->iDecorationsHeight;
	double fBigWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi);
	double fLittleWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi - fHeight);
	
	double fDeltaXLeft = fHeight * fLeftInclination;
	double fDeltaXRight = fHeight * fRightInclination;
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	
	double fDockOffsetX, fDockOffsetY;
	fDockOffsetX = icon->fDrawX - (fHeight - hi) * fLeftInclination;
	fDockOffsetY = fHeight + myBackground.iDockLineWidth;
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (1., 1., 1., 1.);
	
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, my_iFlatSeparatorTexture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glPolygonMode (GL_FRONT, GL_FILL);
	
	if (pDock->container.bIsHorizontal)
	{
		if (! pDock->container.bDirectionUp)
			fDockOffsetY = pDock->container.iHeight - fDockOffsetY;
		
		glTranslatef (fDockOffsetX, fDockOffsetY, 0.);  // coin haut gauche.
		if (! pDock->container.bDirectionUp)
			glScalef (1., -1., 1.);
	}
	else
	{
		if (pDock->container.bDirectionUp)
			fDockOffsetY = pDock->container.iHeight - fDockOffsetY;
		fDockOffsetX = pDock->container.iWidth - fDockOffsetX;
		
		glTranslatef (fDockOffsetY, fDockOffsetX, 0.);
		glRotatef (-90., 0., 0., 1.);
		if (pDock->container.bDirectionUp)
			glScalef (1., -1., 1.);
	}
	
	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.);
	glVertex3f(0., 0., 0.);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1., 0.);
	glVertex3f(fLittleWidth, 0., 0.);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1., 1.);
	glVertex3f(fLittleWidth + fDeltaXRight, - fHeight, 0.);  // Top Right Of The Texture and Quad
	glTexCoord2f(0., 1.);
	glVertex3f(fLittleWidth + fDeltaXRight - fBigWidth, - fHeight, 0.);  // Top Left Of The Texture and Quad
	glEnd();
	
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
}

void cd_rendering_draw_physical_separator_opengl (Icon *icon, CairoDock *pDock, gboolean bBackGround, Icon *prev_icon, Icon *next_icon)
{
	if (prev_icon == NULL)
		prev_icon = icon;
	if (next_icon == NULL)
		next_icon = icon;
	double hi = myIcons.fReflectSize * pDock->container.fRatio + myBackground.iFrameMargin;
	hi = (pDock->container.bDirectionUp ? pDock->container.iHeight - (icon->fDrawY + icon->fHeight * icon->fScale) : icon->fDrawY);
	//g_print ("%s : hi = %.2f/%.2f\n", icon->cName, myIcons.fReflectSize * pDock->container.fRatio + myBackground.iFrameMargin, pDock->container.iHeight - (icon->fDrawY + icon->fHeight * icon->fScale));
	double fLeftInclination = (icon->fDrawX - pDock->container.iWidth / 2) / iVanishingPointY;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->container.iWidth / 2) / iVanishingPointY;
	
	double fHeight, fBigWidth, fLittleWidth;
	if (bBackGround)
	{
		fHeight = pDock->iDecorationsHeight + myBackground.iDockLineWidth - hi;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + 0);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + 0 - fHeight);
	}
	else
	{
		fHeight = hi + myBackground.iDockLineWidth;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (iVanishingPointY + hi - fHeight);
	}
	double fDeltaXLeft = fHeight * fLeftInclination;
	double fDeltaXRight = fHeight * fRightInclination;
	
	double fDockOffsetX, fDockOffsetY;
	if (bBackGround)
	{
		fDockOffsetX = icon->fDrawX - fHeight * fLeftInclination;
		fDockOffsetY = pDock->iDecorationsHeight + 2*myBackground.iDockLineWidth;
	}
	else
	{
		fDockOffsetX = icon->fDrawX;
		fDockOffsetY = fHeight;
	}
	//g_print ("X : %.2f + %.2f/%.2f ; Y : %.2f + %.2f\n", fDockOffsetX, fBigWidth, fLittleWidth, fDockOffsetY, fHeight);
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ZERO);
	glColor4f (0., 0., 0., 0.);
	
	glPolygonMode (GL_FRONT, GL_FILL);
	
	if (pDock->container.bIsHorizontal)
	{
		if (! pDock->container.bDirectionUp)
			fDockOffsetY = pDock->container.iHeight - fDockOffsetY;
		
		glTranslatef (fDockOffsetX, fDockOffsetY, 0.);  // coin haut gauche.
		if (! pDock->container.bDirectionUp)
			glScalef (1., -1., 1.);
	}
	else
	{
		if (pDock->container.bDirectionUp)
			fDockOffsetY = pDock->container.iHeight - fDockOffsetY;
		fDockOffsetX = pDock->container.iWidth - fDockOffsetX;
		
		glTranslatef (fDockOffsetY, fDockOffsetX, 0.);
		glRotatef (-90., 0., 0., 1.);
		if (pDock->container.bDirectionUp)
			glScalef (1., -1., 1.);
	}
	
	glBegin(GL_QUADS);
	glVertex3f(0., 0., 0.);  // Bottom Left Of The Texture and Quad
	glVertex3f(fLittleWidth, 0., 0.);  // Bottom Right Of The Texture and Quad
	glVertex3f(fLittleWidth + fDeltaXRight, - fHeight, 0.);  // Top Right Of The Texture and Quad
	glVertex3f(fLittleWidth + fDeltaXRight - fBigWidth, - fHeight, 0.);  // Top Left Of The Texture and Quad
	glEnd();
	
	if (myBackground.iDockLineWidth != 0)
	{
		glPolygonMode (GL_FRONT, GL_LINE);
		glEnable (GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glLineWidth (myBackground.iDockLineWidth);
		glColor4f (myBackground.fLineColor[0], myBackground.fLineColor[1], myBackground.fLineColor[2], myBackground.fLineColor[3]);
		
		glBegin(GL_LINES);
		glVertex3f(fLittleWidth, 0., 0.);
		glVertex3f(fLittleWidth + fDeltaXRight, - fHeight, 0.);
		glEnd();
		
		glBegin(GL_LINES);
		glVertex3f(0., 0., 0.);
		glVertex3f(fLittleWidth + fDeltaXRight - fBigWidth, - fHeight, 0.);
		glEnd();
		
		glDisable(GL_LINE_SMOOTH);
	}
	
	glDisable (GL_BLEND);
}


void cd_rendering_register_3D_plane_renderer (const gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	pRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-3D-plane-view", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->cPreviewFilePath = g_strdup_printf ("%s/preview-3D-plane.jpg", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->compute_size = cd_rendering_calculate_max_dock_size_3D_plane;
	pRenderer->calculate_icons = cd_rendering_calculate_icons_3D_plane;
	pRenderer->render = cd_rendering_render_3D_plane;
	pRenderer->render_optimized = cd_rendering_render_optimized_3D_plane;
	pRenderer->render_opengl = cd_rendering_render_3D_plane_opengl;
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;
	pRenderer->bUseReflect = TRUE;
	pRenderer->cDisplayedName = D_ (cRendererName);
	
	cairo_dock_register_renderer (cRendererName, pRenderer);
}
