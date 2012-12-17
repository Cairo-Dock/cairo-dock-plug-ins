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

static void render (cairo_t *pCairoContext, CairoDesklet *pDesklet);


static inline void _get_gridXY_from_index (gint nRowsX, gint index, gint* gridX, gint* gridY)
{
	*gridX = index % nRowsX;
	*gridY = index / nRowsX;
}

static void _compute_icons_grid (CairoDesklet *pDesklet, CDViewportParameters *pViewport)
{
	// nombre d'icones.
	guint nIcones = 0;  // nb icones.
	Icon *icon;
	GList *ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
			nIcones ++;
	}
	
	// taille des differents composants.
	pViewport->iIconGapX = 50;
	pViewport->iIconGapY = 10;
	pViewport->fMargin = pViewport->iIconGapX / 2;
	pViewport->fArrowGap = .05 * pDesklet->container.iHeight;
	pViewport->fArrowHeight = 14.;
	pViewport->fScrollbarWidth = 10.;
	pViewport->fScrollbarArrowGap = 4.;
	pViewport->fScrollbarIconGap = 10.;
	
	int iIconSize = 48;
	double h_min = pViewport->iIconSize + myIconsParam.iLabelSize;  // hauteur min pour caser 1 icone.
	double fx=1, fy=1;
	if (h_min > pDesklet->container.iHeight)
	{
		fy = (double) MAX (1, pDesklet->container.iHeight - myIconsParam.iLabelSize) / pViewport->iIconSize;
		pViewport->fArrowHeight *= fy;
		iIconSize *= fy;
	}
	double w_min = pViewport->fMargin + iIconSize + pViewport->fMargin + pViewport->fScrollbarIconGap + pViewport->fScrollbarWidth + pViewport->fScrollbarIconGap;  // largeur min pour caser 1 icone.
	if (w_min > pDesklet->container.iWidth)
	{
		fx = (double) pDesklet->container.iWidth / w_min;
		iIconSize *= fx;
		pViewport->iIconGapX *= fx;
		pViewport->fMargin *= fx;
		pViewport->fScrollbarWidth *= fx;
		pViewport->fScrollbarArrowGap *= fx;
		pViewport->fScrollbarIconGap *= fx;
		w_min = pDesklet->container.iWidth;
	}
	pViewport->iIconSize = iIconSize;
	
	// taille de la grille.
	pViewport->nRowsX = (pDesklet->container.iWidth - w_min) / (pViewport->iIconSize + pViewport->iIconGapX) + 1;
	pViewport->nRowsY = ceil ((double)nIcones / pViewport->nRowsX);
	pViewport->iDeltaHeight = MAX (0, (pViewport->nRowsY - 1) * (pViewport->iIconSize + myIconsParam.iLabelSize + pViewport->iIconGapY) + pViewport->iIconSize + myIconsParam.iLabelSize - pDesklet->container.iHeight);
	pViewport->fMargin = (pDesklet->container.iWidth - (pViewport->nRowsX * (pViewport->iIconSize + pViewport->iIconGapX) - pViewport->iIconSize + pViewport->fScrollbarIconGap + pViewport->fScrollbarWidth + pViewport->fScrollbarIconGap)) / 2;  // on reajuste la marge pour centrer les icones.
}

static void _compute_icons_position (CairoDesklet *pDesklet, CDViewportParameters *pViewport)
{
	if (pViewport->nRowsX == 0)  // either the grid has not yet been calculated, or there is no icon => nothing to do.
		return;
	double fScrollOffset = - pViewport->iScrollOffset;
	int iOffsetY = myIconsParam.iLabelSize +  // le texte des icones de la 1ere ligne
		fScrollOffset;
	
	Icon* icon;
	GList* ic;
	gint i, x, y;
	for (ic = pDesklet->icons, i = 0; ic != NULL; ic = ic->next, i++)
	{
		icon = ic->data;
		
		// position sur la grille.
		_get_gridXY_from_index (pViewport->nRowsX, i, &x, &y);
		
		// on en deduit la position au repos.
		icon->fX = pViewport->fMargin + (icon->fWidth + pViewport->iIconGapX) * x;
		icon->fY = iOffsetY + (icon->fHeight + myIconsParam.iLabelSize + pViewport->iIconGapY) * y;
		
		icon->fDrawX = icon->fX;
		icon->fDrawY = icon->fY;
	}
}


static void _set_scroll (CairoDesklet *pDesklet, int iOffsetY)
{
	//g_print ("%s (%d)\n", __func__, iOffsetY);
	CDViewportParameters *pData = pDesklet->pRendererData;
	g_return_if_fail (pData != NULL);
	
	pData->iScrollOffset = MAX (0, MIN (iOffsetY, pData->iDeltaHeight));
	_compute_icons_position (pDesklet, pData);
	gtk_widget_queue_draw (pDesklet->container.pWidget);
}

static gboolean _add_scroll (CairoDesklet *pDesklet, int iDeltaOffsetY)
{
	//g_print ("%s (%d)\n", __func__, iDeltaOffsetY);
	CDViewportParameters *pData = pDesklet->pRendererData;
	g_return_val_if_fail (pData != NULL, FALSE);
	
	if (iDeltaOffsetY < 0)
	{
		if (pData->iScrollOffset <= 0)
			return FALSE;
	}
	else
	{
		if (pData->iScrollOffset >= pData->iDeltaHeight)
			return FALSE;
	}
	_set_scroll (pDesklet, pData->iScrollOffset + iDeltaOffsetY);
	return TRUE;
}

static gboolean _cd_slide_on_scroll (gpointer data, Icon *pClickedIcon, CairoDesklet *pDesklet, int iDirection)
{
	CDViewportParameters *pData = pDesklet->pRendererData;
	g_return_val_if_fail (pData != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
	if (pData->iDeltaHeight == 0)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	gboolean bScrolled = _add_scroll (pDesklet, iDirection == 1 ? pData->iIconSize : - pData->iIconSize);
	return (bScrolled ? CAIRO_DOCK_INTERCEPT_NOTIFICATION : CAIRO_DOCK_LET_PASS_NOTIFICATION);
}

static gboolean _cd_slide_on_press_button (GtkWidget* pWidget, GdkEventButton* pButton, CairoDesklet *pDesklet)
{
	CDViewportParameters *pData = pDesklet->pRendererData;
	g_return_val_if_fail (pData != NULL, FALSE);
	if (pData->iDeltaHeight == 0)
		return FALSE;
	
	gboolean bIntercept = FALSE;
	if (pButton->type == GDK_BUTTON_PRESS && pButton->button == 1)
	{
		double x_arrow = pDesklet->container.iWidth - pData->fScrollbarIconGap - pData->fScrollbarWidth;
		if (pButton->x > x_arrow)  // on a clique dans la zone de scroll.
		{
			// on regarde sur quoi on clic.
			double y_arrow_top = 2., y_arrow_bottom = pDesklet->container.iHeight - 2.;  // on laisse 2 pixels de marge.
			
			if (pButton->y > y_arrow_top - pData->fScrollbarArrowGap/2 && pButton->y < y_arrow_top + pData->fArrowHeight + pData->fScrollbarArrowGap/2)  // bouton haut
			{
				_set_scroll (pDesklet, 0);
				bIntercept = TRUE;
				pDesklet->retaching = FALSE;
			}
			else if (pButton->y < y_arrow_bottom + pData->fScrollbarArrowGap/2 && pButton->y > y_arrow_bottom - pData->fArrowHeight - pData->fScrollbarArrowGap/2)  // bouton bas
			{
				_set_scroll (pDesklet, pData->iDeltaHeight);
				bIntercept = TRUE;
				pDesklet->making_transparent = FALSE;
			}
			else  // scrollbar
			{
				pData->bDraggingScrollbar = TRUE;
				pData->iClickY = pButton->y;
				pData->iClickOffset = pData->iScrollOffset;
				bIntercept = TRUE;
				pDesklet->moving = TRUE;
			}
			pDesklet->bClicked = !bIntercept;  // on fait croire au desklet qu'on n'a pas clique sur lui. c'est vraiment limite, mais on peut difficilement s'enregistrer au clic avant l'applet qui possede le desklet.
		}
	}
	else if (GDK_BUTTON_RELEASE)
	{
		//g_print ("release\n");
		pData->bDraggingScrollbar = FALSE;
		pDesklet->moving = FALSE;
	}
	return FALSE;
}

static gboolean _cd_slide_on_mouse_moved (gpointer data, CairoDesklet *pDesklet, gboolean *bStartAnimation)
{
	CDViewportParameters *pData = pDesklet->pRendererData;
	g_return_val_if_fail (pData != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
	if (pData->iDeltaHeight == 0)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pData->bDraggingScrollbar)
	{
		double y_arrow_top = 2., y_arrow_bottom = pDesklet->container.iHeight - 2.;  // on laisse 2 pixels de marge.
		double fFrameHeight = pDesklet->container.iHeight;  // hauteur du cadre.
		double fGripHeight = fFrameHeight / (fFrameHeight + pData->iDeltaHeight) * (y_arrow_bottom - y_arrow_top - 2*(pData->fArrowHeight + pData->fScrollbarArrowGap));
		// double ygrip = (double) pData->iScrollOffset / pData->iDeltaHeight * (y_arrow_bottom - y_arrow_top - 2*(pData->fArrowHeight + pData->fScrollbarArrowGap) - fGripHeight);
		
		int delta = pDesklet->container.iMouseY - pData->iClickY;
		_set_scroll (pDesklet, (pData->iClickOffset + (double)delta / (y_arrow_bottom - y_arrow_top - 2*(pData->fArrowHeight + pData->fScrollbarArrowGap) - fGripHeight) * pData->iDeltaHeight));
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

static gboolean on_enter_icon_slide (gpointer pUserData, Icon *pPointedIcon, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	gtk_widget_queue_draw (pContainer->pWidget);  // et oui, on n'a rien d'autre a faire.
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

static CDViewportParameters *configure (CairoDesklet *pDesklet, gpointer *pConfig)
{
	CDViewportParameters *pViewport = g_new0 (CDViewportParameters, 1);
	if (pConfig != NULL)
	{
		// get parameters ...
		// gap icon, horizontal scroll bar, icon size, icon gap
	}
	
	pViewport->color_scrollbar_inside[0] = .8;
	pViewport->color_scrollbar_inside[1] = .8;
	pViewport->color_scrollbar_inside[2] = .8;
	pViewport->color_scrollbar_inside[3] = .75;
	pViewport->color_scrollbar_line[0] = 1.;
	pViewport->color_scrollbar_line[1] = 1.;
	pViewport->color_scrollbar_line[2] = 1.;
	pViewport->color_scrollbar_line[3] = 1.;
	pViewport->color_grip[0] = .9;
	pViewport->color_grip[1] = .9;
	pViewport->color_grip[2] = .9;
	pViewport->color_grip[3] = 1.;
	
	cairo_dock_register_notification_on_object (CAIRO_CONTAINER (pDesklet), NOTIFICATION_SCROLL_ICON, (CairoDockNotificationFunc) _cd_slide_on_scroll, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification_on_object (CAIRO_CONTAINER (pDesklet), NOTIFICATION_MOUSE_MOVED, (CairoDockNotificationFunc) _cd_slide_on_mouse_moved, CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification_on_object (CAIRO_CONTAINER (pDesklet), NOTIFICATION_ENTER_ICON, (CairoDockNotificationFunc) on_enter_icon_slide, CAIRO_DOCK_RUN_FIRST, NULL);
	pViewport->iSidPressEvent = g_signal_connect (G_OBJECT (pDesklet->container.pWidget),
		"button-press-event",
		G_CALLBACK (_cd_slide_on_press_button),
		pDesklet);  // car les notification de clic en provenance du dock sont emises lors du relachement du bouton.
	pViewport->iSidReleaseEvent = g_signal_connect (G_OBJECT (pDesklet->container.pWidget),
		"button-release-event",
		G_CALLBACK (_cd_slide_on_press_button),
		pDesklet);
	
	return pViewport;
}


static void free_data (CairoDesklet *pDesklet)
{
	CDViewportParameters *pViewport = (CDViewportParameters *) pDesklet->pRendererData;
	if (pViewport == NULL)
		return ;
	
	cairo_dock_remove_notification_func_on_object (CAIRO_CONTAINER (pDesklet), NOTIFICATION_SCROLL_ICON, (CairoDockNotificationFunc) _cd_slide_on_scroll, NULL);
	cairo_dock_remove_notification_func_on_object (CAIRO_CONTAINER (pDesklet), NOTIFICATION_MOUSE_MOVED, (CairoDockNotificationFunc) _cd_slide_on_mouse_moved, NULL);
	cairo_dock_remove_notification_func_on_object (CAIRO_CONTAINER (pDesklet), NOTIFICATION_ENTER_ICON, (CairoDockNotificationFunc) on_enter_icon_slide, NULL);
	g_signal_handler_disconnect (pDesklet->container.pWidget, pViewport->iSidPressEvent);
	g_signal_handler_disconnect (pDesklet->container.pWidget, pViewport->iSidReleaseEvent);
	
	g_free (pViewport);
	pDesklet->pRendererData = NULL;
}

/* Not used
static void set_icon_size (CairoDesklet *pDesklet, Icon *pIcon)
{
	CDViewportParameters *pViewport = (CDViewportParameters *) pDesklet->pRendererData;
	if (pViewport == NULL)
		return ;
	
	if (pIcon == pDesklet->pIcon)
	{
		pIcon->fWidth = 0.;
		pIcon->fHeight = 0.;
	}
	else
	{
		pIcon->fWidth = pViewport->iIconSize;
		pIcon->fHeight = pViewport->iIconSize;
	}
}
*/

static void calculate_icons (CairoDesklet *pDesklet)
{
	CDViewportParameters *pViewport = (CDViewportParameters *) pDesklet->pRendererData;
	if (pViewport == NULL)
		return ;
	
	//\____________________ On calcule la grille : repartition et taille des icones, scrollbar.
	_compute_icons_grid (pDesklet, pViewport);
	
	//\____________________ On renseigne chaque icone apres (elles sont toutes identiques).
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
			pIcon->fWidth = pViewport->iIconSize;
			pIcon->fHeight = pViewport->iIconSize;
			cairo_dock_icon_set_allocated_size (pIcon, pIcon->fWidth, pIcon->fHeight);
			
			pIcon->fScale = 1.;
			pIcon->fAlpha = 1.;
			pIcon->fWidthFactor = 1.;
			pIcon->fHeightFactor = 1.;
			pIcon->fGlideScale = 1.;
		}
	}
	
	//\____________________ tant qu'on y est, on calcule leur position.
	_compute_icons_position (pDesklet, pViewport);
}


static void render (cairo_t *pCairoContext, CairoDesklet *pDesklet)
{
	CDViewportParameters *pData = (CDViewportParameters *) pDesklet->pRendererData;
	//g_print ("%s(%x)\n", __func__, pViewport);
	if (pData == NULL)
		return ;
	
	//\____________________ On dessine les barres de defilement.
	if (pData != NULL && pData->iDeltaHeight != 0)
	{
		cairo_save (pCairoContext);
		cairo_set_line_width (pCairoContext, 2.);
		
		double x_arrow = pDesklet->container.iWidth - pData->fScrollbarIconGap - pData->fScrollbarWidth/2;  // pointe de la fleche.
		double y_arrow_top, y_arrow_bottom;
		y_arrow_top = 2.;
		y_arrow_bottom = pDesklet->container.iHeight - 2.;
		
		if (pData->iScrollOffset != 0)  // fleche vers le haut.
		{
			cairo_move_to (pCairoContext, x_arrow, y_arrow_top);
			cairo_rel_line_to (pCairoContext, pData->fScrollbarWidth/2, pData->fArrowHeight);
			cairo_rel_line_to (pCairoContext, -pData->fScrollbarWidth, 0.);
			cairo_close_path (pCairoContext);
			
			cairo_set_source_rgba (pCairoContext, pData->color_scrollbar_inside[0], pData->color_scrollbar_inside[1], pData->color_scrollbar_inside[2], pData->color_scrollbar_inside[3]);
			cairo_fill_preserve (pCairoContext);
			
			cairo_set_source_rgba (pCairoContext, pData->color_scrollbar_line[0], pData->color_scrollbar_line[1], pData->color_scrollbar_line[2], pData->color_scrollbar_line[3]);
			cairo_stroke (pCairoContext);
		}
		if (pData->iScrollOffset != pData->iDeltaHeight)  // fleche vers le bas.
		{
			cairo_move_to (pCairoContext, x_arrow, y_arrow_bottom);
			cairo_rel_line_to (pCairoContext, pData->fScrollbarWidth/2, - pData->fArrowHeight);
			cairo_rel_line_to (pCairoContext, -pData->fScrollbarWidth, 0.);
			cairo_close_path (pCairoContext);
			
			cairo_set_source_rgba (pCairoContext, pData->color_scrollbar_inside[0], pData->color_scrollbar_inside[1], pData->color_scrollbar_inside[2], pData->color_scrollbar_inside[3]);
			cairo_fill_preserve (pCairoContext);
			
			cairo_set_source_rgba (pCairoContext, pData->color_scrollbar_line[0], pData->color_scrollbar_line[1], pData->color_scrollbar_line[2], pData->color_scrollbar_line[3]);
			cairo_stroke (pCairoContext);
		}
		// scrollbar outline
		cairo_move_to (pCairoContext, x_arrow - pData->fScrollbarWidth/2, y_arrow_top + pData->fArrowHeight + pData->fScrollbarArrowGap);
		cairo_rel_line_to (pCairoContext, pData->fScrollbarWidth, 0.);
		cairo_rel_line_to (pCairoContext, 0., y_arrow_bottom - y_arrow_top - 2*(pData->fArrowHeight + pData->fScrollbarArrowGap));
		cairo_rel_line_to (pCairoContext, -pData->fScrollbarWidth, 0.);
		cairo_close_path (pCairoContext);
		
		cairo_set_source_rgba (pCairoContext, pData->color_scrollbar_inside[0], pData->color_scrollbar_inside[1], pData->color_scrollbar_inside[2], pData->color_scrollbar_inside[3]);
		cairo_fill_preserve (pCairoContext);
		
		cairo_set_source_rgba (pCairoContext, pData->color_scrollbar_line[0], pData->color_scrollbar_line[1], pData->color_scrollbar_line[2], pData->color_scrollbar_line[3]);
		cairo_stroke (pCairoContext);
		// grip
		double fFrameHeight = pDesklet->container.iHeight ;  // hauteur du cadre.
		double fGripHeight = fFrameHeight / (fFrameHeight + pData->iDeltaHeight) * (y_arrow_bottom - y_arrow_top - 2*(pData->fArrowHeight + pData->fScrollbarArrowGap));
		double ygrip = (double) pData->iScrollOffset / pData->iDeltaHeight * (y_arrow_bottom - y_arrow_top - 2*(pData->fArrowHeight + pData->fScrollbarArrowGap) - fGripHeight);
		cairo_set_source_rgba (pCairoContext, pData->color_grip[0], pData->color_grip[1], pData->color_grip[2], pData->color_grip[3]);
		cairo_move_to (pCairoContext, x_arrow - pData->fScrollbarWidth/2 + 1, y_arrow_top + pData->fArrowHeight + pData->fScrollbarArrowGap + ygrip);
		cairo_rel_line_to (pCairoContext, pData->fScrollbarWidth - 2, 0.);
		cairo_rel_line_to (pCairoContext, 0., fGripHeight);
		cairo_rel_line_to (pCairoContext, - (pData->fScrollbarWidth - 2), 0.);
		cairo_fill (pCairoContext);
		
		cairo_restore (pCairoContext);
	}
	
	//\____________________ On dessine les icones.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDesklet->icons);
	if (pFirstDrawnElement == NULL)
		return;
	Icon *pIcon;
	GList *ic = pFirstDrawnElement;
	do
	{
		pIcon = ic->data;
		if (pIcon->image.pSurface != NULL && ! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			cairo_save (pCairoContext);
			
			cairo_dock_render_one_icon_in_desklet (pIcon, CAIRO_CONTAINER (pDesklet), pCairoContext, FALSE);
			
			cairo_restore (pCairoContext);
			
			if (pIcon->label.pSurface != NULL)
			{
				cairo_save (pCairoContext);
				cairo_translate (pCairoContext, pIcon->fDrawX, pIcon->fDrawY);
				
				double fOffsetX = 0., fAlpha;
				if (pIcon->bPointed)
				{
					fAlpha = 1.;
					if (pIcon->fDrawX + pIcon->fWidth/2 + pIcon->label.iWidth/2 > pDesklet->container.iWidth)
						fOffsetX = pDesklet->container.iWidth - (pIcon->fDrawX + pIcon->fWidth/2 + pIcon->label.iWidth/2);
					if (pIcon->fDrawX + pIcon->fWidth/2 - pIcon->label.iWidth/2 < 0)
						fOffsetX = pIcon->label.iWidth/2 - (pIcon->fDrawX + pIcon->fWidth/2);
					cairo_set_source_surface (pCairoContext,
						pIcon->label.pSurface,
						fOffsetX + pIcon->fWidth/2 - pIcon->label.iWidth/2,
						-myIconsParam.iLabelSize);
					cairo_paint_with_alpha (pCairoContext, fAlpha);
				}
				else
				{
					fAlpha = .6;
					if (pIcon->label.iWidth > pIcon->fWidth + 2 * myIconsParam.iLabelSize)
					{
						fOffsetX = - myIconsParam.iLabelSize;
						cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (fOffsetX,
							0.,
							fOffsetX + pIcon->fWidth + 2*myIconsParam.iLabelSize,
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
							pIcon->label.pSurface,
							fOffsetX,
							-myIconsParam.iLabelSize);
						cairo_mask (pCairoContext, pGradationPattern);
						cairo_pattern_destroy (pGradationPattern);
					}
					else
					{
						fOffsetX = pIcon->fWidth/2 - pIcon->label.iWidth/2;
						cairo_set_source_surface (pCairoContext,
							pIcon->label.pSurface,
							fOffsetX,
							-myIconsParam.iLabelSize);
						cairo_paint_with_alpha (pCairoContext, fAlpha);
					}
				}
				
				cairo_restore (pCairoContext);
			}
		}
		ic = cairo_dock_get_next_element (ic, pDesklet->icons);
	}
	while (ic != pFirstDrawnElement);
	
	// la scrollbar
	
}


static void render_opengl (CairoDesklet *pDesklet)
{
	static CairoDockGLPath *pScrollPath = NULL;
	
	CDViewportParameters *pData = (CDViewportParameters *) pDesklet->pRendererData;
	//g_print ("%s(%x)\n", __func__, pViewport);
	if (pData == NULL)
		return ;
	
	glPushMatrix ();
	glTranslatef (- pDesklet->container.iWidth/2, - pDesklet->container.iHeight / 2, 0.);
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_disable_texture ();
	
	//\____________________ On dessine les barres de defilement.
	if (pData != NULL && pData->iDeltaHeight != 0)
	{
		glPushMatrix ();
		if (pScrollPath == NULL)
			pScrollPath = cairo_dock_new_gl_path (4, 0., 0., 0, 0);  // des triangles ou des rectangles => 4 points max.
		glLineWidth (2.);
		
		double x_arrow = pDesklet->container.iWidth - pData->fScrollbarIconGap - pData->fScrollbarWidth/2;  // pointe de la fleche.
		double y_arrow_top, y_arrow_bottom;
		y_arrow_bottom = 2.;
		y_arrow_top = pDesklet->container.iHeight - 2.;
		
		if (pData->iScrollOffset != 0)  // fleche vers le haut.
		{
			cairo_dock_gl_path_move_to (pScrollPath, x_arrow, y_arrow_top);
			cairo_dock_gl_path_rel_line_to (pScrollPath, pData->fScrollbarWidth/2, -pData->fArrowHeight);
			cairo_dock_gl_path_rel_line_to (pScrollPath, -pData->fScrollbarWidth, 0.);
			
			glColor4f (pData->color_scrollbar_inside[0], pData->color_scrollbar_inside[1], pData->color_scrollbar_inside[2], pData->color_scrollbar_inside[3]);
			cairo_dock_fill_gl_path (pScrollPath, 0);
			
			glColor4f (pData->color_scrollbar_line[0], pData->color_scrollbar_line[1], pData->color_scrollbar_line[2], pData->color_scrollbar_line[3]);
			cairo_dock_stroke_gl_path (pScrollPath, TRUE);  // TRUE <=> close
		}
		if (pData->iScrollOffset != pData->iDeltaHeight)  // fleche vers le bas.
		{
			cairo_dock_gl_path_move_to (pScrollPath, x_arrow, y_arrow_bottom);
			cairo_dock_gl_path_rel_line_to (pScrollPath, pData->fScrollbarWidth/2, pData->fArrowHeight);
			cairo_dock_gl_path_rel_line_to (pScrollPath, -pData->fScrollbarWidth, 0.);
			
			glColor4f (pData->color_scrollbar_inside[0], pData->color_scrollbar_inside[1], pData->color_scrollbar_inside[2], pData->color_scrollbar_inside[3]);
			cairo_dock_fill_gl_path (pScrollPath, 0);
			
			glColor4f (pData->color_scrollbar_line[0], pData->color_scrollbar_line[1], pData->color_scrollbar_line[2], pData->color_scrollbar_line[3]);
			cairo_dock_stroke_gl_path (pScrollPath, TRUE);  // TRUE <=> close
		}
		
		// scrollbar outline
		cairo_dock_gl_path_move_to (pScrollPath, x_arrow - pData->fScrollbarWidth/2, y_arrow_bottom + pData->fArrowHeight + pData->fScrollbarArrowGap);
		cairo_dock_gl_path_rel_line_to (pScrollPath, pData->fScrollbarWidth, 0.);
		cairo_dock_gl_path_rel_line_to (pScrollPath, 0., y_arrow_top - y_arrow_bottom - 2*(pData->fArrowHeight + pData->fScrollbarArrowGap));
		cairo_dock_gl_path_rel_line_to (pScrollPath, -pData->fScrollbarWidth, 0.);
		
		glColor4f (pData->color_scrollbar_inside[0], pData->color_scrollbar_inside[1], pData->color_scrollbar_inside[2], pData->color_scrollbar_inside[3]);
		cairo_dock_fill_gl_path (pScrollPath, 0);
		
		glColor4f (pData->color_scrollbar_line[0], pData->color_scrollbar_line[1], pData->color_scrollbar_line[2], pData->color_scrollbar_line[3]);
		cairo_dock_stroke_gl_path (pScrollPath, TRUE);
		
		// grip
		double fFrameHeight = pDesklet->container.iHeight;  // hauteur du cadre.
		double fGripHeight = fFrameHeight / (fFrameHeight + pData->iDeltaHeight) * (y_arrow_top - y_arrow_bottom - 2*(pData->fArrowHeight + pData->fScrollbarArrowGap));
		double ygrip = (double) pData->iScrollOffset / pData->iDeltaHeight * (y_arrow_top - y_arrow_bottom - 2*(pData->fArrowHeight + pData->fScrollbarArrowGap) - fGripHeight);
		glColor4f (pData->color_grip[0], pData->color_grip[1], pData->color_grip[2], pData->color_grip[3]);
		cairo_dock_gl_path_move_to (pScrollPath, x_arrow - pData->fScrollbarWidth/2, y_arrow_top - (pData->fArrowHeight + pData->fScrollbarArrowGap) - ygrip);
		cairo_dock_gl_path_rel_line_to (pScrollPath, pData->fScrollbarWidth, 0.);
		cairo_dock_gl_path_rel_line_to (pScrollPath, 0., - fGripHeight);
		cairo_dock_gl_path_rel_line_to (pScrollPath, -pData->fScrollbarWidth, 0.);
		cairo_dock_fill_gl_path (pScrollPath, 0);
		
		glPopMatrix ();
	}
	
	//\____________________ On dessine les icones.
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (1.);
	
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDesklet->icons);
	if (pFirstDrawnElement == NULL)
		return;
	Icon *pIcon;
	GList *ic = pFirstDrawnElement;
	do
	{
		pIcon = ic->data;
		
		if (pIcon->image.iTexture != 0 && ! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			glPushMatrix ();
			
			glTranslatef (pIcon->fDrawX + pIcon->fWidth/2,
				pDesklet->container.iHeight - pIcon->fDrawY - pIcon->fHeight/2,
				0.);
			//g_print (" %d) %d;%d %dx%d\n", pIcon->iIconTexture, (int)(pIcon->fDrawX + pIcon->fWidth/2), (int)(pDesklet->container.iHeight - pIcon->fDrawY - pIcon->fHeight/2), (int)(pIcon->fWidth/2), (int)(pIcon->fHeight/2));
			_cairo_dock_enable_texture ();  // cairo_dock_draw_icon_overlays_opengl() disable textures
			_cairo_dock_apply_texture_at_size (pIcon->image.iTexture, pIcon->fWidth, pIcon->fHeight);
			
			/// generer une notification ...
			if (pIcon->label.iTexture != 0)
			{
				glPushMatrix ();
				
				double dx = .5 * (pIcon->label.iWidth & 1);  // on decale la texture pour la coller sur la grille des coordonnees entieres.
				double dy = .5 * (pIcon->label.iHeight & 1);
				double u0 = 0., u1 = 1.;
				double fOffsetX = 0.;
				if (pIcon->bPointed)
				{
					_cairo_dock_set_alpha (1.);
					if (pIcon->fDrawX + pIcon->fWidth/2 + pIcon->label.iWidth/2 > pDesklet->container.iWidth)
						fOffsetX = pDesklet->container.iWidth - (pIcon->fDrawX + pIcon->fWidth/2 + pIcon->label.iWidth/2);
					if (pIcon->fDrawX + pIcon->fWidth/2 - pIcon->label.iWidth/2 < 0)
						fOffsetX = pIcon->label.iWidth/2 - (pIcon->fDrawX + pIcon->fWidth/2);
				}
				else
				{
					_cairo_dock_set_alpha (.6);
					if (pIcon->label.iWidth > pIcon->fWidth + 2 * myIconsParam.iLabelSize)
					{
						fOffsetX = 0.;
						u1 = (double) (pIcon->fWidth + 2 * myIconsParam.iLabelSize) / pIcon->label.iWidth;
					}
				}
				
				glTranslatef (ceil (fOffsetX) + dx, ceil (pIcon->fHeight/2 + pIcon->label.iHeight / 2) + dy, 0.);
				
				glBindTexture (GL_TEXTURE_2D, pIcon->label.iTexture);
				_cairo_dock_apply_current_texture_portion_at_size_with_offset (u0, 0.,
					u1 - u0, 1.,
					pIcon->label.iWidth * (u1 - u0), pIcon->label.iHeight,
					0., 0.);
				_cairo_dock_set_alpha (1.);
				
				glPopMatrix ();
			}
			
			/**if (pIcon->iQuickInfoTexture != 0)
			{
				glTranslatef (0., (- pIcon->fHeight + pIcon->iQuickInfoHeight)/2, 0.);
				
				_cairo_dock_apply_texture_at_size (pIcon->iQuickInfoTexture,
					pIcon->iQuickInfoWidth,
					pIcon->iQuickInfoHeight);
			}*/
			cairo_dock_draw_icon_overlays_opengl (pIcon, pDesklet->container.fRatio);
			
			glPopMatrix ();
		}

		ic = cairo_dock_get_next_element (ic, pDesklet->icons);
		
	} while (ic != pFirstDrawnElement);
	
	glPopMatrix ();
	_cairo_dock_disable_texture ();
}



void rendering_register_viewport_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render 			= (CairoDeskletRenderFunc) render;
	pRenderer->configure 		= (CairoDeskletConfigureRendererFunc) configure;
	pRenderer->load_data 		= (CairoDeskletLoadRendererDataFunc) NULL;  // nothing to load.
	pRenderer->free_data 		= (CairoDeskletFreeRendererDataFunc) free_data;
	pRenderer->calculate_icons 	= (CairoDeskletCalculateIconsFunc) calculate_icons;
	pRenderer->render_opengl 	= (CairoDeskletGLRenderFunc) render_opengl;
	
	cairo_dock_register_desklet_renderer ("Viewport", pRenderer);
}
