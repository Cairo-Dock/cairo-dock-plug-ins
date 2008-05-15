/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "rendering-desklet-simple.h"


CDSimpleParameters *rendering_configure_simple (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig)
{
	cd_message ("");
	CDSimpleParameters *pSimple = g_new0 (CDSimpleParameters, 1);
	
	if (pConfig != NULL)
	{
		gchar *cImageFilePath = pConfig[0];  // on la charge tout de suite, inutile d'attendre le 'load_data'.
		pSimple->pForeGroundSurface = cairo_dock_create_surface_from_image (cImageFilePath,
			pSourceContext,
			cairo_dock_get_max_scale (pDesklet),
			pDesklet->iWidth, pDesklet->iHeight,
			&pSimple->fImageWidth, &pSimple->fImageHeight,
			GPOINTER_TO_INT (pConfig[1]));
	}
	
	return pSimple;
}

void rendering_free_simple_data (CairoDesklet *pDesklet)
{
	cd_message ("");
	CDSimpleParameters *pSimple = (CDSimpleParameters *) pDesklet->pRendererData;
	if (pSimple == NULL)
		return ;
	
	if (pSimple->pForeGroundSurface != NULL)
		cairo_surface_destroy (pSimple->pForeGroundSurface);
	
	g_free (pSimple);
	pDesklet->pRendererData = NULL;
}

void rendering_load_icons_for_simple (CairoDesklet *pDesklet, cairo_t *pSourceContext)
{
	g_return_if_fail (pDesklet != NULL && pSourceContext != NULL);
	
	Icon *pIcon = pDesklet->pIcon;
	g_return_if_fail (pIcon != NULL);
	pIcon->fWidth = MAX (1, pDesklet->iWidth - g_iDockRadius);
	pIcon->fHeight = MAX (1, pDesklet->iHeight - g_iDockRadius);
	pIcon->fDrawX = g_iDockRadius/2;
	pIcon->fDrawY = g_iDockRadius/2;
	pIcon->fScale = 1;
	cairo_dock_fill_icon_buffers_for_desklet (pIcon, pSourceContext);
}


void rendering_draw_simple_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized)
{
	CDSimpleParameters *pSimple = (CDSimpleParameters *) pDesklet->pRendererData;
	Icon *pIcon = pDesklet->pIcon;
	
	if (pSimple != NULL)
		cairo_save (pCairoContext);
	cairo_translate (pCairoContext, pIcon->fDrawX, pIcon->fDrawY);
	
	if (pIcon->pIconBuffer != NULL)
	{
		cairo_set_source_surface (pCairoContext,
			pIcon->pIconBuffer,
			0.,
			0.);
		cairo_paint (pCairoContext);
	}
	if (pIcon->pQuickInfoBuffer != NULL)
	{
		cairo_translate (pCairoContext,
			(- pIcon->iQuickInfoWidth + pIcon->fWidth) / 2 * pIcon->fScale,
			(pIcon->fHeight - pIcon->iQuickInfoHeight) * pIcon->fScale);
		
		cairo_set_source_surface (pCairoContext,
			pIcon->pQuickInfoBuffer,
			0.,
			0.);
		cairo_paint (pCairoContext);
	}
	if (pSimple != NULL)
	{
		cairo_restore (pCairoContext);
		cairo_translate (pCairoContext, - pIcon->fDrawX, - pIcon->fDrawY);
		if (pSimple->pForeGroundSurface != NULL)
		{
			cairo_set_source_surface (pCairoContext,
				pSimple->pForeGroundSurface,
				0.,
				0.);
			cairo_paint (pCairoContext);
		}
	}
}


void rendering_register_simple_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render = rendering_draw_simple_in_desklet ;
	pRenderer->configure = rendering_configure_simple;
	pRenderer->load_data = NULL;
	pRenderer->free_data = rendering_free_simple_data;
	pRenderer->load_icons = rendering_load_icons_for_simple;
	
	cairo_dock_register_desklet_renderer (MY_APPLET_SIMPLE_DESKLET_RENDERER_NAME, pRenderer);
}
