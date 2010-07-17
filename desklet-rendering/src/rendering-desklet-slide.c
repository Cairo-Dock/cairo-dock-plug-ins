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

#include "rendering-desklet-slide.h"

#define _cairo_dock_set_path_as_current(...) _cairo_dock_set_vertex_pointer(pVertexTab)


static gboolean on_enter_icon_slide (gpointer pUserData, Icon *pPointedIcon, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	gtk_widget_queue_draw (pContainer->pWidget);  // et oui, on n'a rien d'autre a faire.
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

static inline void _slide_pan_delta(CairoDesklet *pDesklet, double fDeltaX, double fDeltaY)
{
	CDSlideParameters *pSlideParams = (CDSlideParameters *) pDesklet->pRendererData;
	pSlideParams->fCurrentPanXSpeed = fDeltaX;
	pSlideParams->fCurrentPanYSpeed = fDeltaY;
	pSlideParams->iCurrentOffsetX += fDeltaX;
	pSlideParams->iCurrentOffsetY += fDeltaY;
	if (pSlideParams->iCurrentOffsetX < 0)
	{
		pSlideParams->iCurrentOffsetX = 0;
		pSlideParams->fCurrentPanXSpeed = 0;
	}
	else if( pSlideParams->iCurrentOffsetX > pSlideParams->iMaxOffsetX )
	{
		pSlideParams->iCurrentOffsetX = pSlideParams->iMaxOffsetX;
		pSlideParams->fCurrentPanXSpeed = 0;
	}
	if (pSlideParams->iCurrentOffsetY < 0)
	{
		pSlideParams->iCurrentOffsetY = 0;
		pSlideParams->fCurrentPanYSpeed = 0;
	}
	else if( pSlideParams->iCurrentOffsetY > pSlideParams->iMaxOffsetY )
	{
		pSlideParams->iCurrentOffsetY = pSlideParams->iMaxOffsetY;
		pSlideParams->fCurrentPanYSpeed = 0;
	}

	gtk_widget_queue_draw (pDesklet->container.pWidget);
}

static gboolean on_update_desklet (gpointer pUserData, CairoDesklet *pDesklet, gboolean *bContinueAnimation)
{
	if (pDesklet->icons != NULL)
	{
		CDSlideParameters *pSlideParams = (CDSlideParameters *) pDesklet->pRendererData;
		if (pSlideParams == NULL)
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		
		if (! pDesklet->container.bInside)  // on est en-dehors du desklet, on ralentit.
		{
			_slide_pan_delta (pDesklet, pSlideParams->fCurrentPanXSpeed*.85, pSlideParams->fCurrentPanYSpeed*.85);
			if (fabs (pSlideParams->fCurrentPanXSpeed)+fabs (pSlideParams->fCurrentPanYSpeed) < pSlideParams->iIconSize/15)
			// vitesse de translation epsilonesque, on quitte.
			{
				pSlideParams->fCurrentPanXSpeed = 0;
				pSlideParams->fCurrentPanYSpeed = 0;
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
				// pSlideParams->iIconSize / 2. (lorsqu'on est a 0%)
				fDeltaX = (pSlideParams->iIconSize / 10) *
									(pDesklet->container.iWidth*0.2 - pDesklet->container.iMouseX)/(pDesklet->container.iWidth*0.2);
				*bContinueAnimation = TRUE;
			}
			// si on est dans la marge de 20% de la largeur du desklet a droite,
			// alors on translate a gauche (-1)
			else if( pDesklet->container.iMouseX >= pDesklet->container.iWidth*0.8 )
			{
				// La force de translation va de 0 (lorsqu'on est a 80%) jusqu'a
				// pSlideParams->iIconSize / 2. (lorsqu'on est a 100%)
				fDeltaX = -(pSlideParams->iIconSize / 10) *
									 (pDesklet->container.iMouseX - pDesklet->container.iWidth*0.8)/(pDesklet->container.iWidth*0.2);
				*bContinueAnimation = TRUE;
			}
			// si on est dans la marge de 20% de la hauteur du desklet en haut,
			// alors on translate en bas
			if (pDesklet->container.iMouseY <= pDesklet->container.iHeight*0.2)
			{
				// La force de translation va de 0 (lorsqu'on est a 20%) jusqu'a
				// pSlideParams->iIconSize / 2. (lorsqu'on est a 0%)
				fDeltaY = -(pSlideParams->iIconSize / 10) *
									 (pDesklet->container.iHeight*0.2 - pDesklet->container.iMouseY)/(pDesklet->container.iHeight*0.2);
				*bContinueAnimation = TRUE;
			}
			// si on est dans la marge de 20% de la hauteur du desklet en bas,
			// alors on translate en haut (-1)
			else if( pDesklet->container.iMouseY >= pDesklet->container.iHeight*0.8 )
			{
				// La force de translation va de 0 (lorsqu'on est a 80%) jusqu'a
				// pSlideParams->iIconSize / 2. (lorsqu'on est a 100%)
				fDeltaY = (pSlideParams->iIconSize / 10) *
									(pDesklet->container.iMouseY - pDesklet->container.iHeight*0.8)/(pDesklet->container.iHeight*0.2);
				*bContinueAnimation = TRUE;
			}
			if( *bContinueAnimation == TRUE )
			{
				_slide_pan_delta( pDesklet, fDeltaX, fDeltaY );
			}
			else
			{
				pSlideParams->fCurrentPanXSpeed = 0.;
				pSlideParams->fCurrentPanYSpeed = 0.;
			}
		}
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

static gboolean on_mouse_move (gpointer pUserData, CairoDesklet *pDesklet, gboolean *bStartAnimation)
{
	if (pDesklet->icons != NULL)
	{
		CDSlideParameters *pSlideParams = (CDSlideParameters *) pDesklet->pRendererData;
		if (pSlideParams == NULL)
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		if (pSlideParams->bInfiniteWidth && (pDesklet->container.iMouseX <= pDesklet->container.iWidth*0.2 || pDesklet->container.iMouseX >= pDesklet->container.iWidth*0.8))
			*bStartAnimation = TRUE;
		if (pSlideParams->bInfiniteHeight && (pDesklet->container.iMouseY <= pDesklet->container.iHeight*0.2 || pDesklet->container.iMouseY >= pDesklet->container.iHeight*0.8))
			*bStartAnimation = TRUE;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

static CDSlideParameters *configure (CairoDesklet *pDesklet, gpointer *pConfig)  // gboolean, int, gdouble[4]
{
	CDSlideParameters *pSlide = g_new0 (CDSlideParameters, 1);
	if (pConfig != NULL)
	{
		pSlide->bRoundedRadius = GPOINTER_TO_INT (pConfig[0]);
		pSlide->iRadius = GPOINTER_TO_INT (pConfig[1]);
		if (pConfig[2] != NULL)
			memcpy (pSlide->fLineColor, pConfig[2], 4 * sizeof (gdouble));
	}
	
	pSlide->iLineWidth = 2;
	pSlide->iGapBetweenIcons = 10;
	pSlide->iMinimumIconSize = 48;
	pSlide->iCurrentOffsetX = 0;
	pSlide->iCurrentOffsetY = 0;
	pSlide->fCurrentPanXSpeed = 0;
	pSlide->fCurrentPanYSpeed = 0;
	pSlide->iMaxOffsetX = 0;
	pSlide->iMaxOffsetY = 0;
	pSlide->bInfiniteHeight=TRUE;
	pSlide->bInfiniteWidth=FALSE;
	
	cairo_dock_register_notification_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_ENTER_ICON, (CairoDockNotificationFunc) on_enter_icon_slide, CAIRO_DOCK_RUN_FIRST, NULL);
	
	cairo_dock_register_notification_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_UPDATE_DESKLET, (CairoDockNotificationFunc) on_update_desklet, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_MOUSE_MOVED, (CairoDockNotificationFunc) on_mouse_move, CAIRO_DOCK_RUN_AFTER, NULL);
	
	return pSlide;
}


static inline void _compute_icons_grid (CairoDesklet *pDesklet, CDSlideParameters *pSlide)
{
	pSlide->fMargin = (pSlide->bRoundedRadius ?
		.5 * pSlide->iLineWidth + (1. - sqrt (2) / 2) * pSlide->iRadius :
		.5 * pSlide->iLineWidth + .5 * pSlide->iRadius);
	
	int iNbIcons = 0;
	Icon *pIcon;
	GList *ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			iNbIcons ++;
	}
	pSlide->iNbIcons = iNbIcons;
	
	double w = pDesklet->container.iWidth - 2 * pSlide->fMargin;
	double h = pDesklet->container.iHeight - 2 * pSlide->fMargin;
	int dh = myLabels.iLabelSize;  // taille verticale ajoutee a chaque icone.
	int dw = 2 * dh;  // taille horizontale ajoutee a chaque icone.
	int di = pSlide->iGapBetweenIcons;  // ecart entre 2 lignes/colonnes.
	
	int p, q;  // nombre de lignes et colonnes.
	int iSize;
	pSlide->iIconSize = 0, pSlide->iNbLines = 0, pSlide->iNbColumns = 0;
	//g_print ("%d icones sur %dx%d (%d)\n", pSlide->iNbIcons, (int)w, (int)h, myLabels.iLabelSize);
	for (p = 1; p <= pSlide->iNbIcons; p ++)
	{
		q = (int) ceil ((double)pSlide->iNbIcons / p);
		iSize = MIN ((h - (p - 1) * di) / p - dh, (w - (q - 1) * di) / q - dw);
		//g_print ("  %dx%d -> %d\n", p, q, iSize);
		if (iSize > pSlide->iIconSize)
		{
			pSlide->iIconSize = iSize;
			pSlide->iNbLines = p;
			pSlide->iNbColumns = q;
		}
		else if(iSize > 0) // there is only one maximum
		{
			break;
		}
	}
	// si les icones sont trop petites, et qu'on a une largeur et/ou une
	// hauteur infinie(s), essayer d'avoir au moins une taille minimale
	if(  pSlide->iIconSize < pSlide->iMinimumIconSize &&
	    (pSlide->bInfiniteWidth || pSlide->bInfiniteHeight) )
	{
		if( pSlide->bInfiniteWidth && pSlide->bInfiniteHeight )
		{
			// surface infinie: on garde le meme nb de colonnes&lignes,
			// mais on met la taille d'icone a iMinimumIconSize
			pSlide->iIconSize = pSlide->iMinimumIconSize;
		}
		else if( pSlide->bInfiniteHeight )
		{
			// hauteur infinie et largeur fixe: on calcule le nombre de colonnes
			// maxi avec pSlide->iIconSize = pSlide->iMinimumIconSize
			pSlide->iIconSize = pSlide->iMinimumIconSize;
			pSlide->iNbColumns = (w + di) / ( pSlide->iIconSize + dw + di );
			if( pSlide->iNbColumns < 1 )
			{
				pSlide->iNbColumns = 1;
				pSlide->iIconSize = w - dw; 
			}
			pSlide->iNbLines = (int) ceil ((double)pSlide->iNbIcons / pSlide->iNbColumns);
		}
		else if( pSlide->bInfiniteWidth )
		{
			// largeur infinie et hauteur fixe: on calcule le nombre de lignes
			// maxi avec pSlide->iIconSize = pSlide->iMinimumIconSize
			pSlide->iIconSize = pSlide->iMinimumIconSize;
			pSlide->iNbLines = (h + di) / ( pSlide->iIconSize + dh + di );
			if( pSlide->iNbLines < 1 )
			{
				pSlide->iNbLines = 1;
				pSlide->iIconSize = h - dh; 
			}
			pSlide->iNbColumns = (int) ceil ((double)pSlide->iNbIcons / pSlide->iNbLines);
		}
		// on calcule l'offset maximal atteignable en X
		pSlide->iMaxOffsetX = MAX(( pSlide->iIconSize + dw + di )*pSlide->iNbColumns - (w + di), 0);
		// on calcule l'offset maximal atteignable en Y
		pSlide->iMaxOffsetY = MAX(( pSlide->iIconSize + dh + di )*pSlide->iNbLines - (h + di), 0);
	}
}

static void load_data (CairoDesklet *pDesklet)
{
	CDSlideParameters *pSlide = (CDSlideParameters *) pDesklet->pRendererData;
	if (pSlide == NULL)
		return ;
	
	_compute_icons_grid (pDesklet, pSlide);
}


static void free_data (CairoDesklet *pDesklet)
{
	cairo_dock_remove_notification_func_on_container (CAIRO_CONTAINER (pDesklet), CAIRO_DOCK_ENTER_ICON, (CairoDockNotificationFunc) on_enter_icon_slide, NULL);
	
	CDSlideParameters *pSlide = (CDSlideParameters *) pDesklet->pRendererData;
	if (pSlide == NULL)
		return ;
	
	g_free (pSlide);
	pDesklet->pRendererData = NULL;
}


static void set_icon_size (CairoDesklet *pDesklet, Icon *pIcon)
{
	CDSlideParameters *pSlide = (CDSlideParameters *) pDesklet->pRendererData;
	if (pSlide == NULL)
		return ;
	
	if (pIcon == pDesklet->pIcon)
	{
		pIcon->fWidth = 0.;
		pIcon->fHeight = 0.;
	}
	else
	{
		pIcon->fWidth = pSlide->iIconSize;
		pIcon->fHeight = pSlide->iIconSize;
	}
}

static void calculate_icons (CairoDesklet *pDesklet)
{
	CDSlideParameters *pSlide = (CDSlideParameters *) pDesklet->pRendererData;
	if (pSlide == NULL)
		return ;
	
	_compute_icons_grid (pDesklet, pSlide);
	cd_debug ("pSlide->iIconSize : %d\n", pSlide->iIconSize);
	
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
			pIcon->fWidth = pSlide->iIconSize;
			pIcon->fHeight = pSlide->iIconSize;
		
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
	CDSlideParameters *pSlide = (CDSlideParameters *) pDesklet->pRendererData;
	//g_print ("%s(%x)\n", __func__, pSlide);
	if (pSlide == NULL)
		return ;
	
	double fRadius = pSlide->iRadius;
	double fLineWidth = pSlide->iLineWidth;
	// le cadre.
	cairo_set_line_width (pCairoContext, pSlide->iLineWidth);
	if (pSlide->bRoundedRadius)
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
			pSlide->iRadius,
			pSlide->iRadius);
		cairo_rel_line_to (pCairoContext,
			pDesklet->container.iWidth - fRadius - fLineWidth,
			0.);
	}
	cairo_set_source_rgba (pCairoContext, pSlide->fLineColor[0], pSlide->fLineColor[1], pSlide->fLineColor[2], pSlide->fLineColor[3]);
	cairo_stroke (pCairoContext);
	
	// les icones.
	double w = pDesklet->container.iWidth - 2 * pSlide->fMargin;
	double h = pDesklet->container.iHeight - 2 * pSlide->fMargin;
	int dh = myLabels.iLabelSize;  // taille verticale ajoutee a chaque icone.
	int dw = 2 * dh;  // taille horizontale ajoutee a chaque icone.
	if( pSlide->iMaxOffsetY == 0 )
	{
		dh = (h - pSlide->iNbLines * (pSlide->iIconSize + myLabels.iLabelSize)) / pSlide->iNbLines;  // ecart entre 2 lignes.
	}
	if( pSlide->iMaxOffsetX == 0 )
	{
		dw = (w - pSlide->iNbColumns * pSlide->iIconSize) / pSlide->iNbColumns;  // ecart entre 2 colonnes.
	}
	
	// on determine la 1ere icone a tracer : l'icone suivant l'icone pointee.
	
	double x = pSlide->fMargin + dw/2, y = pSlide->fMargin + dh/2;
	int q = 0;
	Icon *pIcon;
	GList *ic;
	GList *pVisibleIcons = NULL;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			continue;
		
		pIcon->fDrawX = x - pSlide->iCurrentOffsetX;
		pIcon->fDrawY = y - pSlide->iCurrentOffsetY;
		
		x += pSlide->iIconSize + dw;
		q ++;
		if (q == pSlide->iNbColumns)
		{
			q = 0;
			x = pSlide->fMargin + dw/2;
			y += pSlide->iIconSize + myLabels.iLabelSize + dh;
		}
		// On ne dessine que les icones qui sont visibles
		if( pIcon->fDrawX - pSlide->fMargin + dw/2 >= 0                &&
		    pIcon->fDrawY - pSlide->fMargin + myLabels.iLabelSize >= 0 &&
		    pIcon->fDrawX - pSlide->fMargin + dw/2 <= w - (pSlide->iIconSize + dw)          &&
		    pIcon->fDrawY - pSlide->fMargin + myLabels.iLabelSize <= h - (pSlide->iIconSize + myLabels.iLabelSize + dh))
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
	CDSlideParameters *pSlide = (CDSlideParameters *) pDesklet->pRendererData;
	if (pSlide == NULL)
		return ;
	
	// le cadre.
	double fRadius = (pSlide->bRoundedRadius ? pSlide->iRadius : 0.);
	double fLineWidth = pSlide->iLineWidth;
	if (fLineWidth != 0 && pSlide->fLineColor[3] != 0)
	{
		cairo_dock_draw_rounded_rectangle_opengl (pDesklet->container.iWidth - 2 * fRadius,
			pDesklet->container.iHeight,
			fRadius,
			fLineWidth,
			pSlide->fLineColor);
		glTranslatef (-pDesklet->container.iWidth/2, -pDesklet->container.iHeight/2, 0.);
	}
	
	glTranslatef (-pDesklet->container.iWidth/2, -pDesklet->container.iHeight/2, 0.);
	
	// les icones.
	double w = pDesklet->container.iWidth - 2 * pSlide->fMargin;
	double h = pDesklet->container.iHeight - 2 * pSlide->fMargin;
	int dh = myLabels.iLabelSize;  // taille verticale ajoutee a chaque icone.
	int dw = 2 * dh;  // taille horizontale ajoutee a chaque icone.
	if( pSlide->iMaxOffsetY == 0 )
	{
		// ecart entre 2 lignes si il faut repartir vertivalement les icones.
		dh = (h - pSlide->iNbLines * (pSlide->iIconSize + myLabels.iLabelSize) - 2*pSlide->fMargin - myLabels.iLabelSize) / pSlide->iNbLines;
	}
	if( pSlide->iMaxOffsetX == 0 )
	{
		// ecart entre 2 colonnes si il faut repartir horizontalement les icones.
		dw = (w - pSlide->iNbColumns * pSlide->iIconSize - 2*pSlide->fMargin) / pSlide->iNbColumns;
	}
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (1.);
	
	
	double x = pSlide->fMargin + dw/2, y = pSlide->fMargin + myLabels.iLabelSize + dh/2;
	int q = 0;
	Icon *pIcon;
	GList *ic;
	GList *pVisibleIcons = NULL;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			continue;
		
		pIcon->fDrawX = x - pSlide->iCurrentOffsetX;
		pIcon->fDrawY = y - pSlide->iCurrentOffsetY;
		
		x += pSlide->iIconSize + dw;
		q ++;
		if (q == pSlide->iNbColumns)
		{
			q = 0;
			x = pSlide->fMargin + dw/2;
			y += pSlide->iIconSize + myLabels.iLabelSize + dh;
		}
		// On ne dessine que les icones qui sont visibles
		if( pIcon->fDrawX - pSlide->fMargin - dw/2 >= 0                &&
		    pIcon->fDrawY - pSlide->fMargin - myLabels.iLabelSize - dh/2 >= 0 &&
		    pIcon->fDrawX - pSlide->fMargin - dw/2 <= w - (pSlide->iIconSize + dw/2)          &&
		    pIcon->fDrawY - pSlide->fMargin - myLabels.iLabelSize - dh/2 <= h - (pSlide->iIconSize + myLabels.iLabelSize + dh/2))
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
				
				glTranslatef (fOffsetX, pIcon->fHeight/2 + pIcon->iTextHeight / 2, 0.);
				
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



void rendering_register_slide_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render 			= (CairoDeskletRenderFunc) render;
	pRenderer->configure 		= (CairoDeskletConfigureRendererFunc) configure;
	pRenderer->load_data 		= (CairoDeskletLoadRendererDataFunc) load_data;
	pRenderer->free_data 		= (CairoDeskletFreeRendererDataFunc) free_data;
	pRenderer->calculate_icons 	= (CairoDeskletCalculateIconsFunc) calculate_icons;
	pRenderer->render_opengl 	= (CairoDeskletGLRenderFunc) render_opengl;
	
	cairo_dock_register_desklet_renderer ("Slide", pRenderer);
}
