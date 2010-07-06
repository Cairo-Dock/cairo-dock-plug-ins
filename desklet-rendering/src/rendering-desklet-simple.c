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


static void set_icon_size (CairoDesklet *pDesklet, Icon *pIcon)
{
	if (pIcon == pDesklet->pIcon)
	{
		pIcon->fWidth = MAX (1, pDesklet->container.iWidth);
		pIcon->fHeight = MAX (1, pDesklet->container.iHeight);
	}
}

static void calculate_icons (CairoDesklet *pDesklet)
{
	g_return_if_fail (pDesklet != NULL);
	
	Icon *pIcon = pDesklet->pIcon;
	g_return_if_fail (pIcon != NULL);
	
	pIcon->fWidth = MAX (1, pDesklet->container.iWidth);
	pIcon->fHeight = MAX (1, pDesklet->container.iHeight);
	//pIcon->iImageWidth = pIcon->fWidth;
	//pIcon->iImageHeight = pIcon->fHeight;
	pIcon->fWidthFactor = 1.;
	pIcon->fHeightFactor = 1.;
	pIcon->fScale = 1.;
	pIcon->fGlideScale = 1.;
	pIcon->fAlpha = 1.;
	pIcon->fDrawX = 0.;
	pIcon->fDrawY = 0.;
}


static void render (cairo_t *pCairoContext, CairoDesklet *pDesklet)
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

static void render_opengl (CairoDesklet *pDesklet)
{
	Icon *pIcon = pDesklet->pIcon;
	if (pIcon == NULL)  // peut arriver avant de lier l'icone au desklet.
		return ;
	
	if (pIcon->iIconTexture != 0)
	{
		pIcon->fAlpha = 1.;
		cairo_dock_draw_icon_texture (pIcon, CAIRO_CONTAINER (pDesklet));
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
	pRenderer->render = render;
	pRenderer->configure = NULL;
	pRenderer->load_data = NULL;
	pRenderer->free_data = NULL;
	pRenderer->calculate_icons = calculate_icons;
	pRenderer->render_opengl = render_opengl;
	
	cairo_dock_register_desklet_renderer (MY_APPLET_SIMPLE_DESKLET_RENDERER_NAME, pRenderer);
}
