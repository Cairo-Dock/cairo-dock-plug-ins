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

#include "rendering-caroussel.h"

extern double my_fInclinationOnHorizon;

extern double my_fForegroundRatio;
extern double my_iGapOnEllipse;
extern gboolean my_bRotateIconsOnEllipse;
extern double my_fScrollAcceleration;
extern double my_fScrollSpeed;

/*void cd_rendering_set_subdock_position_caroussel (Icon *pPointedIcon, CairoDock *pDock)
{
	CairoDock *pSubDock = pPointedIcon->pSubDock;
	int iMouseX = pDock->container.iMouseX;
	int iX = iMouseX + (-iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2) / 2;
	//int iX = iMouseX + (iMouseX < pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2 ? (pDock->container.bDirectionUp ? 1 : 0) : (pDock->container.bDirectionUp ? 0 : -1)) * pPointedIcon->fWidth * pPointedIcon->fScale / 2;
	if (pSubDock->container.bIsHorizontal == pDock->container.bIsHorizontal)
	{
		pSubDock->fAlign = 0.5;
		pSubDock->iGapX = iX + pDock->container.iWindowPositionX - g_desktopGeometry.iXScreenWidth[pDock->container.bIsHorizontal] / 2;  // les sous-dock ont un alignement egal a 0.5.  // pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2
		pSubDock->iGapY = pDock->iGapY + pDock->iMaxDockHeight;
	}
	else
	{
		pSubDock->fAlign = (pDock->container.bDirectionUp ? 1 : 0);
		pSubDock->iGapX = (pDock->iGapY + pDock->iMaxDockHeight) * (pDock->container.bDirectionUp ? -1 : 1);
		if (pDock->container.bDirectionUp)
			pSubDock->iGapY = g_desktopGeometry.iXScreenWidth[pDock->container.bIsHorizontal] - (iX + pDock->container.iWindowPositionX) - pSubDock->iMaxDockHeight / 2;  // les sous-dock ont un alignement egal a 1.
		else
			pSubDock->iGapY = iX + pDock->container.iWindowPositionX - pSubDock->iMaxDockHeight / 2;  // les sous-dock ont un alignement egal a 0.
	}
}*/


void cd_rendering_calculate_max_dock_size_caroussel (CairoDock *pDock)
{
	cairo_dock_calculate_icons_positions_at_rest_linear (pDock);
	
	int iEllipseHeight = (1 + myIconsParam.fAmplitude) * pDock->iMaxIconHeight / sqrt (1 + my_fInclinationOnHorizon * my_fInclinationOnHorizon) + my_iGapOnEllipse;
	pDock->iDecorationsHeight = iEllipseHeight + 2 * myDocksParam.iFrameMargin + myIconsParam.fReflectSize;
	
	double fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (pDock->iDecorationsHeight, my_fInclinationOnHorizon, myDocksParam.iDockRadius, myDocksParam.iDockLineWidth);
	pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->fFlatDockWidth, my_fForegroundRatio, fExtraWidth));  // fExtraWidth/2 de chaque cote.
	///pDock->iMaxDockWidth = MIN (pDock->iMaxDockWidth, g_iMaxAuthorizedWidth);
	
	pDock->iMaxDockHeight = myDocksParam.iDockLineWidth + myDocksParam.iFrameMargin + myIconsParam.fReflectSize + iEllipseHeight + pDock->iMaxIconHeight;  // de bas en haut;
	pDock->iMaxDockHeight = MAX (pDock->iMaxDockHeight, myDocksParam.iDockLineWidth + myDocksParam.iFrameMargin + (1 + myIconsParam.fAmplitude) * pDock->iMaxIconHeight + myIconsParam.fReflectSize + myIconsParam.iLabelSize);
	
	pDock->iDecorationsWidth = pDock->iMaxDockWidth;
	
	pDock->iMinDockHeight = pDock->iMaxIconHeight + myIconsParam.fReflectSize + 2 * myDocksParam.iFrameMargin + 2 * myDocksParam.iDockLineWidth;
	
	fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (pDock->iMinDockHeight, my_fInclinationOnHorizon, myDocksParam.iDockRadius, myDocksParam.iDockLineWidth);
	pDock->iMinDockWidth = MIN (pDock->iMaxDockWidth, pDock->fFlatDockWidth + fExtraWidth);
	
	if (pDock->pRendererData == NULL)
	{
		pDock->pRendererData = GINT_TO_POINTER (1);
		gldi_object_register_notification (pDock, NOTIFICATION_UPDATE, (GldiNotificationFunc) cd_rendering_caroussel_update_dock, GLDI_RUN_AFTER, NULL);
	}
	
	if (g_bEasterEggs)
	{
		pDock->iMinDockWidth = pDock->fFlatDockWidth/2;
	}
}


void cd_rendering_calculate_construction_parameters_caroussel (Icon *icon, int iWidth, int iHeight, int iMaxIconHeight, int iMaxIconWidth, int iEllipseHeight, gboolean bDirectionUp, double fExtraWidth, double fLinearWidth, double fXFirstIcon)
{
	double fXIconCenter = icon->fX + icon->fWidth * icon->fScale / 2 - fXFirstIcon;  // abscisse du centre de l'icone.
	double fTheta = (fXIconCenter - .5*fLinearWidth) / fLinearWidth * 2 * G_PI;  // changement de repere, dans ]-pi, pi[.
	
	double a = .5 * iEllipseHeight;  // parametres de l'ellipse, theta=0 en bas (c'est-a-dire devant nous).
	double b = .5 * (iWidth - fExtraWidth - (my_bRotateIconsOnEllipse ? 0 : iMaxIconWidth));
	
	double fXIconCenterDraw, fYIconBottomDraw;  // coordonnees du centre bas de l'icone une fois positionnee sur l'ellipse.
	fXIconCenterDraw = b * sin (fTheta) + .5 * iWidth;
	fYIconBottomDraw = (bDirectionUp ? a * cos (fTheta) + iMaxIconHeight + a : a + myDocksParam.iDockLineWidth - a * cos (fTheta));
	
	icon->fHeightFactor = 1.;
	icon->fOrientation = 0.;
	
	if (my_bRotateIconsOnEllipse)
		icon->fWidthFactor = (G_PI / 2 - fabs (fTheta)) * 2 / G_PI;
	else
		icon->fWidthFactor = 1.;
	icon->fDrawX = fXIconCenterDraw - icon->fWidth * icon->fScale / 2;  /// gerer le placement de profil...
	
	if (fabs (fTheta) < G_PI / 2)  // icone a l'avant plan.
	{
		icon->fDrawX = fXIconCenterDraw - icon->fWidth * icon->fScale / 2;
		icon->fAlpha = 1.;
	}
	else
	{
		icon->fScale *= MAX (0.75, sin ((G_PI - fabs (fTheta)) / 3));
		icon->fAlpha = MAX (0.5, sin (fTheta) * sin (fTheta));
	}
	icon->fDrawY = fYIconBottomDraw  - (bDirectionUp ? icon->fHeight * icon->fScale : 0);
	//g_print ("%s : fTheta = %.2f ; fWidthFactor = %.2f ; fDrawX = %.2f\n", icon->cName, fTheta, icon->fWidthFactor, icon->fDrawX);
}

void cd_rendering_calculate_construction_parameters_caroussel2 (Icon *icon, CairoDock *pDock, int iEllipseHeight, double fExtraWidth, double fLinearWidth)
{
	int iWidth = pDock->container.iWidth;
	int iMaxIconWidth = pDock->iMaxIconHeight;
	int iMaxIconHeight = pDock->iMaxIconHeight;
	gboolean bDirectionUp = pDock->container.bDirectionUp;
	double fTheta = 2*G_PI * icon->fXAtRest / pDock->fFlatDockWidth;
	double a = .5 * iEllipseHeight;  // parametres de l'ellipse, theta=0 en bas (c'est-a-dire devant nous).
	double b = .5 * (iWidth - fExtraWidth - (my_bRotateIconsOnEllipse ? 0 : iMaxIconWidth));
	
	icon->fScale = 1.;
	
	double fXIconCenterDraw, fYIconBottomDraw;  // coordonnees du centre bas de l'icone une fois positionnee sur l'ellipse.
	fXIconCenterDraw = b * sin (fTheta) + .5 * iWidth;
	fYIconBottomDraw = (bDirectionUp ? a * cos (fTheta) + iMaxIconHeight + a : a + myDocksParam.iDockLineWidth - a * cos (fTheta));
	
	icon->fHeightFactor = 1.;
	icon->fOrientation = 0.;
	
	if (my_bRotateIconsOnEllipse)
		icon->fWidthFactor = (G_PI / 2 - fabs (fTheta)) * 2 / G_PI;
	else
		icon->fWidthFactor = 1.;
	icon->fDrawX = fXIconCenterDraw - icon->fWidth * icon->fScale / 2;  /// gerer le placement de profil...
	
	if (fabs (fTheta) < G_PI / 2)  // icone a l'avant plan.
	{
		icon->fDrawX = fXIconCenterDraw - icon->fWidth * icon->fScale / 2;
		icon->fAlpha = 1.;
	}
	else
	{
		icon->fScale *= MAX (0.75, sin ((G_PI - fabs (fTheta)) / 3));
		icon->fAlpha = MAX (0.5, sin (fTheta) * sin (fTheta));
	}
	icon->fDrawY = fYIconBottomDraw  - (bDirectionUp ? icon->fHeight * icon->fScale : 0);
}



void cd_rendering_render_icons_caroussel (cairo_t *pCairoContext, CairoDock *pDock)
{
	GList *pFirstDrawnElement = pDock->icons;
	if (pFirstDrawnElement == NULL)
		return;
	//double fChangeAxes = 0.5 * (pDock->container.iWidth - pDock->iMaxDockWidth);
	
	//\____________________ Du debut jusqu'au milieu de la liste.
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	Icon *icon;
	GList *pLeftElement = pFirstDrawnElement;
	GList *pRightElement = cairo_dock_get_previous_element (pFirstDrawnElement, pDock->icons);
	do
	{
		icon = pLeftElement->data;
		cairo_save (pCairoContext);
		
		//g_print ("redessin a gauche de %s\n", icon->cName);
		cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
		
		cairo_restore (pCairoContext);
		
		if (pLeftElement == pRightElement)
			break;
		
		icon = pRightElement->data;
		cairo_save (pCairoContext);
		
		//g_print ("redessin a droite de %s\n", icon->cName);
		cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
		
		cairo_restore (pCairoContext);
		
		pLeftElement = cairo_dock_get_next_element (pLeftElement, pDock->icons);
		if (pLeftElement == pRightElement)
			break;
		pRightElement = cairo_dock_get_previous_element (pRightElement, pDock->icons);
	}
	while (TRUE);
	//while (icon->fX + icon->fWidth * icon->fScale < 0 && ic != pFirstDrawnElement);  // icon->fScale + fChangeAxes
}


void cd_rendering_render_caroussel (cairo_t *pCairoContext, CairoDock *pDock)
{
	//\____________________ On trace le cadre.
	double fLineWidth = myDocksParam.iDockLineWidth;
	double fMargin = myDocksParam.iFrameMargin;
	///int iEllipseHeight = pDock->container.iHeight - myDocksParam.iDockLineWidth - fMargin - pDock->iMaxIconHeight;  // >0 par construction de iMinDockHeight.
	int iEllipseHeight = pDock->container.iHeight - (myDocksParam.iDockLineWidth + myDocksParam.iFrameMargin + pDock->iMaxIconHeight + myIconsParam.fReflectSize);
	int iFrameHeight = iEllipseHeight + 2 * fMargin + myIconsParam.fReflectSize;
	
	double fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (iFrameHeight, my_fInclinationOnHorizon, myDocksParam.iDockRadius, myDocksParam.iDockLineWidth);
	double fDockWidth = pDock->container.iWidth - fExtraWidth;
	int sens;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	fDockOffsetX = fExtraWidth / 2;
	if (pDock->container.bDirectionUp)
	{
		sens = 1;
		fDockOffsetY = pDock->iMaxIconHeight - fMargin - .5 * fLineWidth;
	}
	else
	{
		sens = -1;
		fDockOffsetY = iFrameHeight + 1.5 * fLineWidth;
	}
	
	cairo_save (pCairoContext);
	double fDeltaXTrapeze = cairo_dock_draw_frame (pCairoContext, myDocksParam.iDockRadius, fLineWidth, fDockWidth, iFrameHeight, fDockOffsetX, fDockOffsetY, sens, my_fInclinationOnHorizon, pDock->container.bIsHorizontal, myDocksParam.bRoundedBottomCorner);
	
	//\____________________ On dessine les decorations dedans.
	fDockOffsetY = (pDock->container.bDirectionUp ? pDock->iMaxIconHeight - fMargin : fLineWidth);
	
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY, fDockOffsetX-fDeltaXTrapeze, fDockWidth+2*fDeltaXTrapeze);
	
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
	
	
	//\____________________ On dessine la ficelle qui les joint.
	if (myIconsParam.iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, myIconsParam.iStringLineWidth, TRUE, FALSE);
	
	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	cd_rendering_render_icons_caroussel (pCairoContext, pDock);
}


static double _cd_rendering_get_rotation_speed (CairoDock *pDock)  // donne la vitesse de rotation entre -1 et 1.
{
	static double a=.2;  // entre -a/2 et a/2 la rotation est nulle.
	double x = 2.*(pDock->container.iMouseX - pDock->container.iWidth/2) / pDock->container.iWidth;  // [-1 ; 1]
	if (x > a)
		return (x - a) / (1 - a);
	else if (x < -a)
		return (x + a) / (1 - a);
	else
		return 0.;
}
Icon *cd_rendering_calculate_icons_caroussel (CairoDock *pDock)
{
	// (x;y) => theta.
	
	Icon *pPointedIcon = cairo_dock_apply_wave_effect (pDock);
	
	//\____________________ On calcule les position/etirements/alpha des icones.
	int iEllipseHeight = pDock->container.iHeight - (myDocksParam.iDockLineWidth + myDocksParam.iFrameMargin + pDock->iMaxIconHeight + myIconsParam.fReflectSize);  // >0 par construction de iMinDockHeight.
	int iFrameHeight = iEllipseHeight + 2 * myDocksParam.iFrameMargin + myIconsParam.fReflectSize;
	double fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (iFrameHeight, my_fInclinationOnHorizon, myDocksParam.iDockRadius, myDocksParam.iDockLineWidth);
	double fLinearWidth = cairo_dock_get_current_dock_width_linear (pDock);
	Icon *pFirstIcon = cairo_dock_get_first_icon (pDock->icons);
	double fXFirstIcon = (pFirstIcon != NULL ? pFirstIcon->fX : 0);
	Icon* icon;
	GList* ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		///cd_rendering_calculate_construction_parameters_caroussel (icon, pDock->container.iWidth, pDock->container.iHeight, pDock->iMaxIconHeight, pDock->iMaxIconHeight, iEllipseHeight, pDock->container.bDirectionUp, fExtraWidth, fLinearWidth, fXFirstIcon);  // il manque un pDock->iMaxIconWidth en 2eme...
		cd_rendering_calculate_construction_parameters_caroussel2 (icon, pDock, iEllipseHeight, fExtraWidth, fLinearWidth);
	}
	
	pDock->iMousePositionType = (pDock->container.bInside ? CAIRO_DOCK_MOUSE_INSIDE : CAIRO_DOCK_MOUSE_OUTSIDE);
	
	cairo_dock_check_can_drop_linear (pDock);  /// marche ?...
	
	if (pDock->container.bInside && ! cairo_dock_container_is_animating (CAIRO_CONTAINER (pDock)))
	{
		double fRotationSpeed = _cd_rendering_get_rotation_speed (pDock);
		if (fRotationSpeed != 0)
			cairo_dock_launch_animation (CAIRO_CONTAINER (pDock));
	}
	
	return pPointedIcon;
}

void cd_rendering_free_caroussel_data (CairoDock *pDock)
{
	if (pDock->pRendererData != NULL)
	{
		gldi_object_remove_notification (CAIRO_CONTAINER (pDock), NOTIFICATION_UPDATE, (GldiNotificationFunc) cd_rendering_caroussel_update_dock, NULL);
	}
}


void cd_rendering_register_caroussel_renderer (const gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	pRenderer->cReadmeFilePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/readme-caroussel-view");
	pRenderer->cPreviewFilePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/preview-caroussel.jpg");
	pRenderer->compute_size = cd_rendering_calculate_max_dock_size_caroussel;
	pRenderer->calculate_icons = cd_rendering_calculate_icons_caroussel;  // cairo_dock_apply_wave_effect;
	pRenderer->render = cd_rendering_render_caroussel;
	pRenderer->render_optimized = NULL;
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;  // cd_rendering_set_subdock_position_caroussel
	pRenderer->free_data = cd_rendering_free_caroussel_data;
	pRenderer->bUseReflect = TRUE;
	pRenderer->cDisplayedName = D_ (cRendererName);
	
	cairo_dock_register_renderer (cRendererName, pRenderer);
}

// TODO !!! @ Fabounet, je n'ai pas vérifié... J'ai juste copié ce que tu avais fait hier :)
static void _scroll_dock_icons (CairoDock *pDock, int iScrollAmount)
{
	if (iScrollAmount == 0)  // fin de scroll
	{
		cairo_dock_trigger_set_WM_icons_geometry (pDock);
		return ;
	}
	
	//\_______________ On fait tourner les icones.
	pDock->iScrollOffset += iScrollAmount;
	if (pDock->iScrollOffset >= pDock->fFlatDockWidth)
		pDock->iScrollOffset -= pDock->fFlatDockWidth;
	if (pDock->iScrollOffset < 0)
		pDock->iScrollOffset += pDock->fFlatDockWidth;
	
	pDock->pRenderer->compute_size (pDock);  // recalcule le pFirstDrawnElement.
	
	//\_______________ On recalcule toutes les icones.
	Icon *pLastPointedIcon = cairo_dock_get_pointed_icon (pDock->icons);
	Icon *pPointedIcon = cairo_dock_calculate_dock_icons (pDock);
	gtk_widget_queue_draw (pDock->container.pWidget);
	
	//\_______________ On gere le changement d'icone.
	if (pPointedIcon != pLastPointedIcon)
	{
		cairo_dock_on_change_icon (pLastPointedIcon, pPointedIcon, pDock);
	}
}

gboolean cd_rendering_caroussel_update_dock (gpointer pUserData, GldiContainer *pContainer, gboolean *bContinueAnimation)
{
	if (! CAIRO_DOCK_IS_DOCK (pContainer))
		return GLDI_NOTIFICATION_LET_PASS;
	CairoDock *pDock = CAIRO_DOCK (pContainer);
	if (pDock->pRenderer->calculate_icons != cd_rendering_calculate_icons_caroussel)
		return GLDI_NOTIFICATION_LET_PASS;
	
	if (pDock->container.bInside)
	{
		double fRotationSpeed = _cd_rendering_get_rotation_speed (pDock);
		int iScrollAmount = ceil (my_fScrollSpeed * fRotationSpeed);
		_scroll_dock_icons (pDock, iScrollAmount);  // avec un scroll de 0, cela termine le scroll.
		*bContinueAnimation |= (fRotationSpeed != 0);
	}
	else if (my_fScrollAcceleration != 0 && pDock->iScrollOffset != 0)  // on de-scrolle.
	{
		int iScrollAmount;
		if (pDock->iScrollOffset < pDock->fFlatDockWidth / 2)
		{
			iScrollAmount = - MAX (2, ceil (pDock->iScrollOffset * my_fScrollAcceleration));
		}
		else
		{
			iScrollAmount = MAX (2, ceil ((pDock->fFlatDockWidth - pDock->iScrollOffset) * my_fScrollAcceleration));
		}
		_scroll_dock_icons (pDock, iScrollAmount);  // avec un scroll de 0, cela termine le scroll.
		*bContinueAnimation |= (pDock->iScrollOffset != 0);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}
