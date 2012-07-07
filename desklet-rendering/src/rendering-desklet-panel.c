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

#include "rendering-desklet-panel.h"

#define GAP_X_MIN 10
#define GAP_Y_MIN 8

static gboolean on_enter_icon (gpointer pUserData, Icon *pPointedIcon, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	gtk_widget_queue_draw (pContainer->pWidget);  // et oui, on n'a rien d'autre a faire.
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

static CDPanelParameters *configure (CairoDesklet *pDesklet, gpointer *pConfig)  // gint, gboolean, gdouble[4]
{
	CDPanelParameters *pPanel = g_new0 (CDPanelParameters, 1);
	if (pConfig != NULL)
	{
		pPanel->iNbLinesForced = GPOINTER_TO_INT (pConfig[0]);
		pPanel->bHorizontalPackaging = GPOINTER_TO_INT (pConfig[1]);
		if (pConfig[2] != NULL)
			memcpy (pPanel->fBgColor, pConfig[2], 4 * sizeof (gdouble));
		if (pPanel->fBgColor[3] != 0)
		{
			pPanel->iLineWidth = 2;
			pPanel->iRadius = 8;
		}
	}
	
	cairo_dock_register_notification_on_object (CAIRO_CONTAINER (pDesklet), NOTIFICATION_ENTER_ICON, (CairoDockNotificationFunc) on_enter_icon, CAIRO_DOCK_RUN_FIRST, NULL);  // CAIRO_CONTAINER (pDesklet)
	
	return pPanel;
}


static inline void _compute_icons_grid (CairoDesklet *pDesklet, CDPanelParameters *pPanel)
{
	pPanel->fMargin = .5 * pPanel->iLineWidth + (1. - sqrt (2) / 2) * pPanel->iRadius;
	
	double w = pDesklet->container.iWidth - 2 * pPanel->fMargin;
	double h = pDesklet->container.iHeight - 2 * pPanel->fMargin;
	pPanel->iMainIconSize = MIN (w, h) / 3;
	cd_debug ("  desklet: %dx%d", (int)w, (int)h);
	
	int iNbIcons = 0;
	Icon *pIcon;
	GList *ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			iNbIcons ++;
	}
	pPanel->iNbIcons = iNbIcons;
	
	int dh = myIconsParam.iLabelSize;  // taille verticale ajoutee a chaque icone.
	int dw = 0;  // taille horizontale ajoutee a chaque icone.
	int dx = GAP_X_MIN;  // ecart entre 2 colonnes.
	int dy = GAP_Y_MIN;  // ecart entre 2 lignes.
	h -= pPanel->iMainIconSize;
	if (pPanel->iNbLinesForced == 0)
	{
		int p, q;  // nombre de lignes et colonnes.
		int iSize;
		pPanel->iIconSize = 0, pPanel->iNbLines = 0, pPanel->iNbColumns = 0;
		//g_print ("%d icones sur %dx%d (%d)\n", pPanel->iNbIcons, (int)w, (int)h, myIconsParam.iLabelSize);
		for (p = 1; p <= pPanel->iNbIcons; p ++)
		{
			q = (int) ceil ((double)pPanel->iNbIcons / p);
			iSize = MIN ((h - p * dy) / p - dh, ((w - (q - 1) * dx) / q - dw) / 2);
			cd_debug ("  %dx%d -> %d", p, q, iSize);
			if (iSize > pPanel->iIconSize)
			{
				pPanel->iIconSize = iSize;
				pPanel->iNbLines = p;
				pPanel->iNbColumns = q;
			}
		}
	}
	else
	{
		int p, q;  // nombre de lignes et colonnes.
		pPanel->iNbLines = p = pPanel->iNbLinesForced;
		pPanel->iNbColumns = q = ceil ((double)iNbIcons / pPanel->iNbLinesForced);
		pPanel->iIconSize = MIN ((h - p * dy) / p - dh, ((w - (q - 1) * dx) / q - dw) / 2);
	}
	pPanel->iIconSize = MIN (pPanel->iIconSize, pPanel->iMainIconSize);
	cd_debug (" panel desklet: %dx%d, %d", pPanel->iNbLines, pPanel->iNbColumns, pPanel->iIconSize);
	
	if ((h - pPanel->iNbLines * (pPanel->iIconSize + dh)) / pPanel->iNbLines > dy)
	{
		pPanel->iMainIconSize += h - pPanel->iNbLines * (pPanel->iIconSize + dh + dy);
	}
}

static void free_data (CairoDesklet *pDesklet)
{
	cairo_dock_remove_notification_func_on_object (CAIRO_CONTAINER (pDesklet), NOTIFICATION_ENTER_ICON, (CairoDockNotificationFunc) on_enter_icon, NULL);
	
	CDPanelParameters *pPanel = (CDPanelParameters *) pDesklet->pRendererData;
	if (pPanel == NULL)
		return ;
	
	g_free (pPanel);
	pDesklet->pRendererData = NULL;
}


static void set_icon_size (CairoDesklet *pDesklet, Icon *pIcon)
{
	CDPanelParameters *pPanel = (CDPanelParameters *) pDesklet->pRendererData;
	if (pPanel == NULL)
		return ;
	
	if (pIcon == pDesklet->pIcon)
	{
		pIcon->fWidth = pPanel->iMainIconSize;
		pIcon->fHeight = pPanel->iMainIconSize;
	}
	else
	{
		pIcon->fWidth = pPanel->iIconSize;
		pIcon->fHeight = pPanel->iIconSize;
	}
}

static void calculate_icons (CairoDesklet *pDesklet)
{
	CDPanelParameters *pPanel = (CDPanelParameters *) pDesklet->pRendererData;
	if (pPanel == NULL)
		return ;
	
	_compute_icons_grid (pDesklet, pPanel);
	cd_debug ("pPanel->iIconSize : %d\n", pPanel->iIconSize);
	
	Icon *pIcon = pDesklet->pIcon;
	if (pIcon != NULL)
	{
		pIcon->fWidth = pPanel->iMainIconSize;
		pIcon->fHeight = pPanel->iMainIconSize;
		pIcon->iImageWidth = pIcon->fWidth;
		pIcon->iImageHeight = pIcon->fHeight;
		
		pIcon->fScale = 1.;
		pIcon->fAlpha = 1.;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		pIcon->fGlideScale = 1.;
		
		pIcon->fDrawX = pPanel->fMargin;
		pIcon->fDrawY = pPanel->fMargin;
	}
	
	// les icones.
	double w = pDesklet->container.iWidth - 2 * pPanel->fMargin;
	double h = pDesklet->container.iHeight - 2 * pPanel->fMargin - pPanel->iMainIconSize;
	int dh = (h - pPanel->iNbLines * (pPanel->iIconSize + myIconsParam.iLabelSize)) / pPanel->iNbLines;  // ecart entre 2 lignes.
	int dw = (w - pPanel->iNbColumns * 2*pPanel->iIconSize) / pPanel->iNbColumns;  // ecart entre 2 colonnes.
	
	double x = pPanel->fMargin + dw/2;  // current icon position.
	double y = pPanel->fMargin + pPanel->iMainIconSize + dh/2 + myIconsParam.iLabelSize;
	int q = 0;  // num line/column
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
			pIcon->fWidth = pPanel->iIconSize;
			pIcon->fHeight = pPanel->iIconSize;
			pIcon->iImageWidth = pIcon->fWidth;
			pIcon->iImageHeight = pIcon->fHeight;
		
			pIcon->fScale = 1.;
			pIcon->fAlpha = 1.;
			pIcon->fWidthFactor = 1.;
			pIcon->fHeightFactor = 1.;
			pIcon->fGlideScale = 1.;
			
			pIcon->fDrawX = x;
			pIcon->fDrawY = y;
			
			if (pPanel->bHorizontalPackaging)
			{
				x += 2*pPanel->iIconSize + dw;
				q ++;
				if (q == pPanel->iNbColumns)
				{
					q = 0;
					x = pPanel->fMargin + dw/2;
					y += pPanel->iIconSize + myIconsParam.iLabelSize + dh;
				}
			}
			else
			{
				y += pPanel->iIconSize + myIconsParam.iLabelSize + dh;
				q ++;
				if (q == pPanel->iNbLines)
				{
					q = 0;
					x += 2*pPanel->iIconSize + dw;
					y = pPanel->fMargin + pPanel->iMainIconSize + dh/2 + myIconsParam.iLabelSize;
				}
			}
		}
	}
}


static void render (cairo_t *pCairoContext, CairoDesklet *pDesklet)
{
	CDPanelParameters *pPanel = (CDPanelParameters *) pDesklet->pRendererData;
	//g_print ("%s(%x)\n", __func__, pPanel);
	if (pPanel == NULL)
		return ;
	
	double fRadius = pPanel->iRadius;
	double fLineWidth = pPanel->iLineWidth;
	double fOffsetX = fRadius + fLineWidth/2;
	double fOffsetY = fLineWidth/2;
	double fFrameWidth = pDesklet->container.iWidth - 2 * fRadius - fLineWidth;
	double fFrameHeight = pDesklet->container.iHeight - fLineWidth;
	// le cadre.
	cairo_set_line_width (pCairoContext, pPanel->iLineWidth);
	
	cairo_move_to (pCairoContext, fOffsetX, fOffsetY);
	
	cairo_rel_curve_to (pCairoContext,
		fFrameWidth/2, 0,
		fFrameWidth/2, pPanel->iMainIconSize,
		fFrameWidth, pPanel->iMainIconSize);
	
	//\_________________ Coin haut droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		fRadius, 0,
		fRadius, fRadius);
	cairo_rel_line_to (pCairoContext, 0, fFrameHeight - fRadius * 2 - pPanel->iMainIconSize);
	//\_________________ Coin bas droit.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, fRadius,
		-fRadius, fRadius);

	cairo_rel_line_to (pCairoContext, - fFrameWidth, 0);
	//\_________________ Coin bas gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		-fRadius, 0,
		-fRadius, - fRadius);
	cairo_rel_line_to (pCairoContext, 0, - (fFrameHeight - fRadius * 2));
	//\_________________ Coin haut gauche.
	cairo_rel_curve_to (pCairoContext,
		0, 0,
		0, -fRadius,
		fRadius, -fRadius);
	
	cairo_set_source_rgba (pCairoContext, pPanel->fBgColor[0], pPanel->fBgColor[1], pPanel->fBgColor[2], 1.);
	cairo_stroke_preserve (pCairoContext);
	
	cairo_set_source_rgba (pCairoContext, pPanel->fBgColor[0], pPanel->fBgColor[1], pPanel->fBgColor[2], pPanel->fBgColor[3]);
	cairo_fill (pCairoContext);
	
	// les icones.
	Icon *pIcon;
	GList *ic;
	
	pIcon = pDesklet->pIcon;
	if (pIcon && pIcon->pIconBuffer != NULL)
	{
		cairo_save (pCairoContext);
		
		cairo_translate (pCairoContext, pIcon->fDrawX, pIcon->fDrawY);
		cairo_set_source_surface (pCairoContext, pIcon->pIconBuffer, 0.0, 0.0);
		if (pIcon->fAlpha == 1)
			cairo_paint (pCairoContext);
		else
			cairo_paint_with_alpha (pCairoContext, pIcon->fAlpha);
			
		/**if (pIcon->pQuickInfoBuffer != NULL)
		{
			cairo_translate (pCairoContext,
				pIcon->fDrawX + pIcon->fWidth,
				pIcon->fDrawY + pIcon->fHeight/2 - pIcon->iQuickInfoHeight/2);
			
			cairo_set_source_surface (pCairoContext,
				pIcon->pQuickInfoBuffer,
				0,
				0);
			cairo_paint (pCairoContext);
		}*/
		cairo_dock_draw_icon_overlays_cairo (pIcon, pDesklet->container.fRatio, pCairoContext);
		cairo_restore (pCairoContext);
	}
	
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDesklet->icons);
	if (pFirstDrawnElement == NULL)
		return;
	ic = pFirstDrawnElement;
	do
	{
		pIcon = ic->data;
		if (pIcon->pIconBuffer != NULL && ! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			cairo_save (pCairoContext);
			
			cairo_translate (pCairoContext, pIcon->fDrawX, pIcon->fDrawY);
			cairo_set_source_surface (pCairoContext, pIcon->pIconBuffer, 0.0, 0.0);
			if (pIcon->fAlpha == 1)
				cairo_paint (pCairoContext);
			else
				cairo_paint_with_alpha (pCairoContext, pIcon->fAlpha);
			
			cairo_restore (pCairoContext);
			
			if (pIcon->label.pSurface != NULL)
			{
				cairo_save (pCairoContext);
				cairo_translate (pCairoContext, pIcon->fDrawX, pIcon->fDrawY);
				
				double fOffsetX = 0., fAlpha;
				if (pIcon->bPointed)
				{
					fAlpha = 1.;
					/**if (pIcon->fDrawX + pIcon->fWidth/2 + pIcon->label.iWidth/2 > pDesklet->container.iWidth)
						fOffsetX = pDesklet->container.iWidth - (pIcon->fDrawX + pIcon->fWidth/2 + pIcon->label.iWidth/2);
					if (pIcon->fDrawX + pIcon->fWidth/2 - pIcon->label.iWidth/2 < 0)
						fOffsetX = pIcon->label.iWidth/2 - (pIcon->fDrawX + pIcon->fWidth/2);
					cairo_set_source_surface (pCairoContext,
						pIcon->label.pSurface,
						fOffsetX + pIcon->fWidth/2 - pIcon->label.iWidth/2,
						-myIconsParam.iLabelSize);*/
					cairo_set_source_surface (pCairoContext,
						pIcon->label.pSurface,
						0.,
						-myIconsParam.iLabelSize);
					cairo_paint_with_alpha (pCairoContext, fAlpha);
				}
				else
				{
					fAlpha = .6;
					if (pIcon->label.iWidth > 2*pIcon->fWidth + 0 * myIconsParam.iLabelSize)
					{
						///fOffsetX = - myIconsParam.iLabelSize;
						cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (fOffsetX,
							0.,
							fOffsetX + 2*pIcon->fWidth + 0*myIconsParam.iLabelSize,
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
						///fOffsetX = pIcon->fWidth/2 - pIcon->label.iWidth/2;
						cairo_set_source_surface (pCairoContext,
							pIcon->label.pSurface,
							fOffsetX,
							-myIconsParam.iLabelSize);
						cairo_paint_with_alpha (pCairoContext, fAlpha);
					}
				}
				
				cairo_restore (pCairoContext);
			}
			
			/**if (pIcon->pQuickInfoBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				cairo_translate (pCairoContext,
					pIcon->fDrawX + pIcon->fWidth,
					pIcon->fDrawY + pIcon->fHeight/2 - pIcon->iQuickInfoHeight/2);
				
				cairo_set_source_surface (pCairoContext,
					pIcon->pQuickInfoBuffer,
					0,
					0);
				cairo_paint (pCairoContext);
				
				cairo_restore (pCairoContext);
			}*/
			cairo_dock_draw_icon_overlays_cairo (pIcon, pDesklet->container.fRatio, pCairoContext);
		}
		ic = cairo_dock_get_next_element (ic, pDesklet->icons);
	}
	while (ic != pFirstDrawnElement);
}


static void render_opengl (CairoDesklet *pDesklet)
{
	static CairoDockGLPath *pPath = NULL;
	static const int iNbPoints1Round = 5;
	static const int iNbPointsCurve = 15;
	CDPanelParameters *pPanel = (CDPanelParameters *) pDesklet->pRendererData;
	if (pPanel == NULL)
		return ;
	
	// draw frame
	double fRadius = pPanel->iRadius;
	double fLineWidth = pPanel->iLineWidth;
	double fOffsetX = fRadius + fLineWidth/2;
	double fOffsetY = fLineWidth/2;
	double fFrameWidth = pDesklet->container.iWidth - 2 * fRadius - fLineWidth;
	double fFrameHeight = pDesklet->container.iHeight - 2 * fRadius - fLineWidth;
	double w = fFrameWidth / 2;
	double h = fFrameHeight / 2;
	double r = fRadius;
	if (fLineWidth != 0 && pPanel->fBgColor[3] != 0)
	{
		if (pPath == NULL)
		{
			pPath = cairo_dock_new_gl_path (4*iNbPoints1Round+iNbPointsCurve+1, -w, -h, pDesklet->container.iWidth, pDesklet->container.iHeight);  // on commence en bas a gauche pour avoir une bonne triangulation du polygone.  // 4 corners + 1 curve
		}
		else
		{
			cairo_dock_gl_path_move_to (pPath, -w, -h-r);
			cairo_dock_gl_path_set_extent (pPath, pDesklet->container.iWidth, pDesklet->container.iHeight);
		}
		_cairo_dock_disable_texture ();
		_cairo_dock_set_blend_alpha ();
	
		cairo_dock_gl_path_arc (pPath, iNbPoints1Round, -w, -h, r, -G_PI/2, -G_PI/2);  // coin bas gauche.
		
		cairo_dock_gl_path_arc (pPath, iNbPoints1Round, -w, h, r, -G_PI, -G_PI/2);  // coin haut gauche.
		
		cairo_dock_gl_path_rel_curve_to (pPath, iNbPointsCurve,
			w, 0,
			w, - pPanel->iMainIconSize,
			2*w, - pPanel->iMainIconSize);
		
		cairo_dock_gl_path_arc (pPath, iNbPoints1Round, w, h - pPanel->iMainIconSize, r, G_PI/2, -G_PI/2);  // coin haut droit.
		
		cairo_dock_gl_path_arc (pPath, iNbPoints1Round, w, -h, r, 0., -G_PI/2);  // coin bas droit.
		
		glColor4f (pPanel->fBgColor[0], pPanel->fBgColor[1], pPanel->fBgColor[2], 1.);
		glLineWidth (fLineWidth);
		cairo_dock_stroke_gl_path (pPath, TRUE);
		
		glColor4f (pPanel->fBgColor[0], pPanel->fBgColor[1], pPanel->fBgColor[2], pPanel->fBgColor[3]);
		cairo_dock_fill_gl_path (pPath, 0);
	}
	
	glTranslatef (-pDesklet->container.iWidth/2, -pDesklet->container.iHeight/2, 0.);
	
	// draw main icon.
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (1.);
	
	Icon *pIcon = pDesklet->pIcon;
	if (pIcon && pIcon->iIconTexture != 0 )
	{
		glPushMatrix ();
		
		glTranslatef (floor (pIcon->fDrawX + pIcon->fWidth/2),
			floor (pDesklet->container.iHeight - pIcon->fDrawY - pIcon->fHeight/2),
			0.);
		_cairo_dock_apply_texture_at_size (pIcon->iIconTexture, pIcon->fWidth, pIcon->fHeight);
		
		/**if (pIcon->iQuickInfoTexture != 0)
		{
			glPushMatrix ();
			double dx = .5 * (pIcon->iQuickInfoWidth & 1);  // on decale la texture pour la coller sur la grille des coordonnees entieres.
			double dy = .5 * (pIcon->iQuickInfoHeight & 1);
			
			glTranslatef (floor (pIcon->fWidth/2 + pIcon->iQuickInfoWidth/2) + dx, dy, 0.);
			
			_cairo_dock_apply_texture_at_size (pIcon->iQuickInfoTexture,
				pIcon->iQuickInfoWidth,
				pIcon->iQuickInfoHeight);
			glPopMatrix ();
		}*/
		cairo_dock_draw_icon_overlays_opengl (pIcon, pDesklet->container.fRatio);
		
		glPopMatrix ();
	}
	
	// draw icons.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDesklet->icons);
	if (pFirstDrawnElement == NULL)
		return;
	GList *ic = pFirstDrawnElement;
	do
	{
		pIcon = ic->data;
		
		if (pIcon->iIconTexture != 0 && ! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			glPushMatrix ();
			
			glTranslatef (floor (pIcon->fDrawX + pIcon->fWidth/2),
				floor (pDesklet->container.iHeight - pIcon->fDrawY - pIcon->fHeight/2),
				0.);
			
			_cairo_dock_enable_texture ();  // cairo_dock_draw_icon_overlays_opengl() disable textures
			_cairo_dock_apply_texture_at_size (pIcon->iIconTexture, pIcon->fWidth, pIcon->fHeight);
			
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
					if (pIcon->fDrawX + pIcon->fWidth + pIcon->label.iWidth/2 > pDesklet->container.iWidth)
						fOffsetX = pDesklet->container.iWidth - (pIcon->fDrawX + pIcon->fWidth + pIcon->label.iWidth/2);
					if (pIcon->fDrawX + pIcon->fWidth - pIcon->label.iWidth/2 < 0)
						fOffsetX = pIcon->label.iWidth/2 - (pIcon->fDrawX + pIcon->fWidth);
				}
				else
				{
					_cairo_dock_set_alpha (.6);
					if (pIcon->label.iWidth > 2*pIcon->fWidth + 2 * myIconsParam.iLabelSize)
					{
						fOffsetX = 0.;
						u1 = (double) (2*pIcon->fWidth + 2 * myIconsParam.iLabelSize) / pIcon->label.iWidth;
					}
				}
				
				glTranslatef (ceil (-pIcon->fWidth/2 + fOffsetX + pIcon->label.iWidth/2) + dx, ceil (pIcon->fHeight/2 + pIcon->label.iHeight / 2) + dy, 0.);
				
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
				double dx = .5 * (pIcon->iQuickInfoWidth & 1);  // on decale la texture pour la coller sur la grille des coordonnees entieres.
				double dy = .5 * (pIcon->iQuickInfoHeight & 1);
				
				glTranslatef (floor (pIcon->fWidth/2 + pIcon->iQuickInfoWidth/2) + dx, dy, 0.);
				
				_cairo_dock_set_alpha (1.);
				_cairo_dock_apply_texture_at_size (pIcon->iQuickInfoTexture,
					pIcon->iQuickInfoWidth,
					pIcon->iQuickInfoHeight);
			}*/
			cairo_dock_draw_icon_overlays_opengl (pIcon, pDesklet->container.fRatio);
			
			glPopMatrix ();
		}
		ic = cairo_dock_get_next_element (ic, pDesklet->icons);
	} while (ic != pFirstDrawnElement);
	
	_cairo_dock_disable_texture ();
}



void rendering_register_panel_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render 			= (CairoDeskletRenderFunc) render;
	pRenderer->configure 		= (CairoDeskletConfigureRendererFunc) configure;
	pRenderer->load_data 		= (CairoDeskletLoadRendererDataFunc) NULL;
	pRenderer->free_data 		= (CairoDeskletFreeRendererDataFunc) free_data;
	pRenderer->calculate_icons 	= (CairoDeskletCalculateIconsFunc) calculate_icons;
	pRenderer->render_opengl 	= (CairoDeskletGLRenderFunc) render_opengl;
	
	cairo_dock_register_desklet_renderer ("Panel", pRenderer);
}
