/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

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

#include "rendering-3D-plane.h"
#include "rendering-3D-plane.h"

extern int iVanishingPointY;
extern CDSpeparatorType my_iDrawSeparator3D;
extern cairo_surface_t *my_pFlatSeparatorSurface[2];
extern double my_fSeparatorColor[4];

void cd_rendering_calculate_max_dock_size_3D_plane (CairoDock *pDock)
{
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->fFlatDockWidth, pDock->iScrollOffset);
	
	pDock->iMaxDockHeight = (int) ((1 + g_fAmplitude) * pDock->iMaxIconHeight + g_fReflectSize) + g_iLabelSize + g_iDockLineWidth + g_iFrameMargin;
	
	double hi = g_fReflectSize + g_iFrameMargin;
	
	double fInclinationOnHorizon = 0, fExtraWidth = 0;
	pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., fExtraWidth));
	fInclinationOnHorizon = 0.5 * pDock->iMaxDockWidth / iVanishingPointY;
	pDock->iDecorationsHeight = hi + (pDock->iMaxIconHeight + g_iFrameMargin) / sqrt (1 + fInclinationOnHorizon * fInclinationOnHorizon);
	fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (pDock->iDecorationsHeight, fInclinationOnHorizon, g_iDockRadius, g_iDockLineWidth);
	cd_debug ("iMaxDockWidth <- %d; fInclinationOnHorizon <- %.2f; fExtraWidth <- %.2f", pDock->iMaxDockWidth, fInclinationOnHorizon, fExtraWidth);
	
	pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, 1., fExtraWidth));
	cd_debug ("pDock->iMaxDockWidth <- %d", pDock->iMaxDockWidth);
	
	pDock->iDecorationsWidth = pDock->iMaxDockWidth;
	
	pDock->iMinDockWidth = pDock->fFlatDockWidth + fExtraWidth;
	pDock->iMinDockHeight = g_iDockLineWidth + g_iFrameMargin + g_fReflectSize + pDock->iMaxIconHeight;
}

void cd_rendering_calculate_construction_parameters_3D_plane (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxDockWidth, double fReflectionOffsetY)
{
	icon->fDrawX = icon->fX;
	icon->fDrawY = icon->fY + fReflectionOffsetY;
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


cairo_surface_t *cd_rendering_create_flat_separator_surface (cairo_t *pSourceContext, int iWidth, int iHeight)
{
	cairo_pattern_t *pStripesPattern = cairo_pattern_create_linear (0.0f,
		iHeight,
		0.,
		0.);
	g_return_val_if_fail (cairo_pattern_status (pStripesPattern) == CAIRO_STATUS_SUCCESS, NULL);
	
	cairo_pattern_set_extend (pStripesPattern, CAIRO_EXTEND_REPEAT);
	
	double fStep = 1.;
	double y = 0, h0 = (fStep * (1 + sqrt (1 + 4. * iHeight / fStep)) / 2) - 1, hk = h0;
	int k = 0;
	for (k = 0; k < h0 / fStep; k ++)
	{
		//g_print ("step : %f ; y = %.2f\n", 1.*hk / iHeight, y);
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			0.,
			0.,
			0.,
			0.);
		y += 1.*hk / iHeight;
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			0.,
			0.,
			0.,
			0.);
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			my_fSeparatorColor[0],
			my_fSeparatorColor[1],
			my_fSeparatorColor[2],
			my_fSeparatorColor[3]);
		y += 1.*hk / iHeight;
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			my_fSeparatorColor[0],
			my_fSeparatorColor[1],
			my_fSeparatorColor[2],
			my_fSeparatorColor[3]);
		hk -= fStep;
	}
	
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		iWidth,
		iHeight);
	cairo_t *pImageContext = cairo_create (pNewSurface);
	cairo_set_source (pImageContext, pStripesPattern);
	cairo_paint (pImageContext);
	
	cairo_pattern_destroy (pStripesPattern);
	cairo_destroy (pImageContext);
	
	return pNewSurface;
}

static void cd_rendering_draw_3D_separator_horizontal (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bIncludeEdges, gboolean bBackGround)
{
	double hi = g_fReflectSize + g_iFrameMargin;
	double fLeftInclination = (icon->fDrawX - pDock->iCurrentWidth / 2) / iVanishingPointY;
	double fRightInclination = (icon->fDrawX + icon->fWidth * icon->fScale - pDock->iCurrentWidth / 2) / iVanishingPointY;
	
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
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
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
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, -1, 1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL], 0, MIN (0, (fHeight + hi) * fLeftInclination));
		}
	}
}
static void cd_rendering_draw_3D_separator_vertical (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bIncludeEdges, gboolean bBackGround)
{
	double hi = g_fReflectSize + g_iFrameMargin;
	double fLeftInclination = fabs (icon->fDrawX - pDock->iCurrentWidth / 2) / iVanishingPointY;
	double fRightInclination = fabs (icon->fDrawX + icon->fWidth * icon->fScale - pDock->iCurrentWidth / 2) / iVanishingPointY;
	
	double fHeight, fBigWidth, fLittleWidth;
	if (bIncludeEdges)
	{
		fHeight = (bBackGround ? pDock->iDecorationsHeight - hi : hi) + g_iDockLineWidth;
		fBigWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY : iVanishingPointY + fHeight);
		fLittleWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY - fHeight : iVanishingPointY);
	}
	else
	{
		fHeight = pDock->iDecorationsHeight;
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
			fDockOffsetY =  pDock->iCurrentHeight - fHeight - (bBackGround ? g_iDockLineWidth + hi : 0);
		else
			fDockOffsetY =  pDock->iCurrentHeight - fHeight - g_iDockLineWidth;
	}
	else
	{
		sens = -1;
		if (bIncludeEdges)
			fDockOffsetY = fHeight + (bBackGround ? g_iDockLineWidth + hi : 0);
		else
			fDockOffsetY = fHeight + g_iDockLineWidth;
	}
	if (icon->fDrawX + icon->fWidth * icon->fScale / 2 > pDock->iCurrentWidth / 2)  // on est a droite.
	{
		if (bIncludeEdges)
			fDockOffsetX = icon->fDrawX - (bBackGround ? fHeight * fLeftInclination : 0);
		else
			fDockOffsetX = icon->fDrawX - (fHeight - hi) * fLeftInclination;
		cairo_translate (pCairoContext, fDockOffsetY, fDockOffsetX);  // coin haut gauche.
		cairo_move_to (pCairoContext, 0, 0);  // coin haut gauche.
		//g_print ("droite : fDockOffsetX : %.2f; fDeltaX:%.2f; fEpsilon:%.2f\n", fDockOffsetX, fDeltaX, fEpsilon);
		
		cairo_rel_line_to (pCairoContext, 0, fLittleWidth);
		cairo_rel_line_to (pCairoContext, sens * fHeight, fDeltaXRight);
		cairo_rel_line_to (pCairoContext, 0, - fBigWidth);
		cairo_rel_line_to (pCairoContext, - sens * fHeight, - fDeltaXLeft);
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, -1, 1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL], 0, 0);
		}
	}
	else  // a gauche.
	{
		if (bIncludeEdges)
			fDockOffsetX = icon->fDrawX + (bBackGround ? fHeight * fLeftInclination : 0);
		else
			fDockOffsetX = icon->fDrawX + (fHeight - hi) * fLeftInclination;
		cairo_translate (pCairoContext, fDockOffsetY, fDockOffsetX);  // coin haut gauche.
		cairo_move_to (pCairoContext, 0, 0);  // coin haut gauche.
		//g_print ("gauche : fDockOffsetX : %.2f; fDeltaX:%.2f; fEpsilon:%.2f\n", fDockOffsetX, fDeltaX, fEpsilon);
		
		cairo_rel_line_to (pCairoContext, 0, fLittleWidth);
		cairo_rel_line_to (pCairoContext, sens * fHeight, - fDeltaXRight + 0);
		cairo_rel_line_to (pCairoContext, 0, - fBigWidth);
		cairo_rel_line_to (pCairoContext, - sens * fHeight, fDeltaXLeft + 0);
		
		if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
		{
			if (! pDock->bDirectionUp)
				cairo_scale (pCairoContext, -1, 1);
			cairo_set_source_surface (pCairoContext, my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL], - fHeight * fLeftInclination, 0);
		}
	}
}
static void cd_rendering_draw_3D_separator_edge_horizontal (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bBackGround)
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
		fDockOffsetY =  (bBackGround ? 1.5*g_iDockLineWidth : 0);
	}
	else
	{
		sens = -1;
		fDockOffsetY =  (bBackGround ? - 1.5*g_iDockLineWidth : 0);
	}
	fDockOffsetX = (bBackGround ? g_iDockLineWidth * fLeftInclination : - 0.5 * g_iDockLineWidth * fLeftInclination);
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	
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

static void cd_rendering_draw_3D_separator_edge_vertical (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bBackGround)
{
	int sens;
	double fDockOffsetY = 0;
	if (pDock->bDirectionUp)
	{
		sens = 1;
	}
	else
	{
		sens = -1;
	}
	
	double hi = g_fReflectSize + g_iFrameMargin;
	double fLeftInclination = fabs (icon->fDrawX - pDock->iCurrentWidth / 2) / iVanishingPointY;
	double fRightInclination = fabs (icon->fDrawX + icon->fWidth * icon->fScale - pDock->iCurrentWidth / 2) / iVanishingPointY;
	
	double fFrameHeight = pDock->iDecorationsHeight + 2 * g_iDockLineWidth;
	double fHeight = (bBackGround ? pDock->iDecorationsHeight - hi : hi) + g_iDockLineWidth;
	
	double fBigWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY : iVanishingPointY + fHeight);
	double fLittleWidth = fabs (fRightInclination - fLeftInclination) * (bBackGround ? iVanishingPointY - fHeight : iVanishingPointY);
	
	double fDeltaXLeft = fHeight * fLeftInclination;
	double fDeltaXRight = fHeight * fRightInclination;
	double fDeltaX = MAX (fDeltaXLeft, fDeltaXRight);
	
	//g_print ("fBigWidth : %.2f ; fLittleWidth : %.2f\n", fBigWidth, fLittleWidth);
	if (icon->fDrawX + icon->fWidth * icon->fScale / 2 > pDock->iCurrentWidth / 2)  // on est a droite.
	{
		cairo_translate (pCairoContext, fDockOffsetY, 0);  // coin haut droit.
		
		cairo_move_to (pCairoContext, 0, fLittleWidth);
		cairo_rel_line_to (pCairoContext, sens * fHeight, fDeltaXRight);
		
		cairo_move_to (pCairoContext, 0, 0);
		cairo_rel_line_to (pCairoContext, sens * fHeight, fDeltaXLeft);
	}
	else  // a gauche.
	{
		cairo_translate (pCairoContext, fDockOffsetY, 0);  // coin haut droit.
		
		cairo_move_to (pCairoContext, 0, fLittleWidth);
		cairo_rel_line_to (pCairoContext, sens * fHeight, - fDeltaXRight + 0);
		
		cairo_move_to (pCairoContext, 0, 0);
		cairo_rel_line_to (pCairoContext, sens * fHeight, - fDeltaXLeft);
	}
}

static void cd_rendering_draw_3D_separator (Icon *icon, cairo_t *pCairoContext, CairoDock *pDock, gboolean bHorizontal, gboolean bBackGround)
{
	//g_print ("%s ()\n", __func__);
	/**if (bHorizontal)
		cd_rendering_draw_3D_separator_horizontal (icon, pCairoContext, pDock, (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR), bBackGround);
	else
		cd_rendering_draw_3D_separator_vertical (icon, pCairoContext, pDock, (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR), bBackGround);*/
	cd_rendering_draw_3D_separator_horizontal (icon, pCairoContext, pDock, (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR), bBackGround);
	
	if (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
	{
		cairo_set_operator (pCairoContext, CAIRO_OPERATOR_DEST_OUT);
		cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 1.0);
		cairo_fill (pCairoContext);
		
		/**if (bHorizontal)
			cd_rendering_draw_3D_separator_edge_horizontal (icon, pCairoContext, pDock, bBackGround);
		else
			cd_rendering_draw_3D_separator_edge_vertical (icon, pCairoContext, pDock, bBackGround);*/
		cd_rendering_draw_3D_separator_edge_horizontal (icon, pCairoContext, pDock, bBackGround);
		
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


void cd_rendering_render_3D_plane (CairoDock *pDock)
{
	//\____________________ On cree le contexte du dessin.
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	
	cairo_set_tolerance (pCairoContext, 0.5);  // avec moins que 0.5 on ne voit pas la difference.
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	//\____________________ On trace le cadre.
	double fLineWidth = g_iDockLineWidth;
	double fMargin = g_iFrameMargin;
	double fRadius = (pDock->iDecorationsHeight + fLineWidth - 2 * g_iDockRadius > 0 ? g_iDockRadius : (pDock->iDecorationsHeight + fLineWidth) / 2 - 1);
	double fDockWidth = cairo_dock_get_current_dock_width_linear (pDock);
	
	int sens;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX + 0 - fMargin : fRadius + fLineWidth / 2);
	if (pDock->bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = pDock->iCurrentHeight - pDock->iDecorationsHeight - 0.5 * fLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = pDock->iDecorationsHeight + 0.5 * fLineWidth;
	}
	
	cairo_save (pCairoContext);
	
	double fInclinationOnHorizon = (fDockWidth / 2) / iVanishingPointY;
        cairo_dock_draw_frame (pCairoContext, fRadius, 1, fDockWidth, pDock->iDecorationsHeight, fDockOffsetX, fDockOffsetY, sens, fInclinationOnHorizon, pDock->bHorizontalDock);  // fLineWidth
	
	//\____________________ On dessine les decorations dedans.
	fDockOffsetY = (pDock->bDirectionUp ? pDock->iCurrentHeight - pDock->iDecorationsHeight - 0.5*fLineWidth : 0.5*fLineWidth);
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
	if (pFirstDrawnElement != NULL)
	{
		double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
		Icon *icon;
		GList *ic = pFirstDrawnElement;
		
		if (my_pFlatSeparatorSurface[0] != NULL || my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
		{
			cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_SQUARE);
			do
			{
				icon = ic->data;
				
				if (icon->acFileName == NULL && CAIRO_DOCK_IS_SEPARATOR (icon))
				{
					cairo_save (pCairoContext);
					cd_rendering_draw_3D_separator (icon, pCairoContext, pDock, pDock->bHorizontalDock, TRUE);
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
			
			if (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
			{
				do
				{
					icon = ic->data;
					
					if (icon->acFileName == NULL && CAIRO_DOCK_IS_SEPARATOR (icon))
					{
						cairo_save (pCairoContext);
						cd_rendering_draw_3D_separator (icon, pCairoContext, pDock, pDock->bHorizontalDock, FALSE);
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
	
	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
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

void cd_rendering_render_optimized_3D_plane (CairoDock *pDock, GdkRectangle *pArea)
{
	//g_print ("%s ((%d;%d) x (%d;%d) / (%dx%d))\n", __func__, pArea->x, pArea->y, pArea->width, pArea->height, pDock->iCurrentWidth, pDock->iCurrentHeight);
	double fLineWidth = g_iDockLineWidth;
	double fMargin = g_iFrameMargin;
	int iWidth = pDock->iCurrentWidth;
	int iHeight = pDock->iCurrentHeight;
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (pDock));
	g_return_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS);
	cairo_rectangle (pCairoContext,
		pArea->x,
		pArea->y,
		pArea->width,
		pArea->height);
	cairo_clip (pCairoContext);
	cairo_set_tolerance (pCairoContext, 0.5);
	cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pCairoContext);
	
	//\____________________ On dessine les decorations du fond sur la portion de fenetre.
	cairo_save (pCairoContext);
	
	double fDockOffsetX, fDockOffsetY;
	if (pDock->bHorizontalDock)
	{
		fDockOffsetX = pArea->x;
		fDockOffsetY = (pDock->bDirectionUp ? iHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	}
	else
	{
		fDockOffsetX = (pDock->bDirectionUp ? iHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
		fDockOffsetY = pArea->y;
	}
	
	cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY);
	if (pDock->bHorizontalDock)
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pArea->width, pDock->iDecorationsHeight);
	else
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pDock->iDecorationsHeight, pArea->height);
	
	fDockOffsetY = (pDock->bDirectionUp ? pDock->iCurrentHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY);
	
	
	//\____________________ On dessine la partie du cadre qui va bien.
	cairo_new_path (pCairoContext);
	
	if (pDock->bHorizontalDock)
	{
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY + 0.5*fLineWidth);
		cairo_rel_line_to (pCairoContext, pArea->width, 0);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		cairo_stroke (pCairoContext);
		
		cairo_new_path (pCairoContext);
		cairo_move_to (pCairoContext, fDockOffsetX, (pDock->bDirectionUp ? iHeight - 0.5*fLineWidth : pDock->iDecorationsHeight + 1.5 * fLineWidth));
		cairo_rel_line_to (pCairoContext, pArea->width, 0);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	}
	else
	{
		cairo_move_to (pCairoContext, fDockOffsetX - fLineWidth / 2, fDockOffsetY);
		cairo_rel_line_to (pCairoContext, 0, pArea->height);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
		cairo_stroke (pCairoContext);
		
		cairo_new_path (pCairoContext);
		cairo_move_to (pCairoContext, (pDock->bDirectionUp ? iHeight - fLineWidth / 2 : pDock->iDecorationsHeight + 1.5 * fLineWidth), fDockOffsetY);
		cairo_rel_line_to (pCairoContext, 0, pArea->height);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, g_fLineColor[0], g_fLineColor[1], g_fLineColor[2], g_fLineColor[3]);
	}
	cairo_stroke (pCairoContext);
	
	cairo_restore (pCairoContext);
	
	//\____________________ On dessine les icones impactees.
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
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
		
		if (my_pFlatSeparatorSurface[0] != NULL || my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
		{
			cairo_set_line_cap (pCairoContext, CAIRO_LINE_CAP_SQUARE);
			do
			{
				icon = ic->data;
				
				if (CAIRO_DOCK_IS_SEPARATOR (icon) && icon->acFileName == NULL)
				{
					if (_cd_separator_is_impacted (icon, pDock, fXMin, fXMax, TRUE, (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)))
					{
						cairo_save (pCairoContext);
						cd_rendering_draw_3D_separator (icon, pCairoContext, pDock, pDock->bHorizontalDock, TRUE);
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
			
			if (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)
			{
				do
				{
					icon = ic->data;
					
					if (CAIRO_DOCK_IS_SEPARATOR (icon) && icon->acFileName == NULL)
					{
						if (_cd_separator_is_impacted (icon, pDock, fXMin, fXMax, FALSE, (my_iDrawSeparator3D == CD_PHYSICAL_SEPARATOR)))
						{
							cairo_save (pCairoContext);
							cd_rendering_draw_3D_separator (icon, pCairoContext, pDock, pDock->bHorizontalDock, FALSE);
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
	
	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (pDock->pDrawFormat && pDock->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (pDock->pGlitzDrawable);
#endif
}


Icon *cd_rendering_calculate_icons_3D_plane (CairoDock *pDock)
{
	Icon *pPointedIcon = cairo_dock_apply_wave_effect (pDock);
	
	CairoDockMousePositionType iMousePositionType = cairo_dock_check_if_mouse_inside_linear (pDock);
	
	cairo_dock_manage_mouse_position (pDock, iMousePositionType);
	
	//\____________________ On calcule les position/etirements/alpha des icones.
	cairo_dock_mark_avoiding_mouse_icons_linear (pDock);
	
	double fReflectionOffsetY = (pDock->bDirectionUp ? -1 : 1) * g_fReflectSize;
	Icon* icon;
	GList* ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cd_rendering_calculate_construction_parameters_3D_plane (icon, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iMaxDockWidth, fReflectionOffsetY);
		cairo_dock_manage_animations (icon, pDock);
	}
	
	return (iMousePositionType == CAIRO_DOCK_MOUSE_INSIDE ? pPointedIcon : NULL);
}

void cd_rendering_register_3D_plane_renderer (void)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	pRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-3D-plane-view", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->cPreviewFilePath = g_strdup_printf ("%s/preview-3D-plane.png", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->calculate_max_dock_size = cd_rendering_calculate_max_dock_size_3D_plane;
	pRenderer->calculate_icons = cd_rendering_calculate_icons_3D_plane;
	pRenderer->render = cd_rendering_render_3D_plane;
	pRenderer->render_optimized = cd_rendering_render_optimized_3D_plane;
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;
	pRenderer->bUseReflect = TRUE;
	
	cairo_dock_register_renderer (MY_APPLET_3D_PLANE_VIEW_NAME, pRenderer);
}

