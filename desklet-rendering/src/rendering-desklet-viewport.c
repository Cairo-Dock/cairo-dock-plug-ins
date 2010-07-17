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

#include "rendering-desklet-viewport.h"

#define _cairo_dock_set_path_as_current(...) _cairo_dock_set_vertex_pointer(pVertexTab)


static gboolean on_enter_icon_viewport (gpointer pUserData, Icon *pPointedIcon, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	gtk_widget_queue_draw (pContainer->pWidget);  // et oui, on n'a rien d'autre a faire.
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

static inline void _viewport_pan_delta(CairoDesklet *pDesklet, double fDeltaX, double fDeltaY)
{
	CDViewportParameters *pViewportConf = (CDViewportParameters *) pDesklet->pRendererData;
	pViewportConf->fCurrentPanXSpeed = fDeltaX;
	pViewportConf->fCurrentPanYSpeed = fDeltaY;
	pViewportConf->iCurrentOffsetX += fDeltaX;
	pViewportConf->iCurrentOffsetY += fDeltaY;
	if (pViewportConf->iCurrentOffsetX < 0)
	{
		pViewportConf->iCurrentOffsetX = 0;
		pViewportConf->fCurrentPanXSpeed = 0;
	}
	else if( pViewportConf->iCurrentOffsetX > pViewportConf->iMaxOffsetX )
	{
		pViewportConf->iCurrentOffsetX = pViewportConf->iMaxOffsetX;
		pViewportConf->fCurrentPanXSpeed = 0;
	}
	if (pViewportConf->iCurrentOffsetY < 0)
	{
		pViewportConf->iCurrentOffsetY = 0;
		pViewportConf->fCurrentPanYSpeed = 0;
	}
	else if( pViewportConf->iCurrentOffsetY > pViewportConf->iMaxOffsetY )
	{
		pViewportConf->iCurrentOffsetY = pViewportConf->iMaxOffsetY;
		pViewportConf->fCurrentPanYSpeed = 0;
	}

	gtk_widget_queue_draw (pDesklet->container.pWidget);
}

static gboolean on_update_desklet (gpointer pUserData, CairoDesklet *pDesklet, gboolean *bContinueAnimation)
{
	if (pDesklet->icons != NULL)
	{
		CDViewportParameters *pViewportConf = (CDViewportParameters *) pDesklet->pRendererData;
		if (pViewportConf == NULL)
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		
		if (! pDesklet->container.bInside)  // on est en-dehors du desklet, on ralentit.
		{
			_viewport_pan_delta (pDesklet, pViewportConf->fCurrentPanXSpeed*.85, pViewportConf->fCurrentPanYSpeed*.85);
			if (fabs (pViewportConf->fCurrentPanXSpeed)+fabs (pViewportConf->fCurrentPanYSpeed) < pViewportConf->iIconSize/15)
			// vitesse de translation epsilonesque, on quitte.
			{
				pViewportConf->fCurrentPanXSpeed = 0;
				pViewportConf->fCurrentPanYSpeed = 0;
				return CAIRO_DOCK_LET_PASS_NOTIFICATION;
			}
			*bContinueAnimation = TRUE;
		}
		else
		{
			double fDeltaX = 0;
			double fDeltaY = 0;
			// si on est dans la marge de 20% de la largeur du desklet a gauche,
			// alors on translate a droite
			if (pDesklet->container.iMouseX <= pDesklet->container.iWidth*0.2)
			{
				// La force de translation va de 0 (lorsqu'on est a 20%) jusqu'a
				// pViewportConf->iIconSize / 2. (lorsqu'on est a 0%)
				fDeltaX = (pViewportConf->iIconSize / 10) *
									(pDesklet->container.iWidth*0.2 - pDesklet->container.iMouseX)/(pDesklet->container.iWidth*0.2);
				*bContinueAnimation = TRUE;
			}
			// si on est dans la marge de 20% de la largeur du desklet a droite,
			// alors on translate a gauche (-1)
			else if( pDesklet->container.iMouseX >= pDesklet->container.iWidth*0.8 )
			{
				// La force de translation va de 0 (lorsqu'on est a 80%) jusqu'a
				// pViewportConf->iIconSize / 2. (lorsqu'on est a 100%)
				fDeltaX = -(pViewportConf->iIconSize / 10) *
									 (pDesklet->container.iMouseX - pDesklet->container.iWidth*0.8)/(pDesklet->container.iWidth*0.2);
				*bContinueAnimation = TRUE;
			}
			// si on est dans la marge de 20% de la hauteur du desklet en haut,
			// alors on translate en bas
			if (pDesklet->container.iMouseY <= pDesklet->container.iHeight*0.2)
			{
				// La force de translation va de 0 (lorsqu'on est a 20%) jusqu'a
				// pViewportConf->iIconSize / 2. (lorsqu'on est a 0%)
				fDeltaY = -(pViewportConf->iIconSize / 10) *
									 (pDesklet->container.iHeight*0.2 - pDesklet->container.iMouseY)/(pDesklet->container.iHeight*0.2);
				*bContinueAnimation = TRUE;
			}
			// si on est dans la marge de 20% de la hauteur du desklet en bas,
			// alors on translate en haut (-1)
			else if( pDesklet->container.iMouseY >= pDesklet->container.iHeight*0.8 )
			{
				// La force de translation va de 0 (lorsqu'on est a 80%) jusqu'a
				// pViewportConf->iIconSize / 2. (lorsqu'on est a 100%)
				fDeltaY = (pViewportConf->iIconSize / 10) *
									(pDesklet->container.iMouseY - pDesklet->container.iHeight*0.8)/(pDesklet->container.iHeight*0.2);
				*bContinueAnimation = TRUE;
			}
			if( *bContinueAnimation == TRUE )
			{
				_viewport_pan_delta( pDesklet, fDeltaX, fDeltaY );
			}
			else
			{
				pViewportConf->fCurrentPanXSpeed = 0.;
				pViewportConf->fCurrentPanYSpeed = 0.;
			}
		}
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

static gboolean on_mouse_move (gpointer pUserData, CairoDesklet *pDesklet, gboolean *bStartAnimation)
{
	if (pDesklet->icons != NULL)
	{
		CDViewportParameters *pViewportConf = (CDViewportParameters *) pDesklet->pRendererData;
		if (pViewportConf == NULL)
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		if (pViewportConf->bInfiniteWidth && (pDesklet->container.iMouseX <= pDesklet->container.iWidth*0.2 || pDesklet->container.iMouseX >= pDesklet->container.iWidth*0.8))
			*bStartAnimation = TRUE;
		if (pViewportConf->bInfiniteHeight && (pDesklet->container.iMouseY <= pDesklet->container.iHeight*0.2 || pDesklet->container.iMouseY >= pDesklet->container.iHeight*0.8))
			*bStartAnimation = TRUE;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

static CDViewportParameters *configure (CairoDesklet *pDesklet, gpointer *pConfig)  // gboolean, int, gdouble[4]
{
	CDViewportParameters *pViewportConf = g_new0 (CDViewportParameters, 1);
	if (pConfig != NULL)
	{
		pViewportConf->bRoundedRadius = GPOINTER_TO_INT (pConfig[0]);
		pViewportConf->iRadius = GPOINTER_TO_INT (pConfig[1]);
		if (pConfig[2] != NULL)
			memcpy (pViewportConf->fLineColor, pConfig[2], 4 * sizeof (gdouble));
	}
	
	pViewportConf->iLineWidth = 2;
	pViewportConf->iGapBetweenIcons = 10;
	pViewportConf->iMinimumIconSize = 48;
	pViewportConf->iCurrentOffsetX = 0;
	pViewportConf->iCurrentOffsetY = 0;
	pViewportConf->fCurrentPanXSpeed = 0;
	pViewportConf->fCurrentPanYSpeed = 0;
	pViewportConf->iMaxOffsetX = 0;
	pViewportConf->iMaxOffsetY = 0;
	pViewportConf->bInfiniteHeight=TRUE;
	pViewportConf->bInfiniteWidth=FALSE;
	
	cairo_dock_register_notification_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_ENTER_ICON, (CairoDockNotificationFunc) on_enter_icon_viewport, CAIRO_DOCK_RUN_FIRST, NULL);
	
	cairo_dock_register_notification_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_UPDATE_DESKLET, (CairoDockNotificationFunc) on_update_desklet, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_MOUSE_MOVED, (CairoDockNotificationFunc) on_mouse_move, CAIRO_DOCK_RUN_AFTER, NULL);
	
	return pViewportConf;
}


static inline void _compute_icons_grid (CairoDesklet *pDesklet, CDViewportParameters *pViewportConf)
{
	pViewportConf->fMargin = (pViewportConf->bRoundedRadius ?
		.5 * pViewportConf->iLineWidth + (1. - sqrt (2) / 2) * pViewportConf->iRadius :
		.5 * pViewportConf->iLineWidth + .5 * pViewportConf->iRadius);
	
	int iNbIcons = 0;
	Icon *pIcon;
	GList *ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			iNbIcons ++;
	}
	pViewportConf->iNbIcons = iNbIcons;
	
	double w = pDesklet->container.iWidth - 2 * pViewportConf->fMargin;
	double h = pDesklet->container.iHeight - 2 * pViewportConf->fMargin;
	int dh = myLabels.iLabelSize;  // taille verticale ajoutee a chaque icone.
	int dw = 2 * dh;  // taille horizontale ajoutee a chaque icone.
	int di = pViewportConf->iGapBetweenIcons;  // ecart entre 2 lignes/colonnes.
	
	int p, q;  // nombre de lignes et colonnes.
	int iSize;
	pViewportConf->iIconSize = 0, pViewportConf->iNbLines = 0, pViewportConf->iNbColumns = 0;
	//g_print ("%d icones sur %dx%d (%d)\n", pViewportConf->iNbIcons, (int)w, (int)h, myLabels.iLabelSize);
	for (p = 1; p <= pViewportConf->iNbIcons; p ++)
	{
		q = (int) ceil ((double)pViewportConf->iNbIcons / p);
		iSize = MIN ((h - (p - 1) * di) / p - dh, (w - (q - 1) * di) / q - dw);
		//g_print ("  %dx%d -> %d\n", p, q, iSize);
		if (iSize > pViewportConf->iIconSize)
		{
			pViewportConf->iIconSize = iSize;
			pViewportConf->iNbLines = p;
			pViewportConf->iNbColumns = q;
		}
		else if(iSize > 0) // there is only one maximum
		{
			break;
		}
	}
	// si les icones sont trop petites, et qu'on a une largeur et/ou une
	// hauteur infinie(s), essayer d'avoir au moins une taille minimale
	if(  pViewportConf->iIconSize < pViewportConf->iMinimumIconSize &&
	    (pViewportConf->bInfiniteWidth || pViewportConf->bInfiniteHeight) )
	{
		if( pViewportConf->bInfiniteWidth && pViewportConf->bInfiniteHeight )
		{
			// surface infinie: on garde le meme nb de colonnes&lignes,
			// mais on met la taille d'icone a iMinimumIconSize
			pViewportConf->iIconSize = pViewportConf->iMinimumIconSize;
		}
		else if( pViewportConf->bInfiniteHeight )
		{
			// hauteur infinie et largeur fixe: on calcule le nombre de colonnes
			// maxi avec pViewportConf->iIconSize = pViewportConf->iMinimumIconSize
			pViewportConf->iIconSize = pViewportConf->iMinimumIconSize;
			pViewportConf->iNbColumns = (w + di) / ( pViewportConf->iIconSize + dw + di );
			if( pViewportConf->iNbColumns < 1 )
			{
				pViewportConf->iNbColumns = 1;
				pViewportConf->iIconSize = w - dw; 
			}
			pViewportConf->iNbLines = (int) ceil ((double)pViewportConf->iNbIcons / pViewportConf->iNbColumns);
		}
		else if( pViewportConf->bInfiniteWidth )
		{
			// largeur infinie et hauteur fixe: on calcule le nombre de lignes
			// maxi avec pViewportConf->iIconSize = pViewportConf->iMinimumIconSize
			pViewportConf->iIconSize = pViewportConf->iMinimumIconSize;
			pViewportConf->iNbLines = (h + di) / ( pViewportConf->iIconSize + dh + di );
			if( pViewportConf->iNbLines < 1 )
			{
				pViewportConf->iNbLines = 1;
				pViewportConf->iIconSize = h - dh; 
			}
			pViewportConf->iNbColumns = (int) ceil ((double)pViewportConf->iNbIcons / pViewportConf->iNbLines);
		}
		// on calcule l'offset maximal atteignable en X
		pViewportConf->iMaxOffsetX = MAX(( pViewportConf->iIconSize + dw + di )*pViewportConf->iNbColumns - (w + di), 0);
		// on calcule l'offset maximal atteignable en Y
		pViewportConf->iMaxOffsetY = MAX(( pViewportConf->iIconSize + dh + di )*pViewportConf->iNbLines - (h + di), 0);
	}
}

static void load_data (CairoDesklet *pDesklet)
{
	CDViewportParameters *pViewportConf = (CDViewportParameters *) pDesklet->pRendererData;
	if (pViewportConf == NULL)
		return ;
	
	_compute_icons_grid (pDesklet, pViewportConf);
}


static void free_data (CairoDesklet *pDesklet)
{
	cairo_dock_remove_notification_func_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_ENTER_ICON, (CairoDockNotificationFunc) on_enter_icon_viewport, NULL);
	
	CDViewportParameters *pViewportConf = (CDViewportParameters *) pDesklet->pRendererData;
	if (pViewportConf == NULL)
		return ;
	
	g_free (pViewportConf);
	pDesklet->pRendererData = NULL;
}


static void set_icon_size (CairoDesklet *pDesklet, Icon *pIcon)
{
	CDViewportParameters *pViewportConf = (CDViewportParameters *) pDesklet->pRendererData;
	if (pViewportConf == NULL)
		return ;
	
	if (pIcon == pDesklet->pIcon)
	{
		pIcon->fWidth = 0.;
		pIcon->fHeight = 0.;
	}
	else
	{
		pIcon->fWidth = pViewportConf->iIconSize;
		pIcon->fHeight = pViewportConf->iIconSize;
	}
}

static void calculate_icons (CairoDesklet *pDesklet)
{
	CDViewportParameters *pViewportConf = (CDViewportParameters *) pDesklet->pRendererData;
	if (pViewportConf == NULL)
		return ;
	
	_compute_icons_grid (pDesklet, pViewportConf);
	cd_debug ("pViewportConf->iIconSize : %d\n", pViewportConf->iIconSize);
	
	Icon *pIcon = pDesklet->pIcon;
	if (pIcon != NULL)  // on ne veut pas charger cette icone.
	{
		pIcon->fWidth = -1;
		pIcon->fHeight = -1;
	}
	
	GList* ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			pIcon->fWidth = -1;
			pIcon->fHeight = -1;
		}
		else
		{
			pIcon->fWidth = pViewportConf->iIconSize;
			pIcon->fHeight = pViewportConf->iIconSize;
		
			pIcon->fScale = 1.;
			pIcon->fAlpha = 1.;
			pIcon->fWidthFactor = 1.;
			pIcon->fHeightFactor = 1.;
			pIcon->fGlideScale = 1.;
		}
	}
}


static void render (cairo_t *pCairoContext, CairoDesklet *pDesklet)
{
	CDViewportParameters *pViewportConf = (CDViewportParameters *) pDesklet->pRendererData;
	//g_print ("%s(%x)\n", __func__, pViewportConf);
	if (pViewportConf == NULL)
		return ;
	
	double fRadius = pViewportConf->iRadius;
	double fLineWidth = pViewportConf->iLineWidth;
	// le cadre.
	cairo_set_line_width (pCairoContext, pViewportConf->iLineWidth);
	if (pViewportConf->bRoundedRadius)
	{
		cairo_translate (pCairoContext, 0., .5 * fLineWidth);
		cairo_dock_draw_rounded_rectangle (pCairoContext,
			fRadius,
			fLineWidth,
			pDesklet->container.iWidth - 2 * fRadius - fLineWidth,
			pDesklet->container.iHeight - 2*fLineWidth);
	}
	else
	{
		cairo_move_to (pCairoContext, 0., 0.);
		cairo_rel_line_to (pCairoContext,
			0.,
			pDesklet->container.iHeight - fRadius - fLineWidth);
		cairo_rel_line_to (pCairoContext,
			pViewportConf->iRadius,
			pViewportConf->iRadius);
		cairo_rel_line_to (pCairoContext,
			pDesklet->container.iWidth - fRadius - fLineWidth,
			0.);
	}
	cairo_set_source_rgba (pCairoContext, pViewportConf->fLineColor[0], pViewportConf->fLineColor[1], pViewportConf->fLineColor[2], pViewportConf->fLineColor[3]);
	cairo_stroke (pCairoContext);
	
	// les icones.
	double w = pDesklet->container.iWidth - 2 * pViewportConf->fMargin;
	double h = pDesklet->container.iHeight - 2 * pViewportConf->fMargin;
	int dh = myLabels.iLabelSize;  // taille verticale ajoutee a chaque icone.
	int dw = 2 * dh;  // taille horizontale ajoutee a chaque icone.
	if( pViewportConf->iMaxOffsetY == 0 )
	{
		dh = (h - pViewportConf->iNbLines * (pViewportConf->iIconSize + myLabels.iLabelSize)) / pViewportConf->iNbLines;  // ecart entre 2 lignes.
	}
	if( pViewportConf->iMaxOffsetX == 0 )
	{
		dw = (w - pViewportConf->iNbColumns * pViewportConf->iIconSize) / pViewportConf->iNbColumns;  // ecart entre 2 colonnes.
	}
	
	// on determine la 1ere icone a tracer : l'icone suivant l'icone pointee.
	
	double x = pViewportConf->fMargin + dw/2, y = pViewportConf->fMargin + dh/2;
	int q = 0;
	Icon *pIcon;
	GList *ic;
	GList *pVisibleIcons = NULL;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			continue;
		
		pIcon->fDrawX = x - pViewportConf->iCurrentOffsetX;
		pIcon->fDrawY = y - pViewportConf->iCurrentOffsetY;
		
		x += pViewportConf->iIconSize + dw;
		q ++;
		if (q == pViewportConf->iNbColumns)
		{
			q = 0;
			x = pViewportConf->fMargin + dw/2;
			y += pViewportConf->iIconSize + myLabels.iLabelSize + dh;
		}
		// On ne dessine que les icones qui sont visibles
		if( pIcon->fDrawX - pViewportConf->fMargin + dw/2 >= 0                &&
		    pIcon->fDrawY - pViewportConf->fMargin + myLabels.iLabelSize >= 0 &&
		    pIcon->fDrawX - pViewportConf->fMargin + dw/2 <= w - (pViewportConf->iIconSize + dw)          &&
		    pIcon->fDrawY - pViewportConf->fMargin + myLabels.iLabelSize <= h - (pViewportConf->iIconSize + myLabels.iLabelSize + dh))
		{
			pVisibleIcons = g_list_append(pVisibleIcons, pIcon);
		}
	}
	
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pVisibleIcons);
	if (pFirstDrawnElement == NULL)
		return;
	ic = pFirstDrawnElement;
	do
	{
		pIcon = ic->data;
		if (pIcon->pIconBuffer != NULL && ! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			cairo_save (pCairoContext);
			
			cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, FALSE, pDesklet->container.iWidth);
			
			cairo_restore (pCairoContext);
			
			
			if (pIcon->pTextBuffer != NULL)
			{
				cairo_save (pCairoContext);
				cairo_translate (pCairoContext, pIcon->fDrawX, pIcon->fDrawY);
				
				double fOffsetX = 0., fAlpha;
				if (pIcon->bPointed)
				{
					fAlpha = 1.;
					if (pIcon->fDrawX + pIcon->fWidth/2 + pIcon->iTextWidth/2 > pDesklet->container.iWidth)
						fOffsetX = pDesklet->container.iWidth - (pIcon->fDrawX + pIcon->fWidth/2 + pIcon->iTextWidth/2);
					if (pIcon->fDrawX + pIcon->fWidth/2 - pIcon->iTextWidth/2 < 0)
						fOffsetX = pIcon->iTextWidth/2 - (pIcon->fDrawX + pIcon->fWidth/2);
					cairo_set_source_surface (pCairoContext,
						pIcon->pTextBuffer,
						fOffsetX + pIcon->fWidth/2 - pIcon->iTextWidth/2,
						-myLabels.iLabelSize);
					cairo_paint_with_alpha (pCairoContext, fAlpha);
				}
				else
				{
					fAlpha = .6;
					if (pIcon->iTextWidth > pIcon->fWidth + 2 * myLabels.iLabelSize)
					{
						fOffsetX = - myLabels.iLabelSize;
						cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (fOffsetX,
							0.,
							fOffsetX + pIcon->fWidth + 2*myLabels.iLabelSize,
							0.);
						cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
						cairo_pattern_add_color_stop_rgba (pGradationPattern,
							0.,
							0.,
							0.,
							0.,
							fAlpha);
						cairo_pattern_add_color_stop_rgba (pGradationPattern,
							0.75,
							0.,
							0.,
							0.,
							fAlpha);
						cairo_pattern_add_color_stop_rgba (pGradationPattern,
							1.,
							0.,
							0.,
							0.,
							0.);
						cairo_set_source_surface (pCairoContext,
							pIcon->pTextBuffer,
							fOffsetX,
							-myLabels.iLabelSize);
						cairo_mask (pCairoContext, pGradationPattern);
						cairo_pattern_destroy (pGradationPattern);
					}
					else
					{
						fOffsetX = pIcon->fWidth/2 - pIcon->iTextWidth/2;
						cairo_set_source_surface (pCairoContext,
							pIcon->pTextBuffer,
							fOffsetX,
							-myLabels.iLabelSize);
						cairo_paint_with_alpha (pCairoContext, fAlpha);
					}
				}
				
				cairo_restore (pCairoContext);
			}
		}
		ic = cairo_dock_get_next_element (ic, pVisibleIcons);
	}
	while (ic != pFirstDrawnElement);
}


static void render_opengl (CairoDesklet *pDesklet)
{
	CDViewportParameters *pViewportConf = (CDViewportParameters *) pDesklet->pRendererData;
	if (pViewportConf == NULL)
		return ;
	
	// le cadre.
	double fRadius = (pViewportConf->bRoundedRadius ? pViewportConf->iRadius : 0.);
	double fLineWidth = pViewportConf->iLineWidth;
	if (fLineWidth != 0 && pViewportConf->fLineColor[3] != 0)
	{
		cairo_dock_draw_rounded_rectangle_opengl (pDesklet->container.iWidth - 2 * fRadius,
			pDesklet->container.iHeight,
			fRadius,
			fLineWidth,
			pViewportConf->fLineColor);
		glTranslatef (-pDesklet->container.iWidth/2, -pDesklet->container.iHeight/2, 0.);
	}
	
	glTranslatef (-pDesklet->container.iWidth/2, -pDesklet->container.iHeight/2, 0.);
	
	// les icones.
	double w = pDesklet->container.iWidth - 2 * pViewportConf->fMargin;
	double h = pDesklet->container.iHeight - 2 * pViewportConf->fMargin;
	int dh = myLabels.iLabelSize;  // taille verticale ajoutee a chaque icone.
	int dw = 2 * dh;  // taille horizontale ajoutee a chaque icone.
	if( pViewportConf->iMaxOffsetY == 0 )
	{
		// ecart entre 2 lignes si il faut repartir vertivalement les icones.
		dh = (h - pViewportConf->iNbLines * (pViewportConf->iIconSize + myLabels.iLabelSize) - 2*pViewportConf->fMargin - myLabels.iLabelSize) / pViewportConf->iNbLines;
	}
	if( pViewportConf->iMaxOffsetX == 0 )
	{
		// ecart entre 2 colonnes si il faut repartir horizontalement les icones.
		dw = (w - pViewportConf->iNbColumns * pViewportConf->iIconSize - 2*pViewportConf->fMargin) / pViewportConf->iNbColumns;
	}
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (1.);
	
	
	double x = pViewportConf->fMargin + dw/2, y = pViewportConf->fMargin + myLabels.iLabelSize + dh/2;
	int q = 0;
	Icon *pIcon;
	GList *ic;
	GList *pVisibleIcons = NULL;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			continue;
		
		pIcon->fDrawX = x - pViewportConf->iCurrentOffsetX;
		pIcon->fDrawY = y - pViewportConf->iCurrentOffsetY;
		
		x += pViewportConf->iIconSize + dw;
		q ++;
		if (q == pViewportConf->iNbColumns)
		{
			q = 0;
			x = pViewportConf->fMargin + dw/2;
			y += pViewportConf->iIconSize + myLabels.iLabelSize + dh;
		}
		// On ne dessine que les icones qui sont visibles
		if( pIcon->fDrawX - pViewportConf->fMargin - dw/2 >= 0                &&
		    pIcon->fDrawY - pViewportConf->fMargin - myLabels.iLabelSize - dh/2 >= 0 &&
		    pIcon->fDrawX - pViewportConf->fMargin - dw/2 <= w - (pViewportConf->iIconSize + dw/2)          &&
		    pIcon->fDrawY - pViewportConf->fMargin - myLabels.iLabelSize - dh/2 <= h - (pViewportConf->iIconSize + myLabels.iLabelSize + dh/2))
		{
			pVisibleIcons = g_list_append(pVisibleIcons, pIcon);
		}
	}
	
	
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pVisibleIcons);
	if (pFirstDrawnElement == NULL)
		return;
	ic = pFirstDrawnElement;
	do
	{
		pIcon = ic->data;
		
		if (pIcon->iIconTexture != 0 && ! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			glPushMatrix ();
			
			glTranslatef (pIcon->fDrawX + pIcon->fWidth/2,
				pDesklet->container.iHeight - pIcon->fDrawY - pIcon->fHeight/2,
				0.);
			//g_print (" %d) %d;%d %dx%d\n", pIcon->iIconTexture, (int)(pIcon->fDrawX + pIcon->fWidth/2), (int)(pDesklet->container.iHeight - pIcon->fDrawY - pIcon->fHeight/2), (int)(pIcon->fWidth/2), (int)(pIcon->fHeight/2));
			_cairo_dock_apply_texture_at_size (pIcon->iIconTexture, pIcon->fWidth, pIcon->fHeight);
			
			/// generer une notification ...
			/*if (pIcon->bHasIndicator && g_pIndicatorBuffer.iTexture != 0)
			{
				glPushMatrix ();
				glTranslatef (0., - pIcon->fHeight/2 + g_pIndicatorBuffer.iHeight/2 * pIcon->fWidth / g_pIndicatorBuffer.iWidth, 0.);
				_cairo_dock_apply_texture_at_size (g_pIndicatorBuffer.iTexture, pIcon->fWidth, g_pIndicatorBuffer.iHeight * pIcon->fWidth / g_pIndicatorBuffer.iWidth);
				glPopMatrix ();
			}*/
			
			if (pIcon->iLabelTexture != 0)
			{
				glPushMatrix ();
				
				double dx = .5 * (pIcon->iTextWidth & 1);  // on decale la texture pour la coller sur la grille des coordonnees entieres.
				double dy = .5 * (pIcon->iTextHeight & 1);
				double u0 = 0., u1 = 1.;
				double fOffsetX = 0.;
				if (pIcon->bPointed)
				{
					_cairo_dock_set_alpha (1.);
					if (pIcon->fDrawX + pIcon->fWidth/2 + pIcon->iTextWidth/2 > pDesklet->container.iWidth)
						fOffsetX = pDesklet->container.iWidth - (pIcon->fDrawX + pIcon->fWidth/2 + pIcon->iTextWidth/2);
					if (pIcon->fDrawX + pIcon->fWidth/2 - pIcon->iTextWidth/2 < 0)
						fOffsetX = pIcon->iTextWidth/2 - (pIcon->fDrawX + pIcon->fWidth/2);
				}
				else
				{
					_cairo_dock_set_alpha (.6);
					if (pIcon->iTextWidth > pIcon->fWidth + 2 * myLabels.iLabelSize)
					{
						fOffsetX = 0.;
						u1 = (double) (pIcon->fWidth + 2 * myLabels.iLabelSize) / pIcon->iTextWidth;
					}
				}
				
				glTranslatef (ceil (fOffsetX) + dx, ceil (pIcon->fHeight/2 + pIcon->iTextHeight / 2) + dy, 0.);
				
				glBindTexture (GL_TEXTURE_2D, pIcon->iLabelTexture);
				_cairo_dock_apply_current_texture_portion_at_size_with_offset (u0, 0.,
					u1 - u0, 1.,
					pIcon->iTextWidth * (u1 - u0), pIcon->iTextHeight,
					0., 0.);
				_cairo_dock_set_alpha (1.);
				
				glPopMatrix ();
			}
			
			if (pIcon->iQuickInfoTexture != 0)
			{
				glTranslatef (0., (- pIcon->fHeight + pIcon->iQuickInfoHeight)/2, 0.);
				
				_cairo_dock_apply_texture_at_size (pIcon->iQuickInfoTexture,
					pIcon->iQuickInfoWidth,
					pIcon->iQuickInfoHeight);
			}
			
			glPopMatrix ();
		}

		ic = cairo_dock_get_next_element (ic, pVisibleIcons);
		
	} while (ic != pFirstDrawnElement);
	
	_cairo_dock_disable_texture ();
}



void rendering_register_viewport_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render 			= (CairoDeskletRenderFunc) render;
	pRenderer->configure 		= (CairoDeskletConfigureRendererFunc) configure;
	pRenderer->load_data 		= (CairoDeskletLoadRendererDataFunc) load_data;
	pRenderer->free_data 		= (CairoDeskletFreeRendererDataFunc) free_data;
	pRenderer->calculate_icons 	= (CairoDeskletCalculateIconsFunc) calculate_icons;
	pRenderer->render_opengl 	= (CairoDeskletGLRenderFunc) render_opengl;
	
	cairo_dock_register_desklet_renderer ("Viewport", pRenderer);
}
