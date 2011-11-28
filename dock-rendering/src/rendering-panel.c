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

#include "rendering-panel.h"

extern gdouble my_fPanelRadius;
extern gdouble my_fPanelInclination;
extern gdouble my_fPanelRatio;
const int iNbCurveSteps = 10;

typedef struct {
	gdouble fGroupGap;  // gap between 2 groups of icons (= width of a separator), at rest.
	} CDPanelData;

static void cd_compute_size (CairoDock *pDock)
{
	//\_____________ On calcule le nombre de groupes et la place qu'ils occupent.
	int iNbGroups = 1;
	double fCurrentGroupWidth = - myIconsParam.iIconGap, fGroupsWidth = 0.;
	GList *ic;
	Icon *pIcon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			if (fCurrentGroupWidth > 0)  // le groupe courant est non vide, sinon c'est juste 2 separateurs cote a cote.
			{
				iNbGroups ++;
				fGroupsWidth += MAX (0, fCurrentGroupWidth);
				//g_print ("fGroupsWidth += %.2f\n", fCurrentGroupWidth);
				fCurrentGroupWidth = - myIconsParam.iIconGap;
			}
			
			continue;
		}
		fCurrentGroupWidth += pIcon->fWidth * my_fPanelRatio + myIconsParam.iIconGap;
		//g_print ("fCurrentGroupWidth <- %.2f\n", fCurrentGroupWidth);
	}
	if (fCurrentGroupWidth > 0)  // le groupe courant est non vide, sinon c'est juste un separateur a la fin.
	{
		fGroupsWidth += MAX (0, fCurrentGroupWidth);
		//g_print ("fGroupsWidth += %.2f\n", fCurrentGroupWidth);
	}
	if (fGroupsWidth < 0)
		fGroupsWidth = 0;
	
	//\_____________ On en deduit l'ecart entre les groupes d'icones.
	double W = cairo_dock_get_max_authorized_dock_width (pDock);
	double fScreenBorderGap = myDocksParam.iDockRadius + myDocksParam.iDockLineWidth;  // on laisse un ecart avec le bord de l'ecran.
	double fGroupGap = (iNbGroups > 1 ? (W - 2*fScreenBorderGap - fGroupsWidth) / (iNbGroups - 1) : W - fScreenBorderGap - fGroupsWidth);
	if (fGroupGap < myIconsParam.iIconGap)  // les icones depassent en largeur.
		fGroupGap = myIconsParam.iIconGap;
	//g_print (" -> %d groups, %d/%d\nfGroupGap = %.2f\n", iNbGroups, (int)fGroupsWidth, (int)W, fGroupGap);
	
	//\_____________ On calcule la position au repos des icones et la taille du dock.
	double xg = fScreenBorderGap;  // abscisse de l'icone courante, et abscisse du debut du groupe courant.
	double x = xg;
	if (iNbGroups == 1 && pDock->icons)  // if the first icon is a separator, place icons to the right.
	{
		pIcon = pDock->icons->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			x += fGroupGap;
	}
	fCurrentGroupWidth = - myIconsParam.iIconGap;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			pIcon->fXAtRest = x;
			if (fCurrentGroupWidth > 0)  // le groupe courant est non vide, sinon c'est juste 2 separateurs cote a cote.
			{
				xg += fCurrentGroupWidth + fGroupGap;
				x = xg;
				//g_print ("jump to %.2f\n", x);
				fCurrentGroupWidth = - myIconsParam.iIconGap;
			}
			
			continue;
		}
		fCurrentGroupWidth += pIcon->fWidth * my_fPanelRatio + myIconsParam.iIconGap;
		
		pIcon->fXAtRest = x;
		x += pIcon->fWidth * my_fPanelRatio + myIconsParam.iIconGap;
	}
	
	pDock->fMagnitudeMax = 0.;  // pas de vague.
	
	pDock->pFirstDrawnElement = pDock->icons;
	
	double hicon = pDock->iMaxIconHeight * my_fPanelRatio;
	pDock->iDecorationsHeight = hicon * pDock->container.fRatio + 2 * myDocksParam.iFrameMargin;
	
	pDock->iMaxDockWidth = pDock->fFlatDockWidth = pDock->iMinDockWidth = MAX (W, x);
	//g_print ("iMaxDockWidth : %d (%.2f)\n", pDock->iMaxDockWidth, pDock->container.fRatio);
	
	pDock->iMaxDockHeight = myDocksParam.iDockLineWidth + myDocksParam.iFrameMargin + hicon * pDock->container.fRatio + myDocksParam.iFrameMargin + myDocksParam.iDockLineWidth + (pDock->container.bIsHorizontal || !myIconsParam.bTextAlwaysHorizontal ? myIconsParam.iLabelSize : 0);
	
	pDock->iMaxDockHeight = MAX (pDock->iMaxDockHeight, pDock->iMaxIconHeight * (1 + myIconsParam.fAmplitude));  // au moins la taille du FBO.
	//g_print ("panel view: pDock->iMaxIconHeight = %d\n", pDock->iMaxIconHeight);

	pDock->iDecorationsWidth = pDock->iMaxDockWidth;
	pDock->iMinDockHeight = 2 * (myDocksParam.iDockLineWidth + myDocksParam.iFrameMargin) + hicon * pDock->container.fRatio;  /// TODO: make the height constant, to avoid moving all windows when space is reserved and ratio changes.
	
	pDock->iActiveWidth = pDock->iMaxDockWidth;
	pDock->iActiveHeight = pDock->iMinDockHeight;
	if (! pDock->container.bIsHorizontal && myIconsParam.bTextAlwaysHorizontal)
		pDock->iMaxDockHeight += 8*myIconsParam.iLabelSize;  // vertical dock, add some padding to draw the labels
	
	CDPanelData *pData = pDock->pRendererData;
	if (pData == NULL)
	{
		pData = g_new0 (CDPanelData, 1);
		pDock->pRendererData = pData;
	}
	pData->fGroupGap = fGroupGap;
}


static void cd_render (cairo_t *pCairoContext, CairoDock *pDock)
{
	//\____________________ On trace le cadre.
	double fLineWidth = myDocksParam.iDockLineWidth;
	double fMargin = myDocksParam.iFrameMargin;
	double fRadius = (pDock->iDecorationsHeight + fLineWidth - 2 * myDocksParam.iDockRadius > 0 ? myDocksParam.iDockRadius : (pDock->iDecorationsHeight + fLineWidth) / 2 - 1);
	double fExtraWidth = 2 * fRadius + fLineWidth;
	double fDockWidth;
	int sens;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	if (cairo_dock_is_extended_dock (pDock))  // mode panel etendu.
	{
		fDockWidth = pDock->container.iWidth - fExtraWidth;
		fDockOffsetX = fExtraWidth / 2;
	}
	else
	{
		fDockWidth = cairo_dock_get_current_dock_width_linear (pDock);
		Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
		fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX - fMargin : fExtraWidth / 2);
		if (fDockOffsetX < fExtraWidth / 2)
			fDockOffsetX = fExtraWidth / 2;
		if (fDockOffsetX + fDockWidth + fExtraWidth / 2 > pDock->container.iWidth)
			fDockWidth = pDock->container.iWidth - fDockOffsetX - fExtraWidth / 2;
	}
	if (pDock->container.bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = pDock->container.iHeight - pDock->iDecorationsHeight - 1.5 * fLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = pDock->iDecorationsHeight + 1.5 * fLineWidth;
	}

	cairo_save (pCairoContext);
	double fDeltaXTrapeze = cairo_dock_draw_frame (pCairoContext, fRadius, fLineWidth, fDockWidth, pDock->iDecorationsHeight, fDockOffsetX, fDockOffsetY, sens, 0., pDock->container.bIsHorizontal, myDocksParam.bRoundedBottomCorner);

	//\____________________ On dessine les decorations dedans.
	fDockOffsetY = (pDock->container.bDirectionUp ? pDock->container.iHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY, fDockOffsetX - fDeltaXTrapeze, fDockWidth + 2*fDeltaXTrapeze);

	//\____________________ On dessine le cadre.
	if (fLineWidth > 0)
	{
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
		cairo_stroke (pCairoContext);
	}
	else
		cairo_new_path (pCairoContext);
	cairo_restore (pCairoContext);
	
	//\____________________ On dessine les separateurs physiques.
	cairo_save (pCairoContext);
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
		cairo_translate (pCairoContext, pDock->container.iHeight/2., pDock->container.iWidth/2.);
		cairo_rotate (pCairoContext, G_PI/2);
		if (pDock->container.bDirectionUp)
			cairo_scale (pCairoContext, 1., -1.);
		cairo_translate (pCairoContext, -pDock->container.iWidth/2., -pDock->container.iHeight/2.);
	}
	
	double x1, x2, dx, delta, h = pDock->iDecorationsHeight + 2*fLineWidth, h_ = h - fLineWidth;
	GList *ic;
	Icon *pIcon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			x1 = pIcon->fDrawX = pIcon->fX;
			
			pIcon = NULL;
			for (ic = ic->next; ic != NULL; ic = ic->next)
			{
				pIcon = ic->data;
				if (!CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
					break;
			}
			if (ic != NULL)
			{
				pIcon = ic->data;
				x2 = pIcon->fDrawX;
			}
			else
				break;
			
			dx = MIN (my_fPanelRadius, (x2 - x1) / 2);
			delta = dx + h*tan(my_fPanelInclination)/2;
			if (delta > (x2 - x1) / 2)
				delta = (x2 - x1) / 2;
			
			cairo_move_to (pCairoContext, x1, pDock->iMaxDockHeight - h);
			cairo_rel_curve_to (pCairoContext,
				dx, 0.,
				delta - dx, h,
				delta, h);
			cairo_rel_line_to (pCairoContext,
				x2 - x1 - 2*delta, 0.);
			cairo_rel_curve_to (pCairoContext,
				dx, 0.,
				delta - dx, -h,
				delta, -h);
			cairo_close_path (pCairoContext);
			
			cairo_set_operator (pCairoContext, CAIRO_OPERATOR_DEST_OUT);
			cairo_set_source_rgba (pCairoContext, 0.0, 0.0, 0.0, 1.0);
			cairo_fill (pCairoContext);
			
			if (fLineWidth > 0)
			{
				cairo_move_to (pCairoContext, x1, pDock->iMaxDockHeight - h_ - fLineWidth/2);
				cairo_rel_curve_to (pCairoContext,
					dx, 0.,
					delta - dx, h_,
					delta, h_);
				cairo_rel_line_to (pCairoContext,
					x2 - x1 - 2*delta, 0.);
				cairo_rel_curve_to (pCairoContext,
					dx, 0.,
					delta - dx, -h_,
					delta, -h_);
				
				cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
				cairo_set_line_width (pCairoContext, fLineWidth);
				cairo_set_source_rgba (pCairoContext, myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
				cairo_stroke (pCairoContext);
			}
		}
	}
	cairo_restore (pCairoContext);
	
	//\____________________ On dessine la ficelle qui les joint.
	if (myIconsParam.iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, myIconsParam.iStringLineWidth, FALSE, FALSE);

	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
	
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);  // * pDock->fMagnitudeMax
	ic = pFirstDrawnElement;
	do
	{
		pIcon = ic->data;
		
		if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			cairo_save (pCairoContext);
			cairo_dock_render_one_icon (pIcon, pDock, pCairoContext, fDockMagnitude, pIcon->bPointed);
			cairo_restore (pCairoContext);
		}
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
}



static void cd_render_optimized (cairo_t *pCairoContext, CairoDock *pDock, GdkRectangle *pArea)
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

	if (pDock->container.bIsHorizontal)
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pArea->width, pDock->iDecorationsHeight);
	else
		cairo_rectangle (pCairoContext, fDockOffsetX, fDockOffsetY, pDock->iDecorationsHeight, pArea->height);

	fDockOffsetY = (pDock->container.bDirectionUp ? pDock->container.iHeight - pDock->iDecorationsHeight - fLineWidth : fLineWidth);
	
	double fRadius = MIN (myDocksParam.iDockRadius, (pDock->iDecorationsHeight + myDocksParam.iDockLineWidth) / 2 - 1);
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
	double fDeltaXTrapeze = fRadius;
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY, fOffsetX - fDeltaXTrapeze, fDockWidth + 2*fDeltaXTrapeze);
	
	//\____________________ On dessine la partie du cadre qui va bien.
	cairo_new_path (pCairoContext);

	if (pDock->container.bIsHorizontal)
	{
		cairo_move_to (pCairoContext, fDockOffsetX, fDockOffsetY - fLineWidth / 2);
		cairo_rel_line_to (pCairoContext, pArea->width, 0);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
		cairo_stroke (pCairoContext);

		cairo_new_path (pCairoContext);
		cairo_move_to (pCairoContext, fDockOffsetX, (pDock->container.bDirectionUp ? iHeight - fLineWidth / 2 : pDock->iDecorationsHeight + 1.5 * fLineWidth));
		cairo_rel_line_to (pCairoContext, pArea->width, 0);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
	}
	else
	{
		cairo_move_to (pCairoContext, fDockOffsetX - fLineWidth / 2, fDockOffsetY);
		cairo_rel_line_to (pCairoContext, 0, pArea->height);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
		cairo_stroke (pCairoContext);

		cairo_new_path (pCairoContext);
		cairo_move_to (pCairoContext, (pDock->container.bDirectionUp ? iHeight - fLineWidth / 2 : pDock->iDecorationsHeight + 1.5 * fLineWidth), fDockOffsetY);
		cairo_rel_line_to (pCairoContext, 0, pArea->height);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
	}
	cairo_stroke (pCairoContext);

	cairo_restore (pCairoContext);

	//\____________________ On dessine les icones impactees.
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);

	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement != NULL)
	{
		double fXMin = (pDock->container.bIsHorizontal ? pArea->x : pArea->y), fXMax = (pDock->container.bIsHorizontal ? pArea->x + pArea->width : pArea->y + pArea->height);
		double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
		double fRatio = pDock->container.fRatio;
		double fXLeft, fXRight;
		
		//g_print ("redraw [%d -> %d]\n", (int) fXMin, (int) fXMax);
		Icon *icon;
		GList *ic = pFirstDrawnElement;
		do
		{
			icon = ic->data;

			fXLeft = icon->fDrawX + icon->fScale + 1;
			fXRight = icon->fDrawX + (icon->fWidth - 1) * icon->fScale * icon->fWidthFactor - 1;

			if (fXLeft < fXMax && fXRight > fXMin)
			{
				cairo_save (pCairoContext);
				//g_print ("dessin optimise de %s [%.2f -> %.2f]\n", icon->cName, fXLeft, fXRight);
				
				icon->fAlpha = 1;
				if (icon->iAnimationState == CAIRO_DOCK_STATE_AVOID_MOUSE)
				{
					icon->fAlpha = 0.7;
				}
				
				cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, icon->bPointed);
				cairo_restore (pCairoContext);
			}

			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
	}
}


static void cd_render_opengl (CairoDock *pDock)
{
	//\_____________ On definit notre rectangle.
	double fLineWidth = myDocksParam.iDockLineWidth;
	double fMargin = myDocksParam.iFrameMargin;
	double fRadius = (pDock->iDecorationsHeight + fLineWidth - 2 * myDocksParam.iDockRadius > 0 ? myDocksParam.iDockRadius : (pDock->iDecorationsHeight + fLineWidth) / 2 - 1);
	double fExtraWidth = 2 * fRadius + fLineWidth;
	double fDockWidth;
	double fFrameHeight = pDock->iDecorationsHeight + fLineWidth;
	
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement == NULL)
		return ;
	if (cairo_dock_is_extended_dock (pDock))  // mode panel etendu.
	{
		fDockWidth = pDock->container.iWidth - fExtraWidth;
		fDockOffsetX = fLineWidth / 2;
	}
	else
	{
		fDockWidth = cairo_dock_get_current_dock_width_linear (pDock);
		Icon *pFirstIcon = pFirstDrawnElement->data;
		fDockOffsetX = (pFirstIcon != NULL ? pFirstIcon->fX + 0 - fMargin - fRadius : fLineWidth / 2);
		if (fDockOffsetX - fLineWidth/2 < 0)
			fDockOffsetX = fLineWidth / 2;
		if (fDockOffsetX + fDockWidth + (2*fRadius + fLineWidth) > pDock->container.iWidth)
			fDockWidth = pDock->container.iWidth - fDockOffsetX - (2*fRadius + fLineWidth);
	}
	
	fDockOffsetY = pDock->iDecorationsHeight + 1.5 * fLineWidth;
	
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	
	//\_____________ On genere les coordonnees du contour.
	const CairoDockGLPath *pFramePath = cairo_dock_generate_rectangle_path (fDockWidth, fFrameHeight, fRadius, myDocksParam.bRoundedBottomCorner);
	
	//\_____________ On remplit avec le fond.
	glPushMatrix ();
	cairo_dock_set_container_orientation_opengl (CAIRO_CONTAINER (pDock));
	glTranslatef (fDockOffsetX + (fDockWidth+2*fRadius)/2,
		fDockOffsetY - fFrameHeight/2,
		0.);
	
	_cairo_dock_set_blend_source ();
	cairo_dock_fill_gl_path (pFramePath, pDock->backgroundBuffer.iTexture);
	
	//\_____________ On trace le contour.
	if (fLineWidth != 0)
	{
		glLineWidth (fLineWidth);
		glColor4f (myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
		cairo_dock_stroke_gl_path (pFramePath, TRUE);
	}
	glPopMatrix ();
	
	
	//\_____________ On trace les separateurs physiques.
	glPushMatrix ();
	cairo_dock_set_container_orientation_opengl (CAIRO_CONTAINER (pDock));
	
	double x1, x2, dx, delta, h = pDock->iDecorationsHeight + 2*fLineWidth, h_ = h - fLineWidth;
	GList *ic;
	Icon *pIcon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			x1 = pIcon->fDrawX = pIcon->fX;
			
			pIcon = NULL;
			for (ic = ic->next; ic != NULL; ic = ic->next)
			{
				pIcon = ic->data;
				if (!CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
					break;
			}
			if (ic != NULL)
			{
				pIcon = ic->data;
				x2 = pIcon->fDrawX;
			}
			else
				break;
			
			CairoDockGLPath *pPath = cairo_dock_new_gl_path (2*(iNbCurveSteps+1) + 1, (x1+x2)/2, h, 0., 0.);  // on part du milieu en haut pour que les triangles soient contenus dans l'enveloppe.
			
			dx = MIN (my_fPanelRadius, (x2 - x1) / 2);
			delta = dx + h*tan(my_fPanelInclination)/2;
			if (delta > (x2 - x1) / 2)
				delta = (x2 - x1) / 2;
			
			cairo_dock_gl_path_rel_line_to (pPath,
				-(x2-x1)/2, 0.);
			cairo_dock_gl_path_rel_curve_to (pPath, iNbCurveSteps,
				dx, 0.,
				delta - dx, -h,
				delta, -h);
			cairo_dock_gl_path_rel_line_to (pPath,
				x2 - x1 - 2*delta, 0.);
			cairo_dock_gl_path_rel_curve_to (pPath, iNbCurveSteps,
				dx, 0.,
				delta - dx, h,
				delta, h);
			
			glBlendFunc (GL_ONE, GL_ZERO);
			glColor4f (0., 0., 0., 0.);
			cairo_dock_fill_gl_path (pPath, 0);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			if (fLineWidth > 0)
			{
				cairo_dock_gl_path_move_to (pPath, x1, h - fLineWidth/2);  // on part du haut/gauche, le nombre de points est ok.
				cairo_dock_gl_path_rel_curve_to (pPath, iNbCurveSteps,
					dx, 0.,
					delta - dx, -h_,
					delta, -h_);
				cairo_dock_gl_path_rel_line_to (pPath,
					x2 - x1 - 2*delta, 0.);
				cairo_dock_gl_path_rel_curve_to (pPath, iNbCurveSteps,
					dx, 0.,
					delta - dx, h_,
					delta, h_);
				glLineWidth (fLineWidth);
				glColor4f (myDocksParam.fLineColor[0], myDocksParam.fLineColor[1], myDocksParam.fLineColor[2], myDocksParam.fLineColor[3]);
				cairo_dock_stroke_gl_path (pPath, FALSE);
			}
			
			cairo_dock_free_gl_path (pPath);
		}
	}
	glPopMatrix ();
	
	
	//\_____________ On dessine la ficelle.
	if (myIconsParam.iStringLineWidth > 0)
		cairo_dock_draw_string_opengl (pDock, myIconsParam.iStringLineWidth, FALSE, FALSE);
	
	
	//\_____________ On dessine les icones.
	pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
	
	ic = pFirstDrawnElement;
	do
	{
		pIcon = ic->data;
		
		if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			glPushMatrix ();
			cairo_dock_render_one_icon_opengl (pIcon, pDock, fDockMagnitude, pIcon->bPointed);
			glPopMatrix ();
		}
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
	//glDisable (GL_LIGHTING);
}


static Icon *cd_calculate_icons (CairoDock *pDock)
{
	//\_____________ On calcule le nombre de groupes et la place qu'ils occupent.
	int iNbGroups = 1;
	double fCurrentGroupWidth = - myIconsParam.iIconGap, fGroupsWidth = 0.;
	double fSeparatorsPonderation = 0;
	GList *ic;
	Icon *pIcon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			pIcon->fScale = 1.;
			if (pIcon->fInsertRemoveFactor != 0)
			{
				if (pIcon->fInsertRemoveFactor > 0)
					pIcon->fScale *= pIcon->fInsertRemoveFactor;
				else
					pIcon->fScale *= (1 + pIcon->fInsertRemoveFactor);
			}
			
			if (fCurrentGroupWidth > 0)  // le groupe courant est non vide, sinon c'est juste 2 separateurs cote a cote.
			{
				iNbGroups ++;
				fSeparatorsPonderation += pIcon->fScale;
				fGroupsWidth += MAX (0, fCurrentGroupWidth);
				fCurrentGroupWidth = - myIconsParam.iIconGap;
			}
			
			continue;
		}
		
		pIcon->fScale = my_fPanelRatio;
		if (pIcon->fInsertRemoveFactor != 0)
		{
			if (pIcon->fInsertRemoveFactor > 0)
				pIcon->fScale *= pIcon->fInsertRemoveFactor;
			else
				pIcon->fScale *= (1 + pIcon->fInsertRemoveFactor);
		}
		
		fCurrentGroupWidth += pIcon->fWidth * pIcon->fScale + myIconsParam.iIconGap;
	}
	if (fCurrentGroupWidth > 0)  // le groupe courant est non vide, sinon c'est juste un separateur a la fin.
	{
		fGroupsWidth += MAX (0, fCurrentGroupWidth);
	}
	if (fGroupsWidth < 0)
		fGroupsWidth = 0;
	
	//\_____________ On en deduit l'ecart entre les groupes d'icones.
	double W = cairo_dock_get_max_authorized_dock_width (pDock);
	double fScreenBorderGap = myDocksParam.iDockRadius + myDocksParam.iDockLineWidth;  // on laisse un ecart avec le bord de l'ecran.
	double fGroupGap;
	if (iNbGroups > 1)
	{
		fGroupGap = (W - 2*fScreenBorderGap - fGroupsWidth) / (iNbGroups - 1);
		if (fSeparatorsPonderation != 0 && iNbGroups > 2)
			fGroupGap /= fSeparatorsPonderation / (iNbGroups - 1);
	}
	else
		fGroupGap = W - fScreenBorderGap - fGroupsWidth;
	
	//\_____________ On determine la position de chaque icone.
	Icon *pPointedIcon = NULL;
	double xm = pDock->container.iMouseX;
	double xg = fScreenBorderGap;  // abscisse de l'icone courante, et abscisse du debut du groupe courant.
	double x = xg;
	fCurrentGroupWidth = - myIconsParam.iIconGap;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			pIcon->fX = x;
			pIcon->fDrawX = pIcon->fX;
			
			if (pDock->container.bDirectionUp)
				pIcon->fY = pDock->iMaxDockHeight - pDock->iMinDockHeight;
			else
				pIcon->fY = pDock->iMinDockHeight;
			pIcon->fDrawY = pIcon->fY;
			
			pIcon->fWidthFactor = 0;
			
			if (fCurrentGroupWidth > 0)  // le groupe courant est non vide, sinon c'est juste 2 separateurs cote a cote.
			{
				xg += fCurrentGroupWidth + fGroupGap * pIcon->fScale;
				if (pPointedIcon == NULL && xm > x && xm < xg)
				{
					pIcon->bPointed = TRUE;
					pPointedIcon = pIcon;
				}
				else
					pIcon->bPointed = FALSE;
				x = xg;
				fCurrentGroupWidth = - myIconsParam.iIconGap;
			}
			
			continue;
		}
		
		fCurrentGroupWidth += pIcon->fWidth * pIcon->fScale + myIconsParam.iIconGap;
		
		pIcon->fX = x;
		if (pPointedIcon == NULL && xm > pIcon->fX - .5*myIconsParam.iIconGap && xm <= pIcon->fX + pIcon->fWidth * pIcon->fScale + .5*myIconsParam.iIconGap)
		{
			pIcon->bPointed = TRUE;
			pPointedIcon = pIcon;
		}
		else
			pIcon->bPointed = FALSE;
		pIcon->fDrawX = pIcon->fX;
		
		if (pDock->container.bDirectionUp)
			pIcon->fY = pDock->iMaxDockHeight - (myDocksParam.iDockLineWidth + myDocksParam.iFrameMargin + pIcon->fHeight * my_fPanelRatio);
		else
			pIcon->fY = myDocksParam.iDockLineWidth + myDocksParam.iFrameMargin;
		pIcon->fDrawY = pIcon->fY;
		
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		pIcon->fOrientation = 0.;
		pIcon->fAlpha = 1.;
		
		x += pIcon->fWidth * pIcon->fScale + myIconsParam.iIconGap;
	}
	
	cairo_dock_check_if_mouse_inside_linear (pDock);
	
	cairo_dock_check_can_drop_linear (pDock);
	
	return pPointedIcon;
}


void cd_update_input_shape (CairoDock *pDock)
{
	if (pDock->pShapeBitmap != NULL)
	{
		CDPanelData *pData = pDock->pRendererData;
		g_return_if_fail (pData != NULL);
		
		#if (GTK_MAJOR_VERSION < 3)
		cairo_t *pCairoContext = gdk_cairo_create (pDock->pShapeBitmap);
		if  (pCairoContext != NULL)
		{
			cairo_set_source_rgba (pCairoContext, 0.0f, 0.0f, 0.0f, 0.0f);
			cairo_set_operator (pCairoContext, CAIRO_OPERATOR_SOURCE);
			
			GList *ic;
			Icon *pIcon;
			for (ic = pDock->icons; ic != NULL; ic = ic->next)
			{
				pIcon = ic->data;
				if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
				{
					if (pDock->container.bIsHorizontal)
					{
						cairo_rectangle (pCairoContext,
							pIcon->fXAtRest + 2 * my_fPanelRadius,  // we let a few pixels to be able to grab the separtator, and to avoid leaving the dock too easily.
							0,
							pData->fGroupGap - 4 * my_fPanelRadius,
							pDock->iMaxDockHeight);  // we use iMaxDockHeight instead of the actual window size, because at this time, the dock's window may not have its definite size.
					}
					else
					{
						cairo_rectangle (pCairoContext,
							0,
							pIcon->fXAtRest + 2 * my_fPanelRadius,  // we let a few pixels to be able to grab the separtator, and to avoid leaving the dock too easily.
							pDock->iMaxDockHeight,
							pData->fGroupGap - 4 * my_fPanelRadius);
					}
					cairo_fill (pCairoContext);
				}
			}
			
			cairo_destroy (pCairoContext);
		}
		#else
		/// TODO: add the rectangles to the region...
		cairo_rectangle_int_t rect;
		GList *ic;
			Icon *pIcon;
			for (ic = pDock->icons; ic != NULL; ic = ic->next)
			{
				pIcon = ic->data;
				if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
				{
					if (pDock->container.bIsHorizontal)
					{
						rect.x = pIcon->fXAtRest + 2 * my_fPanelRadius;  // we let a few pixels to be able to grab the separtator, and to avoid leaving the dock too easily.
						rect.y = 0;
						rect.width = pData->fGroupGap - 4 * my_fPanelRadius;
						rect.height = pDock->iMaxDockHeight;  // we use iMaxDockHeight instead of the actual window size, because at this time, the dock's window may not have its definite size.
					}
					else
					{
						rect.x = 0;
						rect.y = pIcon->fXAtRest + 2 * my_fPanelRadius;  // we let a few pixels to be able to grab the separtator, and to avoid leaving the dock too easily.
						rect.width = pDock->iMaxDockHeight;
						rect.height = pData->fGroupGap - 4 * my_fPanelRadius;
					}
					cairo_region_subtract_rectangle (pDock->pShapeBitmap, &rect);
				}
			}
		#endif
	}
}

void cd_rendering_register_panel_renderer (const gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	// interface
	pRenderer->compute_size = cd_compute_size;
	pRenderer->calculate_icons = cd_calculate_icons;
	pRenderer->render = cd_render;
	pRenderer->render_optimized = cd_render_optimized;
	pRenderer->render_opengl = cd_render_opengl;
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;
	pRenderer->update_input_shape = cd_update_input_shape;
	// parametres
	pRenderer->bUseReflect = FALSE;
	pRenderer->cDisplayedName = D_ (cRendererName);
	pRenderer->cReadmeFilePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/readme-panel-view");
	pRenderer->cPreviewFilePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/preview-panel.jpg");
	
	cairo_dock_register_renderer (cRendererName, pRenderer);
}
