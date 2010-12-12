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

#include "rendering-desklet-controler.h"

#define CONTROLER_RATIO_ICON_DESKLET .75


static gboolean on_button_press_controler (GtkWidget *widget,
	GdkEventButton *pButton,
	CairoDesklet *pDesklet)
{
	if (pButton->button == 1)  // clic gauche.
	{
		CDControlerParameters *pControler = (CDControlerParameters *) pDesklet->pRendererData;
		if (pControler == NULL)
			return FALSE;
		
		if (pButton->type == GDK_BUTTON_PRESS)
		{
			pControler->pClickedIcon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
			if (pControler->pClickedIcon != NULL)
			{
				gtk_widget_queue_draw (pDesklet->container.pWidget);
			}
		}
		else if (pButton->type == GDK_BUTTON_RELEASE)
		{
			if (pControler->pClickedIcon != NULL)
			{
				pControler->pClickedIcon = NULL;
				gtk_widget_queue_draw (pDesklet->container.pWidget);
			}
		}
	}
	return FALSE;
}

CDControlerParameters *rendering_configure_controler (CairoDesklet *pDesklet, CairoDeskletRendererConfigPtr pConfig)
{
	cd_debug ("%s ()\n", __func__);
	CDControlerParameters *pControler = g_new0 (CDControlerParameters, 1);
	
	if (pConfig != NULL)
	{
		pControler->b3D = GPOINTER_TO_INT (pConfig[0]);
		pControler->bCircular = GPOINTER_TO_INT (pConfig[1]);
	}
	
	int iNbIcons = g_list_length (pDesklet->icons);
	pControler->fGapBetweenIcons = (pDesklet->container.iWidth - 2*myDocksParam.iDockRadius) / (iNbIcons + 1);
	
	return pControler;
}

void rendering_load_controler_data (CairoDesklet *pDesklet)
{
	cd_debug ("%s ()\n", __func__);
	CDControlerParameters *pControler = (CDControlerParameters *) pDesklet->pRendererData;
	if (pControler == NULL)
		return ;
	
	int iMaxIconHeight = 0;
	Icon *icon;
	GList* ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		iMaxIconHeight = MAX (iMaxIconHeight, icon->fHeight);
	}
	pControler->iControlPanelHeight = iMaxIconHeight;
	
	if (pControler->b3D)
	{
		pControler->iEllipseHeight = MIN (pDesklet->pIcon->fHeight, pDesklet->container.iHeight - 2 * (myIconsParam.iconTextDescription.iSize + myIconsParam.fReflectSize) - 1);
		pControler->fInclinationOnHorizon = atan2 (pDesklet->container.iHeight, pDesklet->container.iWidth/4);
		pControler->iFrameHeight = pControler->iEllipseHeight + myIconsParam.fReflectSize;
		pControler->fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (pControler->iFrameHeight, pControler->fInclinationOnHorizon, myDocksParam.iDockRadius, myDocksParam.iDockLineWidth);
	}
	else
	{
		
	}
	
	g_signal_connect (G_OBJECT (pDesklet->container.pWidget),
		"button-press-event",
		G_CALLBACK (on_button_press_controler),
		pDesklet);
	g_signal_connect (G_OBJECT (pDesklet->container.pWidget),
		"button-release-event",
		G_CALLBACK (on_button_press_controler),
		pDesklet);
}


void rendering_free_controler_data (CairoDesklet *pDesklet)
{
	cd_debug ("%s ()\n", __func__);
	CDControlerParameters *pControler = (CDControlerParameters *) pDesklet->pRendererData;
	if (pControler == NULL)
		return ;
	
	pControler->pClickedIcon = NULL;
	g_free (pControler);
	pDesklet->pRendererData = NULL;
}


void rendering_load_icons_for_controler (CairoDesklet *pDesklet)
{
	CDControlerParameters *pControler = (CDControlerParameters *) pDesklet->pRendererData;
	if (pControler == NULL)
		return ;
	
	//\_________________ On determine la taille de l'icone centrale.
	double fCentralSphereWidth, fCentralSphereHeight;
	if (pControler->b3D)
	{
		fCentralSphereWidth = MAX (1, (MIN (pDesklet->container.iWidth, pDesklet->container.iHeight - myIconsParam.iconTextDescription.iSize) - myDocksParam.iDockRadius) * CONTROLER_RATIO_ICON_DESKLET - myIconsParam.fReflectSize);
		fCentralSphereHeight = fCentralSphereWidth;
	}
	else
	{
		fCentralSphereWidth = MAX (1, (pDesklet->container.iWidth - myDocksParam.iDockRadius) * CONTROLER_RATIO_ICON_DESKLET);
		fCentralSphereHeight = MAX (1, (pDesklet->container.iHeight - myDocksParam.iDockRadius - myIconsParam.iconTextDescription.iSize) * CONTROLER_RATIO_ICON_DESKLET);
	}
	
	//\_________________ On charge l'icone centrale.
	Icon *pIcon = pDesklet->pIcon;
	if (pIcon != NULL)
	{
		pIcon->fWidth = fCentralSphereWidth;
		pIcon->fHeight = fCentralSphereHeight;
		pIcon->iImageWidth = pIcon->fWidth;
		pIcon->iImageHeight = pIcon->fHeight;
		pIcon->fDrawX = (pDesklet->container.iWidth - pDesklet->pIcon->fWidth) / 2;
		pIcon->fDrawY = myIconsParam.iconTextDescription.iSize + myDocksParam.iDockRadius/2;
		pIcon->fScale = 1.;
		pIcon->fAlpha = 1.;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		cairo_dock_load_icon_buffers (pIcon, CAIRO_CONTAINER (pDesklet));
	}
	
	//\_________________ On charge les boutons.
	double fX = myDocksParam.iDockRadius + pControler->fGapBetweenIcons, fY = myIconsParam.iconTextDescription.iSize + pDesklet->pIcon->fHeight + myIconsParam.fReflectSize;
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
			pIcon->fWidth = MAX (1, (pDesklet->container.iWidth - myDocksParam.iDockRadius) * (1 - CONTROLER_RATIO_ICON_DESKLET));
			pIcon->fHeight = MAX (1, (pDesklet->container.iHeight - myDocksParam.iDockRadius - myIconsParam.iconTextDescription.iSize) *(1 - CONTROLER_RATIO_ICON_DESKLET));
		}
		pIcon->iImageWidth = pIcon->fWidth;
		pIcon->iImageHeight = pIcon->fHeight;
		
		cairo_dock_load_icon_buffers (pIcon, CAIRO_CONTAINER (pDesklet));
		
		pIcon->fDrawX = fX - pIcon->fWidth / 2;
		pIcon->fDrawY = fY;
		fX += pControler->fGapBetweenIcons;
		
		pIcon->fScale = 1.;
		pIcon->fAlpha = 1.;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		cd_debug (" + %dx%d\n", (int)pIcon->fWidth, (int) pIcon->fHeight);
	}
}

// _________________
//|   .--titre--.   |
//|  /  icone    \  |
//| /___reflet____\ |
//|_|__controles__|_|
//
//

void rendering_draw_controler_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet)
{
	CDControlerParameters *pControler = (CDControlerParameters *) pDesklet->pRendererData;
	if (pControler == NULL)
		return ;
	
	int iEllipseHeight = pControler->iEllipseHeight;
	double fInclinationOnHorizon = pControler->fInclinationOnHorizon;
	
	int iFrameHeight = pControler->iFrameHeight;
	double fExtraWidth = pControler->fExtraWidth;
	
	int iControlPanelHeight = pControler->iControlPanelHeight;
	Icon *pIcon;
	GList *ic;
	
	if (pControler->b3D)
	{
		double fX = myDocksParam.iDockRadius + pControler->fGapBetweenIcons, fY = myIconsParam.iconTextDescription.iSize + pDesklet->pIcon->fHeight + myIconsParam.fReflectSize;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			
			pIcon->fDrawX = fX - pIcon->fWidth / 2;
			pIcon->fDrawY = fY;
			
			fX += pControler->fGapBetweenIcons;
		}
		if (pControler->pClickedIcon != NULL)
		{
			pControler->pClickedIcon->fDrawX += 3;
			pControler->pClickedIcon->fDrawY += 3;
		}
		
		//\____________________ On trace le cadre.
		double fLineWidth = myDocksParam.iDockLineWidth;
		double fMargin = 0;
		
		double fDockWidth = pDesklet->container.iWidth - fExtraWidth;
		int sens=1;
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = fExtraWidth / 2;
		fDockOffsetY = pDesklet->container.iHeight - iControlPanelHeight - 2 * fLineWidth - iFrameHeight;
		
		cairo_save (pCairoContext);
		cairo_dock_draw_frame (pCairoContext, myDocksParam.iDockRadius, fLineWidth, fDockWidth, iFrameHeight, fDockOffsetX, fDockOffsetY, sens, fInclinationOnHorizon, pDesklet->container.bIsHorizontal, TRUE);
		
		//\____________________ On dessine les decorations dedans.
		cairo_set_source_rgba (pCairoContext, .8, .8, .8, .75);
		cairo_fill_preserve (pCairoContext);
		
		//\____________________ On dessine le cadre.
		if (fLineWidth > 0)
		{
			cairo_set_line_width (pCairoContext, fLineWidth);
			cairo_set_source_rgba (pCairoContext, .9, .9, .9, 1.);
			cairo_stroke (pCairoContext);
		}
		cairo_restore (pCairoContext);
		
		//\____________________ On dessine les icones.
		cairo_save (pCairoContext);
		pDesklet->pIcon->fDrawY = myIconsParam.iconTextDescription.iSize;
		cairo_dock_render_one_icon_in_desklet (pDesklet->pIcon, pCairoContext, TRUE, TRUE, pDesklet->container.iWidth);
		cairo_restore (pCairoContext);
		
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, FALSE, pDesklet->container.iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
	}
	else
	{
		cairo_save (pCairoContext);
		cairo_dock_render_one_icon_in_desklet (pDesklet->pIcon, pCairoContext, FALSE, TRUE, pDesklet->container.iWidth);
		cairo_restore (pCairoContext);
		
		double fX = myDocksParam.iDockRadius + pControler->fGapBetweenIcons, fY = myIconsParam.iconTextDescription.iSize + pDesklet->pIcon->fHeight;
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
				
				cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, FALSE, pDesklet->container.iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
	}
}


void rendering_register_controler_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render = rendering_draw_controler_in_desklet;
	pRenderer->configure = (CairoDeskletConfigureRendererFunc)rendering_configure_controler;
	pRenderer->load_data = rendering_load_controler_data;
	pRenderer->free_data = rendering_free_controler_data;
	pRenderer->load_icons = rendering_load_icons_for_controler;
	
	cairo_dock_register_desklet_renderer (MY_APPLET_CONTROLER_DESKLET_RENDERER_NAME, pRenderer);
}
