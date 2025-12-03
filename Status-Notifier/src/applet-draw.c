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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-item.h"
#include "applet-draw.h"


static void cd_satus_notifier_compute_grid (void)
{
	if (myData.pItems == NULL)
		return;
	
	// on compte les items actifs.
	int iNbItems = 0;
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (_item_is_visible (pItem))
			iNbItems ++;
	}
	
	// taille disponible.
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	cd_debug ("=== icon: %dx%d", iWidth, iHeight);
	
	// on calcule la meilleure grille.
	int iNbLines, iNbItemsByLine;
	int iSize, iSizeMax = 0;
	for (iNbLines = 1; iNbLines <= iNbItems; iNbLines ++)
	{
		iNbItemsByLine = ceil ((float)iNbItems / iNbLines);
		iSize = MIN (iWidth / iNbItemsByLine, iHeight / iNbLines);
		if (iSize > iSizeMax)
		{
			iSizeMax = iSize;
			myData.iNbLines = iNbLines;
			myData.iNbColumns = iNbItemsByLine;
			myData.iItemSize = iSize;
		}
	}
	//g_print ("=== satus_notifier : %dx%d\n", myData.iNbLines, myData.iNbColumns);
}

static void cd_satus_notifier_compute_icon_size (void)
{
	// count the active items.
	int iNbItems = 0;
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (_item_is_visible (pItem))
			iNbItems ++;
	}
	
	int w0, h0;  // default icon size, as set in the config.
	w0 = myData.iDefaultWidth;
	h0 = myData.iDefaultHeight;
	// current available icon size.
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	cd_debug ("=== icon: %dx%d", iWidth, iHeight);
	if (!myContainer->bIsHorizontal)
	{
		int tmp = iWidth;
		iWidth = iHeight;
		iHeight = tmp;
	}
	
	// compute the required width and the grid.
	int w;
	if (myContainer->bIsHorizontal)
	{
		myData.iNbLines = myConfig.iNbLines;
		myData.iItemSize = MAX (1, iHeight / myConfig.iNbLines);
		myData.iNbColumns = ceil ((float)iNbItems / myConfig.iNbLines);  // nb items by line.
		w = MAX (w0, myData.iItemSize * myData.iNbColumns + myIconsParam.iIconGap * (myData.iNbColumns - 1));
	}
	else
	{
		myData.iNbColumns = myConfig.iNbLines;
		myData.iItemSize = MAX (1, iHeight / myConfig.iNbLines);
		myData.iNbLines = ceil ((float)iNbItems / myConfig.iNbLines);  // nb items by line.
		w = MAX (h0, myData.iItemSize * myData.iNbLines + myIconsParam.iIconGap * (myData.iNbLines - 1));
	}
	cd_debug ("=== required width: %d (now: %d)", w, iWidth);
	
	// if width has changed, update the icon size.
	if (w != iWidth)
	{
		if (myContainer->bIsHorizontal)
			cairo_dock_resize_applet (myApplet, w, h0);
		else
			cairo_dock_resize_applet (myApplet, w0, w);
	}
}


static void cd_satus_notifier_draw_compact_icon (void)
{
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN_CAIRO ();

	int iIconGap;
	if (myConfig.bResizeIcon)
		iIconGap = myIconsParam.iIconGap;
	else
		iIconGap = 0;
	int x_pad = (iWidth - myData.iItemSize * myData.iNbColumns - iIconGap * (myData.iNbColumns - 1)) / 2;  // pad to center the drawing.
	int y_pad = (iHeight - myData.iItemSize * myData.iNbLines) / 2;
	cd_debug ("pad: %d;%d; grid: %dx%d, icon: %dx%d", x_pad, y_pad, myData.iNbLines, myData.iNbColumns, iWidth, iHeight);
	
	// draw each active item, in lines, from left to right.
	int i = 0, j = 0;  // ligne, colonne
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pItem->pSurface != NULL && _item_is_visible (pItem))
		{
			cd_debug ("===  draw %s (%d)", pItem->cId, pItem->iPosition);
			
			// optionally scale icons
			double w = cairo_image_surface_get_width (pItem->pSurface);
			double h = cairo_image_surface_get_height (pItem->pSurface);
			double s = MIN (myData.iItemSize / w, myData.iItemSize / h);
			if (s != 1.0)
			{
				cairo_save (myDrawContext);
				cairo_translate (myDrawContext, x_pad + j * (myData.iItemSize + iIconGap),
					y_pad + i * myData.iItemSize);
				cairo_scale (myDrawContext, s, s);
				cairo_pattern_t *pattern = cairo_pattern_create_for_surface (pItem->pSurface);
				cairo_set_source (myDrawContext, pattern);
				cairo_paint (myDrawContext);
				cairo_pattern_destroy (pattern);
				cairo_restore (myDrawContext);
			}
			else
			{
				cairo_set_source_surface (myDrawContext,
					pItem->pSurface,
					x_pad + j * (myData.iItemSize + iIconGap),
					y_pad + i * myData.iItemSize);
				cairo_paint (myDrawContext);
			}
			
			j ++;
			if (j == myData.iNbColumns)  // ligne suivante.
			{
				j = 0;
				i ++;
			}
		}
	}
	
	CD_APPLET_FINISH_DRAWING_MY_ICON_CAIRO;
}


void cd_satus_notifier_reload_compact_mode (void)
{
	cd_debug ("=== %s ()", __func__);
	// re-compute the grid.
	int iPrevSize = myData.iItemSize;
	if (myConfig.bResizeIcon)
		cd_satus_notifier_compute_icon_size ();
	else
		cd_satus_notifier_compute_grid ();
	
	// load surfaces, or reload them if their size has changed.
	cd_debug ("===  item size: %d -> %d, icon size: %dx%d", iPrevSize, myData.iItemSize, myIcon->image.iWidth, myIcon->image.iHeight);
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (_item_is_visible (pItem))
		{
			if (iPrevSize != myData.iItemSize || pItem->pSurface == NULL)
			{
				gchar *cIconPath = cd_satus_notifier_search_item_icon_s_path (pItem, myData.iItemSize);
				if (cIconPath != NULL)
				{
					if (pItem->pSurface != NULL)
						cairo_surface_destroy (pItem->pSurface);
					pItem->pSurface = cairo_dock_create_surface_from_icon (cIconPath, myData.iItemSize, myData.iItemSize);
					g_free (cIconPath);
				}
				else
				{
					cairo_surface_t *pSurface = ((pItem->iStatus == CD_STATUS_NEEDS_ATTENTION) && pItem->pFallbackIconAttention) ?
						pItem->pFallbackIconAttention : pItem->pFallbackIcon;
					if (pSurface)
					{
						if (pItem->pSurface) cairo_surface_destroy (pItem->pSurface);
						pItem->pSurface = cairo_surface_reference (pSurface);
					}
				}
			}
		}
	}
	
	// redraw all.
	cd_satus_notifier_draw_compact_icon ();
}


CDStatusNotifierItem *cd_satus_notifier_find_item_from_coord (void)
{
	if (myData.pItems == NULL)
		return NULL;
	
	//g_print ("=== %s (%d;%d)\n", __func__, iMouseX, iMouseY);
	// get grid extent
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);	
	
	// get coordinates on the grid.
	double fSizeX, fSizeY;
	cairo_dock_get_current_icon_size (myIcon, myContainer, &fSizeX, &fSizeY);
	
	int iMouseX, iMouseY;  // coordinates on the icon (taking into account the zoom).
	iMouseX = myContainer->iMouseX - myIcon->fDrawX;
	iMouseY = myContainer->iMouseY - myIcon->fDrawY;
	
	if (myContainer->bIsHorizontal)
	{
		iMouseX = iMouseX / fSizeX * iWidth;  // transform to the grid.
		iMouseY = iMouseY / fSizeY * iHeight;
	}
	else
	{
		int tmp = iMouseX;
		iMouseX = iMouseY / fSizeX * iWidth;  // transform to the grid.
		iMouseY = tmp / fSizeY * iHeight;
	}
	
	// get index on the grid.
	int iIconGap;
	if (myConfig.bResizeIcon)
		iIconGap = myIconsParam.iIconGap;
	else
		iIconGap = 0;
	int x_pad = (iWidth - myData.iItemSize * myData.iNbColumns - iIconGap * (myData.iNbColumns - 1)) / 2;
	int y_pad = (iHeight - myData.iItemSize * myData.iNbLines) / 2;
	
	int line, col;  // line, column
	col = (iMouseX - x_pad) / (myData.iItemSize + iIconGap);
	line = (iMouseY - y_pad) / myData.iItemSize;
	
	// get item from index.
	CDStatusNotifierItem *pItem, *pFoundItem = NULL;
	GList *it;
	int i=0, j=0;  // line, column
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pItem->pSurface != NULL && _item_is_visible (pItem))
		{
			if (i == line && j == col)
			{
				pFoundItem = pItem;
				break;
			}
			j ++;
			if (j == myData.iNbColumns)  // next line.
			{
				j = 0;
				i ++;
			}
		}
	}
	
	return pFoundItem;
}


void cd_satus_notifier_update_item_image (CDStatusNotifierItem *pItem)
{
	if (myConfig.bCompactMode)
	{
		gchar *cIconPath = cd_satus_notifier_search_item_icon_s_path (pItem, myData.iItemSize);
		if (cIconPath != NULL)
		{
			if (pItem->pSurface != NULL)
				cairo_surface_destroy (pItem->pSurface);
			pItem->pSurface = cairo_dock_create_surface_from_icon (cIconPath, myData.iItemSize, myData.iItemSize);
			g_free (cIconPath);
		}
		else
		{
			cairo_surface_t *pSurface = ((pItem->iStatus == CD_STATUS_NEEDS_ATTENTION) && pItem->pFallbackIconAttention) ?
				pItem->pFallbackIconAttention : pItem->pFallbackIcon;
			if (pSurface)
			{
				if (pItem->pSurface) cairo_surface_destroy (pItem->pSurface);
				pItem->pSurface = cairo_surface_reference (pSurface);
			}
		}
		cd_satus_notifier_draw_compact_icon ();
	}
	else
	{
		Icon *pIcon = cd_satus_notifier_get_icon_from_item (pItem);
		if (pIcon != NULL && pIcon->image.pSurface != NULL)
		{
			cairo_t *pIconContext = cairo_create (pIcon->image.pSurface);
			int iWidth, iHeight;
			cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
			gchar *cIconPath = cd_satus_notifier_search_item_icon_s_path (pItem, MAX (iWidth, iHeight));
			if (cIconPath)
			{
				cairo_dock_set_image_on_icon (pIconContext, cIconPath, pIcon, CD_APPLET_MY_ICONS_LIST_CONTAINER);
				g_free (cIconPath);
			}
			else
			{
				cairo_surface_t *pSurface = ((pItem->iStatus == CD_STATUS_NEEDS_ATTENTION) && pItem->pFallbackIconAttention) ?
					pItem->pFallbackIconAttention : pItem->pFallbackIcon;
				if (pSurface)
				{
					double w = cairo_image_surface_get_width (pSurface);
					double h = cairo_image_surface_get_height (pSurface);
					double s = MIN (iWidth / w, iHeight / h);
					cairo_scale (pIconContext, s, s);
					cairo_dock_set_icon_surface (pIconContext, pSurface, pIcon);
				}
			}
			cairo_destroy (pIconContext);
		}
	}
}
