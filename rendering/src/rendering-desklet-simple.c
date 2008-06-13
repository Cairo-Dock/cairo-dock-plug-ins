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
	cd_debug ("");
	CDSimpleParameters *pSimple = g_new0 (CDSimpleParameters, 1);
	
	if (pConfig != NULL)  // dessin d'un cadre et d'un reflet propose par ChanGFu.
	{
		//\________________on charge les surfaces tout de suite, inutile d'attendre le 'load_data'.
		gchar *cBackGroundPath = pConfig[0];
		gchar *cForeGroundPath = pConfig[1];
		CairoDockLoadImageModifier iLoadingModifier = GPOINTER_TO_INT (pConfig[2]);
		pSimple->fBackGroundAlpha = * ((double *) pConfig[3]);
		pSimple->fForeGroundAlpha = * ((double *) pConfig[4]);
		pSimple->iLeftSurfaceOffset = GPOINTER_TO_INT (pConfig[5]);
		pSimple->iTopSurfaceOffset = GPOINTER_TO_INT (pConfig[6]);
		pSimple->iRightSurfaceOffset = GPOINTER_TO_INT (pConfig[7]);
		pSimple->iBottomSurfaceOffset = GPOINTER_TO_INT (pConfig[8]);
		
		double fZoomX, fZoomY;
		if (cBackGroundPath != NULL && pSimple->fBackGroundAlpha > 0)
			pSimple->pBackGroundSurface = cairo_dock_create_surface_from_image (cBackGroundPath,
				pSourceContext,
				cairo_dock_get_max_scale (pDesklet),
				pDesklet->iWidth, pDesklet->iHeight,
				iLoadingModifier,
				&pSimple->fImageWidth, &pSimple->fImageHeight,
				&fZoomX, &fZoomY);
		if (cForeGroundPath != NULL && pSimple->fForeGroundAlpha > 0)
			pSimple->pForeGroundSurface = cairo_dock_create_surface_from_image (cForeGroundPath,
				pSourceContext,
				cairo_dock_get_max_scale (pDesklet),
				pDesklet->iWidth, pDesklet->iHeight,
				iLoadingModifier,
				&pSimple->fImageWidth, &pSimple->fImageHeight,
				&fZoomX, &fZoomY);
		pSimple->iLeftSurfaceOffset = GPOINTER_TO_INT (pConfig[5]) * fZoomX;
		pSimple->iTopSurfaceOffset = GPOINTER_TO_INT (pConfig[6]) * fZoomY;
		pSimple->iRightSurfaceOffset = GPOINTER_TO_INT (pConfig[7]) * fZoomX;
		pSimple->iBottomSurfaceOffset = GPOINTER_TO_INT (pConfig[8]) * fZoomY;
	}
	
	return pSimple;
}

void rendering_free_simple_data (CairoDesklet *pDesklet)
{
	cd_debug ("");
	CDSimpleParameters *pSimple = (CDSimpleParameters *) pDesklet->pRendererData;
	if (pSimple == NULL)
		return ;
	
	if (pSimple->pForeGroundSurface != NULL)
		cairo_surface_destroy (pSimple->pForeGroundSurface);
	if (pSimple->pBackGroundSurface != NULL)
		cairo_surface_destroy (pSimple->pBackGroundSurface);
	
	g_free (pSimple);
	pDesklet->pRendererData = NULL;
}

void rendering_load_icons_for_simple (CairoDesklet *pDesklet, cairo_t *pSourceContext)
{
	g_return_if_fail (pDesklet != NULL && pSourceContext != NULL);
	CDSimpleParameters *pSimple = (CDSimpleParameters *) pDesklet->pRendererData;
	
	Icon *pIcon = pDesklet->pIcon;
	g_return_if_fail (pIcon != NULL);
	if (pSimple != NULL)
	{
		pIcon->fWidth = pDesklet->iWidth - pSimple->iLeftSurfaceOffset - pSimple->iRightSurfaceOffset;
		pIcon->fHeight = pDesklet->iHeight - pSimple->iTopSurfaceOffset - pSimple->iBottomSurfaceOffset;
		pIcon->fDrawX = pSimple->iLeftSurfaceOffset;
		pIcon->fDrawY = pSimple->iTopSurfaceOffset;
	}
	else
	{
		pIcon->fWidth = MAX (1, pDesklet->iWidth - g_iDockRadius);  // 2 * g_iDockRadius/2
		pIcon->fHeight = MAX (1, pDesklet->iHeight - g_iDockRadius);
		pIcon->fDrawX = .5 * g_iDockRadius;
		pIcon->fDrawY = .5 * g_iDockRadius;
	}
	pIcon->fScale = 1;
	g_print ("%s (%.2fx%.2f)\n", __func__, pIcon->fWidth, pIcon->fHeight);
	cairo_dock_fill_icon_buffers_for_desklet (pIcon, pSourceContext);
}


void rendering_draw_simple_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized)
{
	CDSimpleParameters *pSimple = (CDSimpleParameters *) pDesklet->pRendererData;
	Icon *pIcon = pDesklet->pIcon;
	
	if (pSimple != NULL)
	{
		if (pSimple->pBackGroundSurface != NULL)
		{
			cairo_set_source_surface (pCairoContext,
				pSimple->pBackGroundSurface,
				0.,
				0.);
			if (pSimple->fBackGroundAlpha == 1)
				cairo_paint (pCairoContext);
			else
				cairo_paint_with_alpha (pCairoContext, pSimple->fBackGroundAlpha);
		}
		cairo_save (pCairoContext);
	}
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
		if (pSimple->pForeGroundSurface != NULL)
		{
			cairo_set_source_surface (pCairoContext,
				pSimple->pForeGroundSurface,
				0.,
				0.);
			if (pSimple->fForeGroundAlpha == 1)
				cairo_paint (pCairoContext);
			else
				cairo_paint_with_alpha (pCairoContext, pSimple->fForeGroundAlpha);
		}
	}
}


static void _predefine_simple_config (CairoDeskletRenderer *pRenderer, const gchar *cConfigName, const gchar *cBackgroundImage, const gchar *cForegroundImage, int iLeftOffset, int iTopOffset, int iRightOffset, int iBottomOffset)
{
	gpointer *pConfig = g_new0 (gpointer, 9);
	if (cBackgroundImage != NULL)
		pConfig[0] = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, cBackgroundImage);
	if (cForegroundImage != NULL)
		pConfig[1] = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, cForegroundImage);
	pConfig[2] = CAIRO_DOCK_FILL_SPACE;
	double *fBackGroundAlpha = g_new (double, 1);
	*fBackGroundAlpha = 1.;
	pConfig[3] = fBackGroundAlpha;
	double *fForeGroundAlpha = g_new (double, 1);
	*fForeGroundAlpha = 1.;
	pConfig[4] =  fForeGroundAlpha;
	pConfig[5] =  GINT_TO_POINTER (iLeftOffset);  // /200
	pConfig[6] =  GINT_TO_POINTER (iTopOffset);  // /200
	pConfig[7] =  GINT_TO_POINTER (iRightOffset);
	pConfig[8] =  GINT_TO_POINTER (iBottomOffset);
	cairo_dock_predefine_desklet_renderer_config (pRenderer, cConfigName, pConfig);
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
	
	_predefine_simple_config (pRenderer, "frame&reflects",
		"frame.svg",
		"reflect.svg",
		5,  // /200
		5,  // /200
		5,
		5);
	
	_predefine_simple_config (pRenderer, "scotch",
		NULL,
		"scotch.svg",
		40,  // /550
		60,  // /500
		40,
		0);
	
	_predefine_simple_config (pRenderer, "frame with scotch",
		NULL,
		"scotch+frame.svg",
		87,  // /550
		76,  // /500
		87,
		50);
	
	_predefine_simple_config (pRenderer, "CD box",
		"cd_box.svg",
		"cd_box_cover.svg",
		93,  // /750
		86,  // /700
		72,
		79);
	
	/*gpointer *pConfig = g_new0 (gpointer, 9);
	pConfig[0] = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, "frame.svg");
	pConfig[1] = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, "reflect.svg");
	pConfig[2] = CAIRO_DOCK_FILL_SPACE;
	double *fBackGroundAlpha = g_new (double, 1);
	*fBackGroundAlpha = 1.;
	pConfig[3] = fBackGroundAlpha;
	double *fForeGroundAlpha = g_new (double, 1);
	*fForeGroundAlpha = 1.;
	pConfig[4] =  fForeGroundAlpha;
	pConfig[5] =  GINT_TO_POINTER (5);  // /200
	pConfig[6] =  GINT_TO_POINTER (5);  // /200
	pConfig[7] =  GINT_TO_POINTER (5);
	pConfig[8] =  GINT_TO_POINTER (5);
	cairo_dock_predefine_desklet_renderer_config (pRenderer, "frame&reflects", pConfig);
	
	pConfig = g_new0 (gpointer, 9);
	pConfig[0] = NULL;
	pConfig[1] = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, "scotch.svg");
	pConfig[2] = CAIRO_DOCK_FILL_SPACE;
	fBackGroundAlpha = g_new (double, 1);
	*fBackGroundAlpha = 0.;
	pConfig[3] = fBackGroundAlpha;
	fForeGroundAlpha = g_new (double, 1);
	*fForeGroundAlpha = 1.;
	pConfig[4] =  fForeGroundAlpha;
	pConfig[5] =  GINT_TO_POINTER (40);  // /550
	pConfig[6] =  GINT_TO_POINTER (60);  // /500
	pConfig[7] =  GINT_TO_POINTER (40);
	pConfig[8] =  GINT_TO_POINTER (0);
	cairo_dock_predefine_desklet_renderer_config (pRenderer, "scotch", pConfig);
	
	pConfig = g_new0 (gpointer, 9);
	pConfig[0] = NULL;
	pConfig[1] = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, "scotch+frame.svg");
	pConfig[2] = CAIRO_DOCK_FILL_SPACE;
	fBackGroundAlpha = g_new (double, 1);
	*fBackGroundAlpha = 0.;
	pConfig[3] = fBackGroundAlpha;
	fForeGroundAlpha = g_new (double, 1);
	*fForeGroundAlpha = 1.;
	pConfig[4] =  fForeGroundAlpha;
	pConfig[5] =  GINT_TO_POINTER (87);  // /550
	pConfig[6] =  GINT_TO_POINTER (76);  // /500
	pConfig[7] =  GINT_TO_POINTER (87);
	pConfig[8] =  GINT_TO_POINTER (50);
	cairo_dock_predefine_desklet_renderer_config (pRenderer, "frame with scotch", pConfig);
	
	pConfig = g_new0 (gpointer, 9);
	pConfig[0] = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, "cd_box.svg");
	pConfig[1] = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, "cd_box_cover.svg");
	pConfig[2] = CAIRO_DOCK_FILL_SPACE;
	fBackGroundAlpha = g_new (double, 1);
	*fBackGroundAlpha = 1.;
	pConfig[3] = fBackGroundAlpha;
	fForeGroundAlpha = g_new (double, 1);
	*fForeGroundAlpha = 1.;
	pConfig[4] =  fForeGroundAlpha;
	pConfig[5] =  GINT_TO_POINTER (93);  // /750
	pConfig[6] =  GINT_TO_POINTER (86);  // /750
	pConfig[7] =  GINT_TO_POINTER (72);
	pConfig[8] =  GINT_TO_POINTER (79);
	cairo_dock_predefine_desklet_renderer_config (pRenderer, "CD box", pConfig);*/
}
