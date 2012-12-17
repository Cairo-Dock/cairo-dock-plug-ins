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

#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "rendering-desklet-simple.h"


static CDSimpleParameters *configure (CairoDesklet *pDesklet, gpointer *pConfig)  // gint x4
{
	CDSimpleParameters *pSimple = g_new0 (CDSimpleParameters, 1);
	if (pConfig != NULL)
	{
		pSimple->iTopMargin = GPOINTER_TO_INT (pConfig[0]);
		pSimple->iLeftMargin = GPOINTER_TO_INT (pConfig[1]);
		pSimple->iBottomMargin = GPOINTER_TO_INT (pConfig[2]);
		pSimple->iRightMargin = GPOINTER_TO_INT (pConfig[3]);
	}
	return pSimple;
}

/* Not used
static void set_icon_size (CairoDesklet *pDesklet, Icon *pIcon)
{
	CDSimpleParameters *pSimple = (CDSimpleParameters *) pDesklet->pRendererData;
	if (pSimple == NULL)
		return ;
	
	if (pIcon == pDesklet->pIcon)
	{
		pIcon->fWidth = MAX (1, pDesklet->container.iWidth - pSimple->iLeftMargin - pSimple->iRightMargin);
		pIcon->fHeight = MAX (1, pDesklet->container.iHeight - pSimple->iTopMargin - pSimple->iBottomMargin);
	}
}
*/

static void calculate_icons (CairoDesklet *pDesklet)
{
	g_return_if_fail (pDesklet != NULL);
	CDSimpleParameters *pSimple = (CDSimpleParameters *) pDesklet->pRendererData;
	if (pSimple == NULL)
		return ;
	
	Icon *pIcon = pDesklet->pIcon;
	g_return_if_fail (pIcon != NULL);
	pIcon->fWidth = MAX (1, pDesklet->container.iWidth - pSimple->iLeftMargin - pSimple->iRightMargin);
	pIcon->fHeight = MAX (1, pDesklet->container.iHeight - pSimple->iTopMargin - pSimple->iBottomMargin);
	cairo_dock_icon_set_allocated_size (pIcon, pIcon->fWidth, pIcon->fHeight);
	pIcon->fDrawX = pSimple->iLeftMargin;
	pIcon->fDrawY = pSimple->iTopMargin;
	pIcon->fWidthFactor = 1.;
	pIcon->fHeightFactor = 1.;
	pIcon->fScale = 1.;
	pIcon->fGlideScale = 1.;
	pIcon->fAlpha = 1.;
}


static void render (cairo_t *pCairoContext, CairoDesklet *pDesklet)
{
	Icon *pIcon = pDesklet->pIcon;
	if (pIcon == NULL)  // peut arriver avant de lier l'icone au desklet.
		return ;
	
	cairo_translate (pCairoContext, pIcon->fDrawX, pIcon->fDrawY);
	
	cairo_dock_apply_image_buffer_surface (&pIcon->image, pCairoContext);
	
	cairo_dock_draw_icon_overlays_cairo (pIcon, pDesklet->container.fRatio, pCairoContext);
}

static void render_opengl (CairoDesklet *pDesklet)
{
	Icon *pIcon = pDesklet->pIcon;
	if (pIcon == NULL)  // peut arriver avant de lier l'icone au desklet.
		return ;
	
	if (pIcon->image.iTexture != 0)
	{
		pIcon->fAlpha = 1.;
		cairo_dock_draw_icon_texture (pIcon, CAIRO_CONTAINER (pDesklet));
	}
	cairo_dock_draw_icon_overlays_opengl (pIcon, pDesklet->container.fRatio);
}

static void free_data (CairoDesklet *pDesklet)
{
	CDSimpleParameters *pSimple = (CDSimpleParameters *) pDesklet->pRendererData;
	if (pSimple == NULL)
		return ;
	
	g_free (pSimple);
	pDesklet->pRendererData = NULL;
}

void rendering_register_simple_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render 			= (CairoDeskletRenderFunc) render;
	pRenderer->configure 		= (CairoDeskletConfigureRendererFunc) configure;
	pRenderer->load_data 		= NULL;
	pRenderer->free_data 		= (CairoDeskletFreeRendererDataFunc) free_data;
	pRenderer->calculate_icons 	= (CairoDeskletCalculateIconsFunc) calculate_icons;
	pRenderer->render_opengl 	= (CairoDeskletGLRenderFunc) render_opengl;
	
	cairo_dock_register_desklet_renderer (MY_APPLET_SIMPLE_DESKLET_RENDERER_NAME, pRenderer);
}
