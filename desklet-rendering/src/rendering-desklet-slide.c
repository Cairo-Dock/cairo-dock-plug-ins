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


static gboolean on_move_inside_desklet (gpointer pUserData, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	if (CAIRO_DOCK_IS_DESKLET (pContainer) && CAIRO_DESKLET (pContainer)->pRenderer->render == rendering_draw_slide_in_desklet)
	{
		CairoDesklet *pDesklet = CAIRO_DESKLET (pContainer);
		Icon *pIcon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
		if (pIcon != NULL)
		{
			if (! pIcon->bPointed)
			{
				Icon *pPointedIcon = cairo_dock_get_pointed_icon (pDesklet->icons);
				if (pPointedIcon != NULL)
					pPointedIcon->bPointed = FALSE;
				pIcon->bPointed = TRUE;
				gtk_widget_queue_draw (pDesklet->pWidget);
			}
		}
		else
		{
			Icon *pPointedIcon = cairo_dock_get_pointed_icon (pDesklet->icons);
			if (pPointedIcon != NULL)
			{
				pPointedIcon->bPointed = FALSE;
				gtk_widget_queue_draw (pDesklet->pWidget);
			}
		}
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

CDSlideParameters *rendering_configure_slide (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig)  // gboolean, int, gdouble[4]
{
	CDSlideParameters *pSlide = g_new0 (CDSlideParameters, 1);
	if (pConfig != NULL)
	{
		pSlide->bRoundedRadius = GPOINTER_TO_INT (pConfig[0]);
		pSlide->iRadius = GPOINTER_TO_INT (pConfig[1]);
		if (pConfig[2] != NULL)
			memcpy (pSlide->fLineColor, pConfig[2], 4 * sizeof (gdouble));
		pSlide->iLineWidth = 2;
		pSlide->iGapBetweenIcons = 10;
	}
	
	cairo_dock_register_notification_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_MOUSE_MOVED, (CairoDockNotificationFunc) on_move_inside_desklet, CAIRO_DOCK_RUN_FIRST, NULL);
	
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
	int dh = myLabels.iLabelSize;  // taille verticale ajoutee a chaque icone.
	int dw = 2 * dh;  // taille horizontale ajoutee a chaque icone.
	int di = pSlide->iGapBetweenIcons;  // ecart entre 2 lignes/colonnes.
	
	int p, q;  // nombre de lignes et colonnes.
	int iSize;
	pSlide->iIconSize = 0, pSlide->iNbLines = 0, pSlide->iNbColumns = 0;
	//g_print ("%d icones sur %dx%d (%d)\n", pSlide->iNbIcons, (int)w, (int)h, myLabels.iLabelSize);
	for (p = 1; p <= pSlide->iNbIcons; p ++)
	{
		q = (int) ceil ((double)pSlide->iNbIcons / p);
		iSize = MIN ((h - (p - 1) * di) / p - dh, (w - (q - 1) * di) / q - dw);
		//g_print ("  %dx%d -> %d\n", p, q, iSize);
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
	cairo_dock_remove_notification_func_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_MOUSE_MOVED, (CairoDockNotificationFunc) on_move_inside_desklet, NULL);
	
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
	}
	else
	{
		cairo_move_to (pCairoContext, 0., 0.);
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
	
	//glDisable (GL_TEXTURE_2D);
	
	double fRadius = pSlide->iRadius;
	double fLineWidth = pSlide->iLineWidth;
	// le cadre.
	if (pSlide->bRoundedRadius)
	{
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
		int i = 0;
		glTranslatef (-pDesklet->iWidth/2, -pDesklet->iHeight/2, 0.);
		_cairo_dock_set_vertex_xy (0, 0., pDesklet->iHeight);
		_cairo_dock_set_vertex_xy (1, 0., fRadius);
		_cairo_dock_set_vertex_xy (2, fRadius, 0.);
		_cairo_dock_set_vertex_xy (3, pDesklet->iWidth, 0.);
		_cairo_dock_set_path_as_current ();
		cairo_dock_draw_current_path_opengl (fLineWidth, pSlide->fLineColor, 4);
	}
	
	// les icones.
	double w = pDesklet->iWidth - 2 * pSlide->fMargin;
	double h = pDesklet->iHeight - 2 * pSlide->fMargin;
	int dh = (h - pSlide->iNbLines * (pSlide->iIconSize + myLabels.iLabelSize)) / (pSlide->iNbLines != 1 ? pSlide->iNbLines - 1 : 1);  // ecart entre 2 lignes.
	int dw = (w - pSlide->iNbColumns * pSlide->iIconSize) / pSlide->iNbColumns;  // ecart entre 2 colonnes.
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (1.);
	
	
	double x = pSlide->fMargin + dw/2, y = pSlide->fMargin + myLabels.iLabelSize;
	int q = 0;
	Icon *pIcon;
	GList *ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		
		pIcon->fDrawX = x;
		pIcon->fDrawY = y;
		
		x += pSlide->iIconSize + dw;
		q ++;
		if (q == pSlide->iNbColumns)
		{
			q = 0;
			x = pSlide->fMargin + dw/2;
			y += pSlide->iIconSize + myLabels.iLabelSize + dh;
		}
	}
	
	
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDesklet->icons);
	if (pFirstDrawnElement == NULL)
		return;
	ic = pFirstDrawnElement;
	do
	{
		pIcon = ic->data;
		
		if (pIcon->iIconTexture != 0)
		{
			glPushMatrix ();
			
			glTranslatef (pIcon->fDrawX + pIcon->fWidth/2,
				pDesklet->iHeight - pIcon->fDrawY - pIcon->fHeight/2,
				0.);
			_cairo_dock_apply_texture_at_size (pIcon->iIconTexture, pIcon->fWidth, pIcon->fHeight);
			
			if (pIcon->iLabelTexture != 0)
			{
				glPushMatrix ();
				
				double u0 = 0., u1 = 1.;
				double fOffsetX = 0.;
				if (pIcon->bPointed)
				{
					_cairo_dock_set_alpha (1.);
					if (pIcon->fDrawX + pIcon->fWidth/2 + pIcon->iTextWidth/2 > pDesklet->iWidth)
						fOffsetX = pDesklet->iWidth - (pIcon->fDrawX + pIcon->fWidth/2 + pIcon->iTextWidth/2);
					if (pIcon->fDrawX + pIcon->fWidth/2 - pIcon->iTextWidth/2 < 0)
						fOffsetX = pIcon->iTextWidth/2 - (pIcon->fDrawX + pIcon->fWidth/2);
				}
				else
				{
					_cairo_dock_set_alpha (.6);
					if (pIcon->iTextWidth > pIcon->fWidth + 2 * myLabels.iLabelSize)
					{
						fOffsetX = 0.;
						u1 = (double) (pIcon->fWidth + 2 * myLabels.iLabelSize) / pIcon->iTextWidth;
					}
				}
				
				glTranslatef (fOffsetX, pIcon->fHeight/2 + pIcon->iTextHeight / 2, 0.);
				
				glBindTexture (GL_TEXTURE_2D, pIcon->iLabelTexture);
				_cairo_dock_apply_current_texture_portion_at_size_with_offset (u0, 0.,
					u1 - u0, 1.,
					pIcon->iTextWidth * (u1 - u0), pIcon->iTextHeight,
					0., 0.);
				_cairo_dock_set_alpha (1.);
				
				glPopMatrix ();
			}
			
			if (pIcon->iQuickInfoTexture != 0)
			{
				glTranslatef (0., (- pIcon->fHeight + pIcon->iQuickInfoHeight)/2, 0.);
				
				_cairo_dock_apply_texture_at_size (pIcon->iQuickInfoTexture,
					pIcon->iQuickInfoWidth,
					pIcon->iQuickInfoHeight);
			}
			
			glPopMatrix ();
		}
		
		ic = cairo_dock_get_next_element (ic, pDesklet->icons);
	} while (ic != pFirstDrawnElement);
	
	_cairo_dock_disable_texture ();
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
