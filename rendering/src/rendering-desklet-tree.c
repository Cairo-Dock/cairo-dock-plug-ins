/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "rendering-desklet-tree.h"

#define TREE_WIDTH 150
#define TREE_HEIGHT 161
static int s_iLeafPosition[2][3*3] = {{-30,40,1 , 60,105,0 , -45,115,1},{-60,65,0 , 55,115,1 , -30,115,0}};


CDTreeParameters *rendering_configure_tree (CairoDockDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig)
{
	g_print ("%s ()\n", __func__);
	GList *pIconsList = pDesklet->icons;
	if (pIconsList == NULL)
		return NULL;
	
	int iNbIcons = 0;
	Icon *pIcon;
	GList *ic;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (! CAIRO_DOCK_IS_SEPARATOR (pIcon))
			iNbIcons ++;
	}
	if (iNbIcons == 0)
		return NULL;
	
	CDTreeParameters *pTree = g_new0 (CDTreeParameters, 1);
	pTree->iNbIconsInTree = iNbIcons;
	pTree->iNbBranches = (int) ceil (1.*iNbIcons/3.);
	
	double h = pDesklet->iHeight, w = pDesklet->iWidth;
	pTree->fTreeWidthFactor = (w > TREE_WIDTH ? 1 : w / TREE_WIDTH);
	pTree->fTreeHeightFactor = h / (pTree->iNbBranches * TREE_HEIGHT);
	
	g_print (" -> %d icones, %d branches, proportions : %.2fx%.2f\n", pTree->iNbIconsInTree, pTree->iNbBranches, pTree->fTreeWidthFactor, pTree->fTreeHeightFactor);
	
	return pTree;
}


void rendering_load_tree_data (CairoDockDesklet *pDesklet, cairo_t *pSourceContext)
{
	CDTreeParameters *pTree = (CDTreeParameters *) pDesklet->pRendererData;
	if (pTree == NULL)
		return ;
	
	double fImageWidth = TREE_WIDTH * pTree->fTreeWidthFactor, fImageHeight = TREE_HEIGHT * pTree->fTreeHeightFactor;
	gchar *cImageFilePath = g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/branche1.svg", NULL);
	pTree->pBrancheSurface[0] = cairo_dock_load_image (pSourceContext,
		cImageFilePath,
		&fImageWidth,
		&fImageHeight,
		0.,
		1.,
		FALSE);
	
	cImageFilePath[strlen(cImageFilePath)-5] = '2';
	pTree->pBrancheSurface[1] = cairo_dock_load_image (pSourceContext,
		cImageFilePath,
		&fImageWidth,
		&fImageHeight,
		0.,
		1.,
		FALSE);
	g_free (cImageFilePath);
}


void rendering_free_tree_data (CairoDockDesklet *pDesklet)
{
	g_print ("%s ()\n", __func__);
	CDTreeParameters *pTree = (CDTreeParameters *) pDesklet->pRendererData;
	if (pTree == NULL)
		return ;
	
	cairo_surface_destroy (pTree->pBrancheSurface[0]);
	cairo_surface_destroy (pTree->pBrancheSurface[1]);
	
	g_free (pTree);
	pDesklet->pRendererData = NULL;
}


void rendering_load_icons_for_tree (CairoDockDesklet *pDesklet, cairo_t *pSourceContext)
{
	g_return_if_fail (pDesklet != NULL && pSourceContext != NULL);
	CDTreeParameters *pTree = (CDTreeParameters *) pDesklet->pRendererData;
	if (pTree == NULL)
		return ;
	
	GList* ic;
	Icon *icon;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->fWidth = 48 * MIN (pTree->fTreeWidthFactor, pTree->fTreeHeightFactor);
		icon->fHeight = 48 * MIN (pTree->fTreeWidthFactor, pTree->fTreeHeightFactor);
		
		cairo_dock_fill_icon_buffers_for_desklet (icon, pSourceContext);
	}
}



void rendering_draw_tree_in_desklet (cairo_t *pCairoContext, CairoDockDesklet *pDesklet, gboolean bRenderOptimized)
{
	CDTreeParameters *pTree = (CDTreeParameters *) pDesklet->pRendererData;
	g_print ("%s (%x)\n", __func__, pTree);
	if (pTree == NULL)
		return ;
	
	double h = pDesklet->iHeight, w = pDesklet->iWidth;
	int i;
	for (i = 0; i < pTree->iNbBranches; i ++)
	{
		cairo_save (pCairoContext);
		//g_print (" branche %d en (%.2f;%.2f)\n", i, (w - pTree->fTreeWidthFactor * TREE_WIDTH) / 2, h - i * TREE_HEIGHT * pTree->fTreeHeightFactor);
		cairo_translate (pCairoContext, (w - pTree->fTreeWidthFactor * TREE_WIDTH) / 2, h - (i + 1) * TREE_HEIGHT * pTree->fTreeHeightFactor);
		cairo_set_source_surface (pCairoContext, pTree->pBrancheSurface[i%2], 0, 0);
		cairo_paint (pCairoContext);
		cairo_restore (pCairoContext);
	}
	
	
	int iBrancheNumber=0, iBrancheType=0, iLeafNumber=0;
	double x, y;
	int sens;
	Icon *pIcon;
	GList *ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (! CAIRO_DOCK_IS_SEPARATOR (pIcon))
		{
			x = s_iLeafPosition[iBrancheType][3*iLeafNumber];
			y = s_iLeafPosition[iBrancheType][3*iLeafNumber+1];
			sens = s_iLeafPosition[iBrancheType][3*iLeafNumber+2];
			
			pIcon->fDrawX = w / 2 + x * pTree->fTreeWidthFactor - pIcon->fWidth / 2;
			pIcon->fDrawY = h - (iBrancheNumber * TREE_HEIGHT + y) * pTree->fTreeHeightFactor - sens * pIcon->fHeight;
			pIcon->fScale = 1;
			pIcon->fAlpha = 1;
			pIcon->fWidthFactor = 1;
			pIcon->fHeightFactor = 1;
			
			cairo_save (pCairoContext);
			cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, TRUE, pDesklet->iWidth);
			cairo_restore (pCairoContext);
			
			iLeafNumber ++;
			if (iLeafNumber == 3)
			{
				iLeafNumber = 0;
				iBrancheNumber ++;
				iBrancheType = iBrancheNumber % 2;
			}
		}
	}
}


void rendering_register_tree_desklet_renderer (void)
{
	CairoDockDeskletRenderer *pRenderer = g_new0 (CairoDockDeskletRenderer, 1);
	pRenderer->render = rendering_draw_tree_in_desklet ;
	pRenderer->configure = rendering_configure_tree;
	pRenderer->load_data = rendering_load_tree_data;
	pRenderer->free_data = rendering_free_tree_data;
	pRenderer->load_icons = rendering_load_icons_for_tree;
	
	cairo_dock_register_desklet_renderer (MY_APPLET_TREE_DESKLET_RENDERER_NAME, pRenderer);
}
