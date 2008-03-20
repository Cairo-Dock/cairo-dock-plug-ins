/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "rendering-desklet-simple.h"


void rendering_draw_icon_in_desklet (cairo_t *pCairoContext, CairoDockDesklet *pDesklet)
{
	Icon *pIcon = pDesklet->pIcon;
	cairo_translate (pCairoContext, pIcon->fDrawX, pIcon->fDrawY);

	if (pIcon->pIconBuffer != NULL)
	{
		cairo_set_source_surface (pCairoContext, pIcon->pIconBuffer, 0.0, 0.0);
		cairo_paint (pCairoContext);
	}
	if (pIcon->pQuickInfoBuffer != NULL)
	{
		cairo_translate (pCairoContext,
			(- pIcon->iQuickInfoWidth + pIcon->fWidth) / 2 * pIcon->fScale,
			(pIcon->fHeight - pIcon->iQuickInfoHeight) * pIcon->fScale);

		cairo_set_source_surface (pCairoContext,
			pIcon->pQuickInfoBuffer,
			0,
			0);
		cairo_paint (pCairoContext);
	}
}


void rendering_register_simple_desklet_renderer (void)
{
	CairoDockDeskletRenderer *pRenderer = g_new0 (CairoDockDeskletRenderer, 1);
	pRenderer->render = rendering_draw_icon_in_desklet;
	
	cairo_dock_register_renderer (MY_APPLET_SIMPLE_DESKLET_RENDERER_NAME, pRenderer);
}
