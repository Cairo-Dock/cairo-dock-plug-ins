/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "rendering-desklet-mediaplayer.h"

extern CairoDockLabelDescription g_iconTextDescription;

CDMediaplayerParameters *rendering_configure_mediaplayer (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig)
{
	cd_debug ("");
	CDMediaplayerParameters *pMediaplayer = g_new0 (CDMediaplayerParameters, 1);
	if (pConfig != NULL)  // dessin de l'artiste et du titre sur le coté du desklet.
	{
		pMediaplayer->cArtist = pConfig[0];
		pMediaplayer->cTitle = pConfig[1];
		if (pMediaplayer->cArtist != NULL)
			pMediaplayer->pArtistSurface = cairo_dock_create_surface_from_text (pMediaplayer->cArtist,
			pSourceContext,
			&g_iconTextDescription,
			cairo_dock_get_max_scale (pDesklet),
			&pMediaplayer->fArtistWidth, &pMediaplayer->fArtistHeight, &pMediaplayer->fArtistXOffset, &pMediaplayer->fArtistYOffset);
		if (pMediaplayer->cTitle != NULL)
			pMediaplayer->pTitleSurface = cairo_dock_create_surface_from_text (pMediaplayer->cTitle,
			pSourceContext,
			&g_iconTextDescription,
			cairo_dock_get_max_scale (pDesklet),
			&pMediaplayer->fTitleWidth, &pMediaplayer->fTitleHeight, &pMediaplayer->fTitleXOffset, &pMediaplayer->fTitleYOffset);
		
		pMediaplayer->bControlButton = GPOINTER_TO_INT (pConfig[2]);
	}
	return pMediaplayer;
}

void rendering_free_mediaplayer_data (CairoDesklet *pDesklet)
{
	cd_debug ("");
	CDMediaplayerParameters *pMediaplayer = (CDMediaplayerParameters *) pDesklet->pRendererData;
	if (pMediaplayer == NULL)
		return;
	
	//On ne free pa cArtist et cTitle, ces vars appartiennent à l'applet.
	
	if (pMediaplayer->pArtistSurface != NULL)	
		cairo_surface_destroy (pMediaplayer->pArtistSurface);
	if (pMediaplayer->pTitleSurface != NULL)
		cairo_surface_destroy (pMediaplayer->pTitleSurface);
		
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
			pIcon->fWidth = (pDesklet->iHeight - g_iDockRadius) / 4 * 3; 
		else
			pIcon->fWidth = pDesklet->iHeight - g_iDockRadius; 
		
		pIcon->fWidth = MAX (1, pIcon->fWidth); 
		pIcon->fHeight = pIcon->fWidth; //L'icône aura la même taille en W et en H pour afficher le texte sur le coté
		//Du coup l'utilisateur pourra alonger le W pour que le texte soit visible
	}
	else
	{
		pIcon->fWidth = MAX (1, pDesklet->iWidth - g_iDockRadius);  // 2 * g_iDockRadius/2
		pIcon->fHeight = MAX (1, pDesklet->iHeight - g_iDockRadius); //Icône de taille normal
	}
	pIcon->fDrawX = .5 * g_iDockRadius;
	pIcon->fDrawY = .5 * g_iDockRadius;
	pIcon->fScale = 1;
	
	g_print ("%s (%.2fx%.2f)\n", __func__, pIcon->fWidth, pIcon->fHeight);
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


void rendering_draw_mediaplayer_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized)
{
	CDMediaplayerParameters *pMediaplayer = (CDMediaplayerParameters *) pDesklet->pRendererData;
	Icon *pIcon;
	Icon *pMainIcon = pDesklet->pIcon;
	GList *ic;
	
	if (pMediaplayer->bControlButton)
	{
		int iNbIcons = g_list_length (pDesklet->icons), i = 1, j = iNbIcons / 2;
		double fBandWidth = (pDesklet->iHeight - g_iDockRadius) / 4, fIconBandOffset = fBandWidth / iNbIcons;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next) {
			pIcon = ic->data;
			pIcon->fScale = 1.;
			pIcon->fAlpha = 1.;
			pIcon->fDrawX = i * (pMainIcon->fWidth / iNbIcons) - pIcon->fWidth;
			if (i <= j)
				pIcon->fDrawY = ((pDesklet->iHeight - g_iDockRadius) - fBandWidth) + (fIconBandOffset * (i - 1));
			else
				pIcon->fDrawY = ((pDesklet->iHeight - g_iDockRadius) - fBandWidth) + (fIconBandOffset * (iNbIcons - i));
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
				cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->iWidth);
				cairo_restore (pCairoContext);
			}
		}
	}
}

void rendering_register_mediaplayer_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render = rendering_draw_mediaplayer_in_desklet ;
	pRenderer->configure = rendering_configure_mediaplayer;
	pRenderer->load_data = NULL;
	pRenderer->free_data = rendering_free_mediaplayer_data;
	pRenderer->load_icons = rendering_load_icons_for_mediaplayer;
	
	cairo_dock_register_desklet_renderer (MY_APPLET_MEDIAPLAYER_DESKLET_RENDERER_NAME, pRenderer);
}
