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

#include <rendering-caroussel.h>

extern double my_fInclinationOnHorizon;

extern double my_fForegroundRatio;
extern double my_iGapOnEllipse;
extern gboolean my_bRotateIconsOnEllipse;


/*void cd_rendering_set_subdock_position_caroussel (Icon *pPointedIcon, CairoDock *pDock)
{
	CairoDock *pSubDock = pPointedIcon->pSubDock;
	int iMouseX = pDock->iMouseX;
	int iX = iMouseX + (-iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2) / 2;
	//int iX = iMouseX + (iMouseX < pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2 ? (pDock->bDirectionUp ? 1 : 0) : (pDock->bDirectionUp ? 0 : -1)) * pPointedIcon->fWidth * pPointedIcon->fScale / 2;
	if (pSubDock->bHorizontalDock == pDock->bHorizontalDock)
	{
		pSubDock->fAlign = 0.5;
		pSubDock->iGapX = iX + pDock->iWindowPositionX - g_iScreenWidth[pDock->bHorizontalDock] / 2;  // les sous-dock ont un alignement egal a 0.5.  // pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2
		pSubDock->iGapY = pDock->iGapY + pDock->iMaxDockHeight;
	}
	else
	{
		pSubDock->fAlign = (pDock->bDirectionUp ? 1 : 0);
		pSubDock->iGapX = (pDock->iGapY + pDock->iMaxDockHeight) * (pDock->bDirectionUp ? -1 : 1);
		if (pDock->bDirectionUp)
			pSubDock->iGapY = g_iScreenWidth[pDock->bHorizontalDock] - (iX + pDock->iWindowPositionX) - pSubDock->iMaxDockHeight / 2;  // les sous-dock ont un alignement egal a 1.
		else
			pSubDock->iGapY = iX + pDock->iWindowPositionX - pSubDock->iMaxDockHeight / 2;  // les sous-dock ont un alignement egal a 0.
	}
}*/


void cd_rendering_calculate_max_dock_size_caroussel (CairoDock *pDock)
{
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->fFlatDockWidth, pDock->iScrollOffset);
	
	int iEllipseHeight = (1 + g_fAmplitude) * pDock->iMaxIconHeight / sqrt (1 + my_fInclinationOnHorizon * my_fInclinationOnHorizon) + my_iGapOnEllipse;
	pDock->iDecorationsHeight = iEllipseHeight + 2 * myBackground.iFrameMargin + myIcons.fReflectSize;
	
	double fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (pDock->iDecorationsHeight, my_fInclinationOnHorizon, myBackground.iDockRadius, myBackground.iDockLineWidth);
	pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->fFlatDockWidth, my_fForegroundRatio, fExtraWidth));  // fExtraWidth/2 de chaque cote.
	///pDock->iMaxDockWidth = MIN (pDock->iMaxDockWidth, g_iMaxAuthorizedWidth);
	
	pDock->iMaxDockHeight = myBackground.iDockLineWidth + myBackground.iFrameMargin + myIcons.fReflectSize + iEllipseHeight + pDock->iMaxIconHeight;  // de bas en haut;
	pDock->iMaxDockHeight = MAX (pDock->iMaxDockHeight, myBackground.iDockLineWidth + myBackground.iFrameMargin + (1 + g_fAmplitude) * pDock->iMaxIconHeight + myIcons.fReflectSize + myLabels.iconTextDescription.iSize);
	
	pDock->iDecorationsWidth = pDock->iMaxDockWidth;
	
	pDock->iMinDockHeight = pDock->iMaxIconHeight + myIcons.fReflectSize + 2 * myBackground.iFrameMargin + 2 * myBackground.iDockLineWidth;
	
	fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (pDock->iMinDockHeight, my_fInclinationOnHorizon, myBackground.iDockRadius, myBackground.iDockLineWidth);
	pDock->iMinDockWidth = MIN (pDock->iMaxDockWidth, pDock->fFlatDockWidth + fExtraWidth);
}


void cd_rendering_calculate_construction_parameters_caroussel (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iMaxIconHeight, int iMaxIconWidth, int iEllipseHeight, gboolean bDirectionUp, double fExtraWidth, double fLinearWidth, double fXFirstIcon)
{
	double fXIconCenter = icon->fX + icon->fWidth * icon->fScale / 2 - fXFirstIcon;  // abscisse du centre de l'icone.
	double fTheta = (fXIconCenter - .5*fLinearWidth) / fLinearWidth * 2 * G_PI;  // changement de repere, dans ]-pi, pi[.
	//g_print ("fXIconCenter : %.2f / %.2f => Theta : %.2f (%dx%d)\n", fXIconCenter, fLinearWidth, fTheta, iCurrentWidth, iCurrentHeight);
	
	double a = .5 * iEllipseHeight;  // parametres de l'ellipse, theta=0 en bas (c'est-a-dire devant nous).
	double b = .5 * (iCurrentWidth - fExtraWidth - (my_bRotateIconsOnEllipse ? 0 : iMaxIconWidth));
	
	double fXIconCenterDraw, fYIconBottomDraw;  // coordonnees du centre bas de l'icone une fois positionnee sur l'ellipse.
	fXIconCenterDraw = b * sin (fTheta) + .5 * iCurrentWidth;
	fYIconBottomDraw = (bDirectionUp ? a * cos (fTheta) + iMaxIconHeight + a : a + myBackground.iDockLineWidth - a * cos (fTheta));
	
	icon->fHeightFactor = 1.;
	icon->fOrientation = 0.;
	
	if (my_bRotateIconsOnEllipse)
		icon->fWidthFactor = (G_PI / 2 - fabs (fTheta)) * 2 / G_PI;
	else
		icon->fWidthFactor = 1.;
	icon->fDrawX = fXIconCenterDraw - icon->fWidth * icon->fScale / 2;  // 'cairo_dock_manage_animations' va gerer le placement de profil.
	
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
	//g_print ("%s : fTheta = %.2f ; fWidthFactor = %.2f ; fDrawX = %.2f\n", icon->acName, fTheta, icon->fWidthFactor, icon->fDrawX);
}


void cd_rendering_render_icons_caroussel (cairo_t *pCairoContext, CairoDock *pDock)
{
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;
	//double fChangeAxes = 0.5 * (pDock->iCurrentWidth - pDock->iMaxDockWidth);
	
	//\____________________ Du debut jusqu'au milieu de la liste.
	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	Icon *icon;
	GList *pLeftElement = pFirstDrawnElement;
	GList *pRightElement = cairo_dock_get_previous_element (pFirstDrawnElement, pDock->icons);
	do
	{
		icon = pLeftElement->data;
		cairo_save (pCairoContext);
		
		//g_print ("redessin a gauche de %s\n", icon->acName);
		cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
		
		cairo_restore (pCairoContext);
		
		if (pLeftElement == pRightElement)
			break;
		
		icon = pRightElement->data;
		cairo_save (pCairoContext);
		
		//g_print ("redessin a droite de %s\n", icon->acName);
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
	double fLineWidth = myBackground.iDockLineWidth;
	double fMargin = myBackground.iFrameMargin;
	///int iEllipseHeight = pDock->iCurrentHeight - myBackground.iDockLineWidth - fMargin - pDock->iMaxIconHeight;  // >0 par construction de iMinDockHeight.
	int iEllipseHeight = pDock->iCurrentHeight - (myBackground.iDockLineWidth + myBackground.iFrameMargin + pDock->iMaxIconHeight + myIcons.fReflectSize);
	int iFrameHeight = iEllipseHeight + 2 * fMargin + myIcons.fReflectSize;
	
	double fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (iFrameHeight, my_fInclinationOnHorizon, myBackground.iDockRadius, myBackground.iDockLineWidth);
	double fDockWidth = pDock->iCurrentWidth - fExtraWidth;
	int sens;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	fDockOffsetX = fExtraWidth / 2;
	if (pDock->bDirectionUp)
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
	double fDeltaXTrapeze = cairo_dock_draw_frame (pCairoContext, myBackground.iDockRadius, fLineWidth, fDockWidth, iFrameHeight, fDockOffsetX, fDockOffsetY, sens, my_fInclinationOnHorizon, pDock->bHorizontalDock);
	
	//\____________________ On dessine les decorations dedans.
	fDockOffsetY = (pDock->bDirectionUp ? pDock->iMaxIconHeight - fMargin : fLineWidth);
	
	cairo_dock_render_decorations_in_frame (pCairoContext, pDock, fDockOffsetY, fDockOffsetX-fDeltaXTrapeze, fDockWidth+2*fDeltaXTrapeze);
	
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
		cairo_dock_draw_string (pCairoContext, pDock, myIcons.iStringLineWidth, TRUE, FALSE);
	
	//\____________________ On dessine les icones et les etiquettes, en tenant compte de l'ordre pour dessiner celles en arriere-plan avant celles en avant-plan.
	cd_rendering_render_icons_caroussel (pCairoContext, pDock);
}


Icon *cd_rendering_calculate_icons_caroussel (CairoDock *pDock)
{
	Icon *pPointedIcon = cairo_dock_apply_wave_effect (pDock);
	
	CairoDockMousePositionType iMousePositionType = (pDock->bInside ? CAIRO_DOCK_MOUSE_INSIDE : CAIRO_DOCK_MOUSE_OUTSIDE);
	
	cairo_dock_manage_mouse_position (pDock, iMousePositionType);
	
	//\____________________ On calcule les position/etirements/alpha des icones.
	cairo_dock_mark_avoiding_mouse_icons_linear (pDock);
	
	int iEllipseHeight = pDock->iCurrentHeight - (myBackground.iDockLineWidth + myBackground.iFrameMargin + pDock->iMaxIconHeight + myIcons.fReflectSize);  // >0 par construction de iMinDockHeight.
	int iFrameHeight = iEllipseHeight + 2 * myBackground.iFrameMargin + myIcons.fReflectSize;
	double fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (iFrameHeight, my_fInclinationOnHorizon, myBackground.iDockRadius, myBackground.iDockLineWidth);
	double fLinearWidth = cairo_dock_get_current_dock_width_linear (pDock);
	Icon *pFirstIcon = cairo_dock_get_first_drawn_icon (pDock);
	double fXFirstIcon = (pFirstIcon != NULL ? pFirstIcon->fX : 0);
	Icon* icon;
	GList* ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cd_rendering_calculate_construction_parameters_caroussel (icon, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iMaxIconHeight, pDock->iMaxIconHeight, iEllipseHeight, pDock->bDirectionUp, fExtraWidth, fLinearWidth, fXFirstIcon);  // il manque un pDock->iMaxIconWidth en 2eme...
		cairo_dock_manage_animations (icon, pDock);
	}
	
	return (iMousePositionType == CAIRO_DOCK_MOUSE_INSIDE ? pPointedIcon : NULL);
}


void cd_rendering_register_caroussel_renderer (const gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	pRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-caroussel-view", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->cPreviewFilePath = g_strdup_printf ("%s/preview-caroussel.png", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->calculate_max_dock_size = cd_rendering_calculate_max_dock_size_caroussel;
	pRenderer->calculate_icons = cd_rendering_calculate_icons_caroussel;  // cairo_dock_apply_wave_effect;
	pRenderer->render = cd_rendering_render_caroussel;
	pRenderer->render_optimized = NULL;
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;  // cd_rendering_set_subdock_position_caroussel
	pRenderer->bUseReflect = TRUE;
	
	cairo_dock_register_renderer (cRendererName, pRenderer);
}
