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

extern double g_fSubDockSizeRatio;

extern gint g_iScreenWidth[2];
extern gint g_iScreenHeight[2];
extern gint g_iMaxAuthorizedWidth;

extern gint g_iDockLineWidth;
extern gint g_iDockRadius;
extern double g_fLineColor[4];
extern gint g_iFrameMargin;
extern gint g_iStringLineWidth;
extern double g_fStringColor[4];

extern gboolean g_bDirectionUp;
extern double g_fAmplitude;
extern int g_iLabelSize;

extern double my_rendering_fParabolePower;
extern double my_rendering_fParaboleFactor;


void cd_rendering_set_subdock_position_parabole (Icon *pPointedIcon, CairoDock *pDock)
{
	CairoDock *pSubDock = pPointedIcon->pSubDock;
	int iMouseX = pDock->iMouseX;
	int iX = iMouseX + (-iMouseX + pPointedIcon->fDrawX + pPointedIcon->fWidth * pPointedIcon->fScale / 2) / 2;
	
	if (pDock->iWindowPositionX + pPointedIcon->fDrawX < g_iScreenWidth[pDock->bHorizontalDock] / 2)
	{
		pSubDock->fAlign = 0;
		pSubDock->iGapX = (pDock->iGapY + pDock->iMaxDockHeight);
		pSubDock->iGapY = iX + pDock->iWindowPositionX - pSubDock->iMaxDockHeight / 2;
	}
	else
	{
		pSubDock->fAlign = 1;
		pSubDock->iGapX = -(pDock->iGapY + pDock->iMaxDockHeight);
		pSubDock->iGapY = g_iScreenWidth[pDock->bHorizontalDock] - (iX + pDock->iWindowPositionX) - pSubDock->iMaxDockHeight / 2;
	}
}


void cd_rendering_calculate_max_dock_size_parabole (CairoDock *pDock)
{
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->iFlatDockWidth, pDock->iScrollOffset);
	pDock->iMaxDockWidth = ceil (cairo_dock_calculate_max_dock_width (pDock, pDock->pFirstDrawnElement, pDock->iFlatDockWidth, 1.)) + 1;
	pDock->iMaxDockWidth = MIN (pDock->iMaxDockWidth, g_iMaxAuthorizedWidth);
	
	int iParabolicDeviation = my_rendering_fParaboleFactor * pDock->iMaxDockWidth;
	pDock->iMaxDockHeight = iParabolicDeviation + pDock->iMaxIconHeight;  // pDock->iMaxIconHeight/2 en haut et en bas.
	
	pDock->iDecorationsWidth = 0;
	pDock->iDecorationsHeight = 0;
	
	pDock->iMinDockWidth = pDock->iFlatDockWidth + 2 * g_iDockRadius + 2 * g_iFrameMargin + g_iDockLineWidth;  // A voir ...
	pDock->iMinDockHeight = pDock->iMaxIconHeight + 2 * g_iFrameMargin + 2 * g_iDockLineWidth;
}


void cd_rendering_calculate_construction_parameters_parabole (Icon *icon, int iCurrentWidth, int iCurrentHeight, int iFlatDockWidth, gboolean bDirectionUp)
{
	double fXIconCenter = icon->fX + icon->fWidth * icon->fScale / 2;  // abscisse du centre de l'icone.
	g_print ("fXIconCenter : %.2f / %d\n", fXIconCenter, iCurrentWidth);
	//double fYIconCenter = my_rendering_fParaboleFactor * iCurrentWidth * pow (fXIconCenter / iCurrentWidth, my_rendering_fParabolePower);
	double fYIconCenter = my_rendering_fParaboleFactor * pow (pow (1.*iCurrentWidth, 1./my_rendering_fParabolePower) - pow (iCurrentWidth - fXIconCenter, 1./my_rendering_fParabolePower), my_rendering_fParabolePower);
	if (bDirectionUp)
		fYIconCenter = iCurrentHeight - fYIconCenter;
	
	icon->fWidthFactor = 1.;
	icon->fHeightFactor = 1.;
	
	icon->fDrawX = icon->fX;
	if (icon->fDrawX >= 0 && icon->fDrawX + icon->fWidth * icon->fScale <= iCurrentWidth)
		icon->fAlpha = 1;
	else
		icon->fAlpha = .25;
	
	icon->fDrawY = fYIconCenter  - icon->fHeight * icon->fScale / 2;
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
	
	//\____________________ On trace le cadre.
	
	
	//\____________________ On dessine les decorations dedans.
	
	//\____________________ On dessine le cadre.
	
	
	//\____________________ On dessine la ficelle qui les joint.
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	if (g_iStringLineWidth > 0)
		cairo_dock_draw_string (pCairoContext, pDock, g_iStringLineWidth, TRUE);
	
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


Icon *cd_rendering_calculate_icons_parabole (CairoDock *pDock)
{
	Icon *pPointedIcon = cairo_dock_apply_wave_effect (pDock);
	
	CairoDockMousePositionType iMousePositionType = cd_rendering_check_if_mouse_inside_parabole (pDock);
	
	cairo_dock_manage_mouse_position (pDock, iMousePositionType);
	
	//\____________________ On calcule les position/etirements/alpha des icones.
	cd_rendering_check_if_mouse_inside_parabole (pDock);
	
	Icon* icon;
	GList* ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cd_rendering_calculate_construction_parameters_parabole (icon, pDock->iCurrentWidth, pDock->iCurrentHeight, pDock->iFlatDockWidth, g_bDirectionUp);
		cairo_dock_manage_animations (icon, pDock);
	}
	
	return (iMousePositionType == CAIRO_DOCK_MOUSE_INSIDE ? pPointedIcon : NULL);
}


void cd_rendering_register_parabole_renderer (void)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	pRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-parabolic-view", CD_RENDERING_SHARE_DATA_DIR);
	pRenderer->calculate_max_dock_size = cd_rendering_calculate_max_dock_size_parabole;
	pRenderer->calculate_icons = cd_rendering_calculate_icons_parabole;  // cairo_dock_apply_wave_effect;
	pRenderer->render = cd_rendering_render_parabole;
	pRenderer->render_optimized = NULL;
	pRenderer->set_subdock_position = cd_rendering_set_subdock_position_parabole;
	
	cairo_dock_register_renderer (CD_RENDERING_PARABOLIC_VIEW_NAME, pRenderer);
}
