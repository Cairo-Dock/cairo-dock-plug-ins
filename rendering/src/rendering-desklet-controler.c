/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include "math.h"
#include <cairo-dock.h>

#include "rendering-desklet-controler.h"

#define CONTROLER_RATIO_ICON_DESKLET .75


CDControlerParameters *rendering_load_controler_data (CairoDockDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig)
{
	g_print ("%s ()\n", __func__);
	GList *pIconsList = pDesklet->icons;
	
	CDControlerParameters *pControler = g_new0 (CDControlerParameters, 1);
	
	if (pConfig != NULL)
	{
		pControler->b3D = GPOINTER_TO_INT (pConfig[0]);
		pControler->bCircular = GPOINTER_TO_INT (pConfig[1]);
	}
	
	
	int iNbIcons = g_list_length (pIconsList);
	pControler->fGapBetweenIcons = (pDesklet->iWidth - 2*g_iDockRadius) / (iNbIcons + 1);
	
	int iMaxIconWidth = 0, iMaxIconHeight = 0;
	Icon *icon;
	GList* ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		iMaxIconWidth = MAX (iMaxIconWidth, icon->fWidth);
		iMaxIconHeight = MAX (iMaxIconHeight, icon->fHeight);
	}
	
	double fCentralSphereWidth, fCentralSphereHeight;
	if (pControler->b3D)
	{
		fCentralSphereWidth = MAX (1, MIN (pDesklet->iWidth, pDesklet->iHeight) * CONTROLER_RATIO_ICON_DESKLET - g_fReflectSize);
		fCentralSphereHeight = fCentralSphereWidth;
		
		pControler->iEllipseHeight = MIN (fCentralSphereHeight, pDesklet->iHeight - 2 * (g_iLabelSize + g_fReflectSize) - 1);
		pControler->fInclinationOnHorizon = atan2 (pDesklet->iHeight, pDesklet->iWidth/4);
		pControler->iFrameHeight = pControler->iEllipseHeight + 0*2 * g_iFrameMargin + g_fReflectSize;
		pControler->fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (pControler->iFrameHeight, pControler->fInclinationOnHorizon, g_iDockRadius, g_iDockLineWidth);
	}
	else
	{
		fCentralSphereWidth = MAX (1, (pDesklet->iWidth - g_iDockRadius - g_iLabelSize) * CONTROLER_RATIO_ICON_DESKLET);
		fCentralSphereHeight = MAX (1, (pDesklet->iHeight - g_iDockRadius) * CONTROLER_RATIO_ICON_DESKLET);
	}
	
	return pControler;
}


void rendering_free_controler_data (CairoDockDesklet *pDesklet)
{
	g_print ("%s ()\n", __func__);
	
	CDControlerParameters *pControler = (CDControlerParameters *) pDesklet->pRendererData;
	if (pControler == NULL)
		return ;
	
	g_free (pControler);
	pDesklet->pRendererData = NULL;
}


void rendering_load_icons_for_controler (CairoDockDesklet *pDesklet, cairo_t *pSourceContext)
{
	CDControlerParameters *pControler = (CDControlerParameters *) pDesklet->pRendererData;
	if (pControler == NULL)
		return ;
	
	
	double fCentralSphereWidth, fCentralSphereHeight;
	if (pControler->b3D)
	{
		fCentralSphereWidth = MAX (1, (MIN (pDesklet->iWidth, pDesklet->iHeight - g_iLabelSize) - g_iDockRadius) * CONTROLER_RATIO_ICON_DESKLET - g_fReflectSize);
		fCentralSphereHeight = fCentralSphereWidth;
	}
	else
	{
		fCentralSphereWidth = MAX (1, (pDesklet->iWidth - g_iDockRadius) * CONTROLER_RATIO_ICON_DESKLET);
		fCentralSphereHeight = MAX (1, (pDesklet->iHeight - g_iDockRadius - g_iLabelSize) * CONTROLER_RATIO_ICON_DESKLET);
	}
	
	Icon *pIcon = pDesklet->pIcon;
	if (pIcon != NULL)
	{
		pIcon->fWidth = fCentralSphereWidth;
		pIcon->fHeight = fCentralSphereHeight;
		pIcon->fDrawX = (pDesklet->iWidth - pDesklet->pIcon->fWidth) / 2;
		pIcon->fDrawY = g_iLabelSize + g_iDockRadius/2;
		pIcon->fScale = 1.;
		pIcon->fAlpha = 1.;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		cairo_dock_fill_icon_buffers_for_desklet (pDesklet->pIcon, pSourceContext);
	}
	GList* ic;
	
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pControler->b3D)
		{
			pIcon->fWidth = fCentralSphereWidth * (1 - CONTROLER_RATIO_ICON_DESKLET);
			pIcon->fHeight = pIcon->fWidth;
		}
		else
		{
			pIcon->fWidth = MAX (1, (pDesklet->iWidth - g_iDockRadius) * (1 - CONTROLER_RATIO_ICON_DESKLET));
			pIcon->fHeight = MAX (1, (pDesklet->iHeight - g_iDockRadius - g_iLabelSize) *(1 - CONTROLER_RATIO_ICON_DESKLET));
		}
		cairo_dock_fill_icon_buffers_for_desklet (pIcon, pSourceContext);
		g_print (" + %dx%d\n", (int)pIcon->fWidth, (int) pIcon->fHeight);
	}
}

// _________________
//|   .--titre--.   |
//|  /  icone    \  |
//| /___reflet____\ |
//|_|__controles__|_|
//
//

void rendering_draw_controler_in_desklet (cairo_t *pCairoContext, CairoDockDesklet *pDesklet, gboolean bRenderOptimized)
{
	CDControlerParameters *pControler = (CDControlerParameters *) pDesklet->pRendererData;
	if (pControler == NULL)
		return ;
	
	int iEllipseHeight = pControler->iEllipseHeight;
	double fInclinationOnHorizon = pControler->fInclinationOnHorizon;
	
	int iFrameHeight = pControler->iFrameHeight;
	double fExtraWidth = pControler->fExtraWidth;
	
	
	int iNbIcons = 0, iControlPanelHeight = 0;
	double fIconExtent = 0;
	Icon *pIcon;
	GList *ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		fIconExtent += pIcon->fWidth;
		iControlPanelHeight = MAX (iControlPanelHeight, pIcon->fHeight);
		iNbIcons ++;
	}
	
	
	if (pControler->b3D)
	{
		double fX = g_iDockRadius + pControler->fGapBetweenIcons, fY = g_iLabelSize + pDesklet->pIcon->fHeight + g_fReflectSize;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			
			pIcon->fDrawX = fX - pIcon->fWidth / 2;
			pIcon->fDrawY = fY;
			
			fX += pControler->fGapBetweenIcons;
		}
		
		//\____________________ On trace le cadre.
		double fLineWidth = g_iDockLineWidth;
		double fMargin = 0*g_iFrameMargin;
		
		double fDockWidth = pDesklet->iWidth - fExtraWidth;
		int sens=1;
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = fExtraWidth / 2;
		fDockOffsetY = pDesklet->iHeight - iControlPanelHeight - 2 * fLineWidth - iFrameHeight;
		
		cairo_save (pCairoContext);
		cairo_dock_draw_frame (pCairoContext, g_iDockRadius, fLineWidth, fDockWidth, iFrameHeight, fDockOffsetX, fDockOffsetY, sens, fInclinationOnHorizon, pDesklet->bIsHorizontal);
		
		//\____________________ On dessine les decorations dedans.
		cairo_save (pCairoContext);
		double fColor[4];
		int i;
		for (i = 0; i < 4; i ++)
		{
			fColor[i] = (g_fDeskletColorInside[i] * pDesklet->iGradationCount + g_fDeskletColor[i] * (CD_NB_ITER_FOR_GRADUATION - pDesklet->iGradationCount)) / CD_NB_ITER_FOR_GRADUATION;
		}
		cairo_set_source_rgba (pCairoContext, fColor[0], fColor[1], fColor[2], .75);
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
		
		//\____________________ On dessine le cadre.
		if (fLineWidth > 0)
		{
			cairo_set_line_width (pCairoContext, fLineWidth);
			cairo_set_source_rgba (pCairoContext, fColor[0], fColor[1], fColor[2], 1.);
			cairo_stroke (pCairoContext);
		}
		cairo_restore (pCairoContext);
		
		//\____________________ On dessine les icones.
		cairo_save (pCairoContext);
		pDesklet->pIcon->fDrawY = g_iLabelSize;
		cairo_dock_render_one_icon_in_desklet (pDesklet->pIcon, pCairoContext, TRUE, TRUE, pDesklet->iWidth);
		cairo_restore (pCairoContext);
		
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, FALSE, pDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
	}
	else
	{
		cairo_save (pCairoContext);
		cairo_dock_render_one_icon_in_desklet (pDesklet->pIcon, pCairoContext, FALSE, TRUE, pDesklet->iWidth);
		cairo_restore (pCairoContext);
		
		double fX = g_iDockRadius + pControler->fGapBetweenIcons, fY = g_iLabelSize + pDesklet->pIcon->fHeight;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			
			pIcon->fDrawX = fX - pIcon->fWidth / 2;
			pIcon->fDrawY = fY;
			
			fX += pControler->fGapBetweenIcons;
		}
		
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, FALSE, pDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
	}
}


void rendering_register_controler_desklet_renderer (void)
{
	CairoDockDeskletRenderer *pRenderer = g_new0 (CairoDockDeskletRenderer, 1);
	pRenderer->render = rendering_draw_controler_in_desklet;
	pRenderer->load_data = rendering_load_controler_data;
	pRenderer->free_data = rendering_free_controler_data;
	pRenderer->load_icons = rendering_load_icons_for_controler;
	
	cairo_dock_register_desklet_renderer (MY_APPLET_CONTROLER_DESKLET_RENDERER_NAME, pRenderer);
}
