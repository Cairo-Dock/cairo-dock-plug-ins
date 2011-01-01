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

#include "rendering-desklet-mediaplayer.h"

static gboolean on_button_press_mediaplayer (GtkWidget *widget,
	GdkEventButton *pButton,
	CairoDesklet *pDesklet)
{
	if (pButton->button == 1)  // clic gauche.
	{
		CDMediaplayerParameters *pMediaplayer = (CDMediaplayerParameters *) pDesklet->pRendererData;
		if (pMediaplayer == NULL)
			return FALSE;
		
		if (pButton->type == GDK_BUTTON_PRESS)
		{
			pMediaplayer->pClickedIcon = cairo_dock_find_clicked_icon_in_desklet (pDesklet);
			if (pMediaplayer->pClickedIcon != NULL)
			{
				gtk_widget_queue_draw (pDesklet->container.pWidget);
			}
		}
		else if (pButton->type == GDK_BUTTON_RELEASE)
		{
			if (pMediaplayer->pClickedIcon != NULL)
			{
				pMediaplayer->pClickedIcon = NULL;
				gtk_widget_queue_draw (pDesklet->container.pWidget);
			}
		}
	}
	return FALSE;
}

CDMediaplayerParameters *rendering_configure_mediaplayer (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig)
{
	cd_debug ("");
	CDMediaplayerParameters *pMediaplayer = g_new0 (CDMediaplayerParameters, 1);
	if (pConfig != NULL)  // dessin de l'artiste et du titre sur le coté du desklet.
	{
		pMediaplayer->cArtist = pConfig[0];
		pMediaplayer->cTitle = pConfig[1];
		if (pMediaplayer->cArtist != NULL)
			pMediaplayer->pArtistSurface = cairo_dock_create_surface_from_text_full (pMediaplayer->cArtist,
			&myIconsParam.iconTextDescription,
			cairo_dock_get_max_scale (pDesklet),
			pDesklet->container.iWidth,
			&pMediaplayer->fArtistWidth, &pMediaplayer->fArtistHeight, &pMediaplayer->fArtistXOffset, &pMediaplayer->fArtistYOffset);
		if (pMediaplayer->cTitle != NULL)
			pMediaplayer->pTitleSurface = cairo_dock_create_surface_from_text_full (pMediaplayer->cTitle,
			&myIconsParam.iconTextDescription,
			cairo_dock_get_max_scale (pDesklet),
			pDesklet->container.iWidth,
			&pMediaplayer->fTitleWidth, &pMediaplayer->fTitleHeight, &pMediaplayer->fTitleXOffset, &pMediaplayer->fTitleYOffset);
		
		pMediaplayer->bControlButton = GPOINTER_TO_INT (pConfig[2]);
	}
	return pMediaplayer;
}

void rendering_load_mediaplayer_data (CairoDesklet *pDesklet, cairo_t *pSourceContext)
{
	CDMediaplayerParameters *pMediaplayer = (CDMediaplayerParameters *) pDesklet->pRendererData;
	if (pMediaplayer == NULL)
		return ;
	
	//On initialise la bande des boutons de controle
	pMediaplayer->iNbIcons = g_list_length (pDesklet->icons);
	pMediaplayer->iIconsLimit = pMediaplayer->iNbIcons / 2;
	pMediaplayer->fBandWidth = (pDesklet->container.iHeight - myDocksParam.iDockRadius) / 4;
	pMediaplayer->fIconBandOffset = pMediaplayer->fBandWidth / pMediaplayer->iNbIcons;
	
	//On force la détection du clique sur les icônes
	g_signal_connect (G_OBJECT (pDesklet->container.pWidget),
		"button-press-event",
		G_CALLBACK (on_button_press_mediaplayer),
		pDesklet);
	g_signal_connect (G_OBJECT (pDesklet->container.pWidget),
		"button-release-event",
		G_CALLBACK (on_button_press_mediaplayer),
		pDesklet);
}


void rendering_free_mediaplayer_data (CairoDesklet *pDesklet)
{
	cd_debug ("");
	CDMediaplayerParameters *pMediaplayer = (CDMediaplayerParameters *) pDesklet->pRendererData;
	if (pMediaplayer == NULL)
		return;
	
	//On ne free pa cArtist et cTitle, ces vars appartiennent à l'applet.
	
	if (pMediaplayer->pArtistSurface != NULL)
	{
		cairo_surface_destroy (pMediaplayer->pArtistSurface);
		pMediaplayer->pArtistSurface = NULL;
	}
	if (pMediaplayer->pTitleSurface != NULL)
	{
		cairo_surface_destroy (pMediaplayer->pTitleSurface);
		pMediaplayer->pTitleSurface = NULL;
	}
		
	g_free (pMediaplayer);
	pDesklet->pRendererData = NULL;
}

void rendering_load_icons_for_mediaplayer (CairoDesklet *pDesklet, cairo_t *pSourceContext)
{
	g_return_if_fail (pDesklet != NULL && pSourceContext != NULL);
	CDMediaplayerParameters *pMediaplayer = (CDMediaplayerParameters *) pDesklet->pRendererData;
	
	Icon *pIcon = pDesklet->pIcon;
	g_return_if_fail (pIcon != NULL);
	if (pMediaplayer != NULL)
	{
		if (pMediaplayer->bControlButton) //Certain voudrons uniquement l'info, d'autre l'info + les boutons de controle
			pIcon->fWidth = (pDesklet->container.iHeight - myDocksParam.iDockRadius) / 4 * 3; 
		else
			pIcon->fWidth = pDesklet->container.iHeight - myDocksParam.iDockRadius; 
		
		pIcon->fWidth = MAX (1, pIcon->fWidth); 
		pIcon->fHeight = pIcon->fWidth; //L'icône aura la même taille en W et en H pour afficher le texte sur le coté
		//Du coup l'utilisateur pourra alonger le W pour que le texte soit visible
	}
	else
	{
		pIcon->fWidth = MAX (1, pDesklet->container.iWidth - myDocksParam.iDockRadius);  // 2 * myDocksParam.iDockRadius/2
		pIcon->fHeight = MAX (1, pDesklet->container.iHeight - myDocksParam.iDockRadius); //Icône de taille normal
	}
	pIcon->iImageWidth = pIcon->fWidth;
	pIcon->iImageHeight = pIcon->fHeight;
	pIcon->fDrawX = .5 * myDocksParam.iDockRadius;
	pIcon->fDrawY = .5 * myDocksParam.iDockRadius;
	pIcon->fScale = 1;
	
	cd_debug ("%s (%.2fx%.2f)\n", __func__, pIcon->fWidth, pIcon->fHeight);
	cairo_dock_fill_icon_buffers_for_desklet (pIcon, pSourceContext);
	
	GList* ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		pIcon->fWidth = pDesklet->pIcon->fWidth / 5.;
		pIcon->fHeight = pIcon->fWidth;
		cairo_dock_fill_icon_buffers_for_desklet (pIcon, pSourceContext);
	}
}


void rendering_draw_mediaplayer_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet)
{
	CDMediaplayerParameters *pMediaplayer = (CDMediaplayerParameters *) pDesklet->pRendererData;
	Icon *pIcon;
	Icon *pMainIcon = pDesklet->pIcon;
	GList *ic;
	
	if (pMediaplayer->bControlButton)
	{
		int i = 1;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next) {
			pIcon = ic->data;
			pIcon->fScale = 1.;
			pIcon->fAlpha = 1.;
			pIcon->fDrawX = i * (pMainIcon->fWidth / pMediaplayer->iNbIcons) - pIcon->fWidth;
			if (i <= pMediaplayer->iIconsLimit)
				pIcon->fDrawY = ((pDesklet->container.iHeight - myDocksParam.iDockRadius) - pMediaplayer->fBandWidth) + (pMediaplayer->fIconBandOffset * (i - 1));
			else
				pIcon->fDrawY = ((pDesklet->container.iHeight - myDocksParam.iDockRadius) - pMediaplayer->fBandWidth) + (pMediaplayer->fIconBandOffset * (pMediaplayer->iNbIcons - i));
			i++;
		}
	}
	
	pIcon = pMainIcon;
	cairo_save (pCairoContext);
	cairo_translate (pCairoContext, pIcon->fDrawX, pIcon->fDrawY);
	if (pIcon->pIconBuffer != NULL) //On dessine l'icône
	{
		cairo_set_source_surface (pCairoContext,
			pIcon->pIconBuffer,
			0.,
			0.);
		cairo_paint (pCairoContext);
	}
	if (pIcon->pQuickInfoBuffer != NULL) //On dessine la quickinfo
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
	
	cairo_restore (pCairoContext);
	if (pMediaplayer != NULL) //On dessine nos informations
	{
		if (pMediaplayer->pArtistSurface != NULL)
		{
			double fX = pIcon->fWidth + 5, fY = pIcon->fHeight / 3;
			cairo_set_source_surface (pCairoContext, pMediaplayer->pArtistSurface, fX, fY);
			cairo_paint (pCairoContext);
		}
		if (pMediaplayer->pTitleSurface != NULL)
		{
			double fX = pIcon->fWidth + 5, fY = (pIcon->fHeight / 3) * 2;
			cairo_set_source_surface (pCairoContext, pMediaplayer->pTitleSurface, fX, fY);
			cairo_paint (pCairoContext);
		}
	}
	
	if (pMediaplayer->bControlButton) // On dessine nos icônes prev, play, stop, next.
	{
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next) {
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->container.iWidth);
				cairo_restore (pCairoContext);
			}
		}
	}
}

void rendering_update_text_for_mediaplayer (CairoDesklet *pDesklet, gpointer *pNewData)
{
	CDMediaplayerParameters *pMediaplayer = (CDMediaplayerParameters *) pDesklet->pRendererData;
	if (pMediaplayer == NULL)
		return;
	
	//On reset tout!
	if (pMediaplayer->pArtistSurface != NULL)
	{
		cairo_surface_destroy (pMediaplayer->pArtistSurface);
		pMediaplayer->pArtistSurface = NULL;
	}
	if (pMediaplayer->pTitleSurface != NULL)
	{
		cairo_surface_destroy (pMediaplayer->pTitleSurface);
		pMediaplayer->pTitleSurface = NULL;
	}
	
	//On réattribue les textes
	pMediaplayer->cArtist = pNewData[0];
	pMediaplayer->cTitle = pNewData[1];
	
	if (pMediaplayer->cArtist != NULL)
		pMediaplayer->pArtistSurface = cairo_dock_create_surface_from_text_full (pMediaplayer->cArtist,
			&myIconsParam.iconTextDescription,
			cairo_dock_get_max_scale (pDesklet),
			pDesklet->container.iWidth,
			&pMediaplayer->fArtistWidth, &pMediaplayer->fArtistHeight, &pMediaplayer->fArtistXOffset, &pMediaplayer->fArtistYOffset);
	if (pMediaplayer->cTitle != NULL)
		pMediaplayer->pTitleSurface = cairo_dock_create_surface_from_text_full (pMediaplayer->cTitle,
			&myIconsParam.iconTextDescription,
			cairo_dock_get_max_scale (pDesklet),
			pDesklet->container.iWidth,
			&pMediaplayer->fTitleWidth, &pMediaplayer->fTitleHeight, &pMediaplayer->fTitleXOffset, &pMediaplayer->fTitleYOffset);
	
	cd_debug ("");
}

void rendering_register_mediaplayer_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render = rendering_draw_mediaplayer_in_desklet ;
	pRenderer->configure = rendering_configure_mediaplayer;
	pRenderer->load_data = rendering_load_mediaplayer_data;
	pRenderer->free_data = rendering_free_mediaplayer_data;
	pRenderer->load_icons = rendering_load_icons_for_mediaplayer;
	pRenderer->update = rendering_update_text_for_mediaplayer;
	
	cairo_dock_register_desklet_renderer (MY_APPLET_MEDIAPLAYER_DESKLET_RENDERER_NAME, pRenderer);
}
