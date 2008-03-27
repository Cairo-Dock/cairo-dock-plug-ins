/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include "math.h"
#include <cairo-dock.h>

#include "rendering-desklet-controler.h"

#define CONTROLER_RATIO_ICON_DESKLET .8


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


void rendering_free_controler_parameters (CDControlerParameters *pControler, gboolean bFree)
{
	if (pControler == NULL)
		return ;
	
	if (bFree)
		g_free (pControler);
	else
		memset (pControler, 0, sizeof (CDControlerParameters));
}


void rendering_load_icons_for_controler_desklet (CairoDockDesklet *pDesklet)
{
	CDControlerParameters *pControler = (CDControlerParameters *) pDesklet->pRendererData;
	if (pControler == NULL)
		return ;
	
	Icon *pIcon = pDesklet->pIcon;
	if (pIcon != NULL)
	{
		if (pControler->b3D)
		{
			pIcon->fWidth = MAX (1, MIN (pDesklet->iWidth, pDesklet->iHeight) * CONTROLER_RATIO_ICON_DESKLET - g_fReflectSize);
			pIcon->fHeight = pDesklet->pIcon->fWidth;
		}
		else
		{
			pIcon->fWidth = MAX (1, (pDesklet->iWidth - g_iDockRadius - g_iLabelSize) * CONTROLER_RATIO_ICON_DESKLET);
			pIcon->fHeight = MAX (1, (pDesklet->iHeight - g_iDockRadius) * CONTROLER_RATIO_ICON_DESKLET);
		}
		
		pIcon->fDrawX = (pDesklet->iWidth - pDesklet->pIcon->fWidth) / 2;
		pIcon->fDrawY = (pDesklet->iHeight - pDesklet->pIcon->fHeight) / 2 + (pControler->b3D ? g_iLabelSize : 0);
		pIcon->fScale = 1.;
		pIcon->fAlpha = 1.;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		cairo_dock_load_one_icon_from_scratch (pDesklet->pIcon, CAIRO_DOCK_CONTAINER (pDesklet));
	}
	GList* ic;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_DOCK_CONTAINER (pDesklet));
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pControler->b3D)
		{
			pIcon->fWidth = 0;
			pIcon->fHeight = 0;
		}
		else
		{
			pIcon->fWidth = MAX (1, .2 * pDesklet->iWidth - g_iLabelSize);
			pIcon->fHeight = MAX (1, .2 * pDesklet->iHeight - g_iLabelSize);
		}
		cairo_dock_fill_icon_buffers (pIcon, pCairoContext, 1, CAIRO_DOCK_HORIZONTAL, pControler->b3D);  // en 3D on charge les reflets.
	}
	cairo_destroy (pCairoContext);
}



void rendering_draw_controler_in_desklet (cairo_t *pCairoContext, CairoDockDesklet *pDesklet)
{
	CDControlerParameters *pControler = (CDControlerParameters *) pDesklet->pRendererData;
	if (pControler == NULL)
		return ;
	
	int iEllipseHeight = pControler->iEllipseHeight;
	double fInclinationOnHorizon = pControler->fInclinationOnHorizon;
	
	int iFrameHeight = pControler->iFrameHeight;
	double fExtraWidth = pControler->fExtraWidth;
	if (pDesklet->icons == NULL)
		return ;
	
	Icon *pIcon;
	GList *ic;
	if (pControler->b3D)
	{
		int iNbIcons = 0, iControlPanelHeight = 0;
		double fIconExtent = 0;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			fIconExtent += pIcon->fWidth;
			iControlPanelHeight = MAX (iControlPanelHeight, pIcon->fHeight);
			iNbIcons ++;
		}
		
		double fGapBetweenIcons = (iNbIcons > 1 ? (pDesklet->iWidth - 2*g_iDockRadius - fIconExtent) / (iNbIcons - 1) : 0);
		
		double fX = g_iDockRadius, fY = g_iLabelSize + pDesklet->pIcon->fHeight + g_fReflectSize;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			
			pIcon->fDrawX = fX;
			pIcon->fDrawY = fY;
			
			fX += pIcon->fWidth + fGapBetweenIcons;
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
		
		cairo_save (pCairoContext);
		pDesklet->pIcon->fDrawY = g_iLabelSize;
		cairo_dock_render_one_icon_in_desklet (pDesklet->pIcon, pCairoContext, TRUE, FALSE, pDesklet->iWidth);
		cairo_restore (pCairoContext);
	}
	else
	{
		/*cairo_save (pCairoContext);
		cairo_dock_render_one_icon_in_desklet (pDesklet->pIcon, pCairoContext, FALSE, FALSE, pDesklet->iWidth);
		cairo_restore (pCairoContext);
		
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				pIcon->fDrawX = pDesklet->pIcon->fDrawX + pDesklet->pIcon->fWidth / 2 + (bFlip ? b : a) * cos (fTheta) - pIcon->fWidth/2;
				pIcon->fDrawY = pDesklet->pIcon->fDrawY + pDesklet->pIcon->fHeight / 2 + (bFlip ? a : b) * sin (fTheta) - pIcon->fHeight/2 + g_iLabelSize;
				cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, TRUE, pDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
		}*/
	}
}
