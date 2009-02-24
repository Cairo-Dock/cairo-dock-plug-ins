/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "rendering-desklet-simple.h"


void rendering_load_icons_for_simple (CairoDesklet *pDesklet, cairo_t *pSourceContext)
{
	g_return_if_fail (pDesklet != NULL && pSourceContext != NULL);
	
	Icon *pIcon = pDesklet->pIcon;
	g_return_if_fail (pIcon != NULL);
	
	pIcon->fWidth = MAX (1, pDesklet->iWidth - 0);
	pIcon->fHeight = MAX (1, pDesklet->iHeight - 0);
	pIcon->fDrawX = .5 * 0;
	pIcon->fDrawY = .5 * 0;
	pIcon->fScale = 1.;
	pIcon->fGlideScale = 1.;
	cd_debug ("%s (%.2fx%.2f)", __func__, pIcon->fWidth, pIcon->fHeight);
	cairo_dock_fill_icon_buffers_for_desklet (pIcon, pSourceContext);
}


void rendering_draw_simple_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized)
{
	Icon *pIcon = pDesklet->pIcon;
	if (pIcon == NULL)  // peut arriver avant de lier l'icone au desklet.
		return ;
	
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
}

void rendering_draw_simple_in_desklet_opengl (CairoDesklet *pDesklet)
{
	Icon *pIcon = pDesklet->pIcon;
	if (pIcon == NULL)  // peut arriver avant de lier l'icone au desklet.
		return ;
	
	if (pIcon->iIconTexture != 0)
	{
		glPushMatrix ();
		cairo_dock_draw_icon_texture (pIcon, CAIRO_CONTAINER (pDesklet));
		glPopMatrix ();
	}
	if (pIcon->iQuickInfoTexture != 0)
	{
		glTranslatef (0.,
			(- pIcon->fHeight + pIcon->iQuickInfoHeight)/2,
			0.);
		cairo_dock_draw_texture (pIcon->iQuickInfoTexture,
			pIcon->iQuickInfoWidth,
			pIcon->iQuickInfoHeight);
	}
}


void rendering_register_simple_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render = rendering_draw_simple_in_desklet;
	pRenderer->configure = NULL;
	pRenderer->load_data = NULL;
	pRenderer->free_data = NULL;
	pRenderer->load_icons = rendering_load_icons_for_simple;
	pRenderer->render_opengl = rendering_draw_simple_in_desklet_opengl;
	
	cairo_dock_register_desklet_renderer (MY_APPLET_SIMPLE_DESKLET_RENDERER_NAME, pRenderer);
}
