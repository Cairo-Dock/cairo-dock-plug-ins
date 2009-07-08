/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "rendering-desklet-slide.h"

#define _cairo_dock_set_path_as_current(...) _cairo_dock_set_vertex_pointer(pVertexTab)

CDSlideParameters *rendering_configure_slide (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig)  // gboolean, int, gdouble[4], gdouble[4]
{
	GList *pIconsList = pDesklet->icons;
	
	CDSlideParameters *pSlide = g_new0 (CDSlideParameters, 1);
	if (pConfig != NULL)
	{
		pSlide->bRoundedRadius = GPOINTER_TO_INT (pConfig[0]);
		pSlide->iRadius = GPOINTER_TO_INT (pConfig[1]);
		if (pConfig[2] != NULL)
			memcpy (pSlide->fLineColor, pConfig[2], 4 * sizeof (gdouble));
		if (pConfig[3] != NULL)
			memcpy (pSlide->fBgColor, pConfig[3], 4 * sizeof (gdouble));
		pSlide->iLineWidth = 2;
		pSlide->iGapBetweenIcons = 10;
	}
	
	return pSlide;
}


static inline void _compute_icons_grid (CairoDesklet *pDesklet, CDSlideParameters *pSlide)
{
	pSlide->fMargin = (pSlide->bRoundedRadius ?
		.5 * pSlide->iLineWidth + (1. - sqrt (2) / 2) * pSlide->iRadius :
		.5 * pSlide->iLineWidth + .5 * pSlide->iRadius);
	
	pSlide->iNbIcons = g_list_length (pDesklet->icons);
	
	double w = pDesklet->iWidth - 2 * pSlide->fMargin;
	double h = pDesklet->iHeight - 2 * pSlide->fMargin;
	int dh = myLabels.iLabelSize;  // ecart entre 2 lignes.
	int dw = 3 * dh;  // ecart entre 2 colonnes.
	int di = pSlide->iGapBetweenIcons;
	
	int p, q;  // nombre de lignes et colonnes.
	int iSize;
	pSlide->iIconSize = 0, pSlide->iNbLines = 0, pSlide->iNbColumns = 0;
	g_print ("%d icones sur %dx%d (%d)\n", pSlide->iNbIcons, (int)w, (int)h, myLabels.iLabelSize);
	for (p = 1; p <= pSlide->iNbIcons; p ++)
	{
		q = (int) ceil ((double)pSlide->iNbIcons / p);
		iSize = MIN ((h - (p - 1) * di) / p - dh, (w - (q - 1) * di) / q - dw);
		g_print ("  %dx%d -> %d\n", p, q, iSize);
		if (iSize > pSlide->iIconSize)
		{
			pSlide->iIconSize = iSize;
			pSlide->iNbLines = p;
			pSlide->iNbColumns = q;
		}
	}
}

void rendering_load_slide_data (CairoDesklet *pDesklet, cairo_t *pSourceContext)
{
	CDSlideParameters *pSlide = (CDSlideParameters *) pDesklet->pRendererData;
	if (pSlide == NULL)
		return ;
	
	_compute_icons_grid (pDesklet, pSlide);
}


void rendering_free_slide_data (CairoDesklet *pDesklet)
{
	CDSlideParameters *pSlide = (CDSlideParameters *) pDesklet->pRendererData;
	if (pSlide == NULL)
		return ;
	
	g_free (pSlide);
	pDesklet->pRendererData = NULL;
}


void rendering_load_icons_for_slide (CairoDesklet *pDesklet, cairo_t *pSourceContext)
{
	CDSlideParameters *pSlide = (CDSlideParameters *) pDesklet->pRendererData;
	if (pSlide == NULL)
		return ;
	
	_compute_icons_grid (pDesklet, pSlide);
	g_print ("pSlide->iIconSize : %d\n", pSlide->iIconSize);
	
	Icon *pIcon;
	GList* ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		pIcon->fWidth = pSlide->iIconSize;
		pIcon->fHeight = pSlide->iIconSize;

		pIcon->fScale = 1.;
		pIcon->fAlpha = 1.;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		pIcon->fGlideScale = 1.;
		
		cairo_dock_fill_icon_buffers_for_desklet (pIcon, pSourceContext);
	}
}



void rendering_draw_slide_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized)
{
	CDSlideParameters *pSlide = (CDSlideParameters *) pDesklet->pRendererData;
	//g_print ("%s(%x)\n", __func__, pSlide);
	if (pSlide == NULL)
		return ;
	
	double fRadius = pSlide->iRadius;
	double fLineWidth = pSlide->iLineWidth;
	// le cadre.
	cairo_set_line_width (pCairoContext, pSlide->iLineWidth);
	if (pSlide->bRoundedRadius)
	{
		cairo_translate (pCairoContext, 0., .5 * fLineWidth);
		cairo_dock_draw_rounded_rectangle (pCairoContext,
			fRadius,
			fLineWidth,
			pDesklet->iWidth - 2 * fRadius - fLineWidth,
			pDesklet->iHeight - 2*fLineWidth);
		cairo_set_source_rgba (pCairoContext, pSlide->fBgColor[0], pSlide->fBgColor[1], pSlide->fBgColor[2], pSlide->fBgColor[3]);
		cairo_fill_preserve (pCairoContext);
	}
	else
	{
		// le fond
		cairo_move_to (pCairoContext, .5 * pSlide->fMargin, 0.);  // en haut a gauche.
		cairo_rel_line_to (pCairoContext,
			0.,
			pDesklet->iHeight - fRadius - fLineWidth);  // on descend
		cairo_rel_line_to (pCairoContext,
			fRadius,
			fRadius);  // coin bas gauche.
		cairo_rel_line_to (pCairoContext,
			pDesklet->iWidth - fRadius,
			0.);  // vers la droite.
		cairo_rel_line_to (pCairoContext,
			0.,
			- pDesklet->iHeight + fRadius);  // on remonte.
		cairo_rel_line_to (pCairoContext,
			- fRadius,
			- fRadius);  // coin haut droit.
		cairo_close_path (pCairoContext);
		cairo_set_source_rgba (pCairoContext, pSlide->fBgColor[0], pSlide->fBgColor[1], pSlide->fBgColor[2], pSlide->fBgColor[3]);
		cairo_fill (pCairoContext);
		
		// le cadre
		cairo_move_to (pCairoContext, .5 * pSlide->fMargin, 0.);
		cairo_rel_line_to (pCairoContext,
			0.,
			pDesklet->iHeight - fRadius - fLineWidth);
		cairo_rel_line_to (pCairoContext,
			pSlide->iRadius,
			pSlide->iRadius);
		cairo_rel_line_to (pCairoContext,
			pDesklet->iWidth - fRadius - fLineWidth,
			0.);
	}
	cairo_set_source_rgba (pCairoContext, pSlide->fLineColor[0], pSlide->fLineColor[1], pSlide->fLineColor[2], pSlide->fLineColor[3]);
	cairo_stroke (pCairoContext);
	
	// les icones.
	double w = pDesklet->iWidth - 2 * pSlide->fMargin;
	double h = pDesklet->iHeight - 2 * pSlide->fMargin;
	int dh = (h - pSlide->iNbLines * (pSlide->iIconSize + myLabels.iLabelSize)) / (pSlide->iNbLines != 1 ? pSlide->iNbLines - 1 : 1);  // ecart entre 2 lignes.
	int dw = (w - pSlide->iNbColumns * pSlide->iIconSize) / pSlide->iNbColumns;  // ecart entre 2 colonnes.
	
	double x = pSlide->fMargin + dw/2, y = pSlide->fMargin + myLabels.iLabelSize;
	int q = 0;
	Icon *pIcon;
	GList *ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->pIconBuffer != NULL)
		{
			cairo_save (pCairoContext);
			
			pIcon->fDrawX = x;
			pIcon->fDrawY = y;
			cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, TRUE, pDesklet->iWidth);
			
			cairo_restore (pCairoContext);
			
			x += pSlide->iIconSize + dw;
			q ++;
			if (q == pSlide->iNbColumns)
			{
				q = 0;
				x = pSlide->fMargin + dw/2;
				y += pSlide->iIconSize + myLabels.iLabelSize + dh;
			}
		}
	}
}


void rendering_draw_slide_in_desklet_opengl (CairoDesklet *pDesklet)
{
	_cairo_dock_define_static_vertex_tab (7);
	CDSlideParameters *pSlide = (CDSlideParameters *) pDesklet->pRendererData;
	//g_print ("%s(%x)\n", __func__, pSlide);
	if (pSlide == NULL)
		return ;
	
	double fRadius = pSlide->iRadius;
	double fLineWidth = pSlide->iLineWidth;
	// le cadre.
	if (pSlide->bRoundedRadius)
	{
		// le fond
		cairo_dock_draw_rounded_rectangle_opengl (fRadius,
			0.,
			pDesklet->iWidth - 2 * fRadius,
			pDesklet->iHeight,
			0., 0.,
			pSlide->fBgColor);
		// le cadre
		cairo_dock_draw_rounded_rectangle_opengl (fRadius,
			fLineWidth,
			pDesklet->iWidth - 2 * fRadius,
			pDesklet->iHeight,
			0., 0.,
			pSlide->fLineColor);
	}
	else
	{
		// le fond
		_cairo_dock_set_vertex_xy (0, 0., pDesklet->iHeight);
		_cairo_dock_set_vertex_xy (1, 0., fRadius);
		_cairo_dock_set_vertex_xy (2, fRadius, 0.);
		_cairo_dock_set_vertex_xy (3, pDesklet->iWidth, 0.);
		_cairo_dock_set_path_as_current ();
		cairo_dock_draw_current_path_opengl (fLineWidth, pSlide->fBgColor, 4);
		
		// le cadre

	}
	
	// les icones.
	double w = pDesklet->iWidth - 2 * pSlide->fMargin;
	double h = pDesklet->iHeight - 2 * pSlide->fMargin;
	int dh = myLabels.iLabelSize;  // ecart entre 2 lignes.
	int dw = 2 * dh;  // ecart entre 2 colonnes.
	int di = pSlide->iGapBetweenIcons;
	
	double x = pSlide->fMargin, y = pSlide->fMargin + dh;
	int iNumLine = 0;
	Icon *pIcon;
	GList *ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->pIconBuffer != NULL)
		{
			glPushMatrix ();
			
			pIcon->fDrawX = x;
			pIcon->fDrawY = y;
			cairo_dock_draw_icon_texture (pIcon, CAIRO_CONTAINER (pDesklet));
			//cairo_dock_render_one_icon_opengl (pIcon, pDesklet, 1., TRUE);
			
			glPopMatrix ();
			
			x += pSlide->iIconSize + dw + di;
			if (x + pSlide->iIconSize + dw > w)
			{
				x = pSlide->fMargin;
				y += pSlide->iIconSize + dh + di;
			}
		}
	}
}



void rendering_register_slide_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render = rendering_draw_slide_in_desklet;
	pRenderer->configure = rendering_configure_slide;
	pRenderer->load_data = rendering_load_slide_data;
	pRenderer->free_data = rendering_free_slide_data;
	pRenderer->load_icons = rendering_load_icons_for_slide;
	pRenderer->render_opengl = rendering_draw_slide_in_desklet_opengl;
	
	cairo_dock_register_desklet_renderer (MY_APPLET_SLIDE_DESKLET_RENDERER_NAME, pRenderer);
}
