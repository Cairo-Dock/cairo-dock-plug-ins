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

extern double my_rendering_fParabolePower;
extern double my_rendering_fParaboleFactor;


void cd_rendering_set_subdock_position_parabole (Icon *pPointedIcon, CairoDock *pDock)
{
	CairoDock *pSubDock = pPointedIcon->pSubDock;
	int iMouseX = pDock->iMouseX;
	//int iX = iMouseX + (-iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2) / 2;
	int iX = iMouseX;
	
	if (pDock->iWindowPositionX + pPointedIcon->fDrawX < g_iScreenWidth[pDock->bHorizontalDock] / 2)
	{
		iX = iMouseX + (-iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2) / 2;
		pSubDock->fAlign = 0;
		pSubDock->iGapY = (pDock->iGapY + pDock->iMaxDockHeight);
		pSubDock->iGapX = iX + pDock->iWindowPositionX - pSubDock->iMaxDockHeight / 2;
	}
	else
	{
		iX = iMouseX + pPointedIcon->fWidth;
		pSubDock->fAlign = 1;
		pSubDock->iGapY = (pDock->iGapY + pDock->iMaxDockHeight);
		pSubDock->iGapX =  pDock->iWindowPositionX + iX - g_iScreenWidth[pDock->bHorizontalDock];
	}
}


void cd_rendering_calculate_max_dock_size_parabole (CairoDock *pDock)
{
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->iFlatDockWidth, pDock->iScrollOffset);
	//pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->iFlatDockWidth, 1., 0));
	GList* ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->fXMax = icon->fXAtRest + 2*icon->fWidth;
		icon->fXMin = icon->fXAtRest - 2*icon->fWidth;
	}
	
	pDock->iMaxDockWidth = MIN (pDock->iFlatDockWidth, g_iMaxAuthorizedWidth) + 10;
	
	int iParabolicDeviation = my_rendering_fParaboleFactor * pDock->iMaxDockWidth;
	pDock->iMaxDockHeight = iParabolicDeviation + pDock->iMaxIconHeight * (1 + 0) + g_iLabelSize;  // pDock->iMaxIconHeight/2 en haut et en bas.
	
	pDock->iDecorationsWidth = 0;
	pDock->iDecorationsHeight = 0;
	
	pDock->iMinDockWidth = pDock->iFlatDockWidth;
	pDock->iMinDockHeight = pDock->iMaxIconHeight;
}


void cd_rendering_calculate_construction_parameters_parabole (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iFlatDockWidth, gboolean bDirectionUp, double fAlign, gboolean bHorizontalDock)
{
	double fXIconCenter = icon->fX + icon->fWidth * icon->fScale / 2;  // abscisse du centre de l'icone.
	//g_print ("fXIconCenter : %.2f / %d\n", fXIconCenter, iCurrentWidth);
	//double fYIconCenter = my_rendering_fParaboleFactor * iCurrentWidth * pow (fXIconCenter / iCurrentWidth, my_rendering_fParabolePower);
	double fMaxYIconCenter = pow (1.*iCurrentWidth, 1./my_rendering_fParabolePower);
	double fYIconCenter = my_rendering_fParaboleFactor * pow (fMaxYIconCenter - pow ((fAlign == 0 ? iCurrentWidth - fXIconCenter : fXIconCenter), 1./my_rendering_fParabolePower), my_rendering_fParabolePower);
	//if (fAlign == 1.)
	//	fYIconCenter = fMaxYIconCenter - fYIconCenter;
	if (bDirectionUp)
		fYIconCenter = iCurrentHeight - fYIconCenter;
	
	icon->fWidthFactor = 1.;
	icon->fHeightFactor = 1.;
	
	icon->fDrawX = icon->fX;
	if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= iCurrentWidth)
		icon->fAlpha = 1;
	else
		icon->fAlpha = .85;
	
	icon->fDrawY = fYIconCenter - icon->fHeight * icon->fScale / 2;
	icon->fOrientation = .7;
}



void cd_rendering_render_parabole (CairoDock *pDock)
{
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
	
	//\____________________ On dessine les icones et les etiquettes.
	double fRatio = (pDock->iRefCount == 0 ? 1 : g_fSubDockSizeRatio);
	cairo_dock_render_icons_linear (pCairoContext, pDock, fRatio);
	
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
	int x_abs = dx + pDock->iFlatDockWidth / 2;  // ecart par rapport a la gauche du dock minimal  plat.
	
	//\_______________ On calcule l'ensemble des parametres des icones.
	double fMagnitude = 0.;
	Icon *pPointedIcon = cairo_dock_calculate_wave_with_position_linear (pDock->icons, pDock->pFirstDrawnElement, x_abs, fMagnitude, pDock->iFlatDockWidth, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->fAlign, pDock->fFoldingFactor);
	return pPointedIcon;
}

Icon *cd_rendering_calculate_icons_parabole (CairoDock *pDock)
{
	Icon *pPointedIcon = cairo_dock_apply_no_wave_effect (pDock);
	
	CairoDockMousePositionType iMousePositionType = cd_rendering_check_if_mouse_inside_parabole (pDock);
	
	cairo_dock_manage_mouse_position (pDock, iMousePositionType);
	
	//\____________________ On calcule les position/etirements/alpha des icones.
	cd_rendering_check_if_mouse_inside_parabole (pDock);
	
	Icon* icon;
	GList* ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cd_rendering_calculate_construction_parameters_parabole (icon, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iFlatDockWidth, g_bDirectionUp, pDock->fAlign, pDock->bHorizontalDock);
		cairo_dock_manage_animations (icon, pDock);
	}
	
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
	
	cairo_dock_register_renderer (MY_APPLET_PARABOLIC_VIEW_NAME, pRenderer);
}
