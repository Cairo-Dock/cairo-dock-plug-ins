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


void cd_satus_notifier_compute_grid (void)
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
		if (pItem->iStatus != CD_STATUS_PASSIVE)
			iNbItems ++;
	}
	
	// taille disponible.
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	g_print ("icon: %dx%d\n", iWidth, iHeight);
	
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
	//g_print ("satus_notifier : %dx%d\n", myData.iNbLines, myData.iNbColumns);
}


void cd_satus_notifier_draw_compact_icon (void)
{
	//g_print ("%s ()\n", __func__);
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	// erase all.
	cairo_dock_erase_cairo_context (myDrawContext);
	
	// icons background.
	if (g_pIconBackgroundBuffer.pSurface != NULL)
	{
		cairo_save (myDrawContext);
		cairo_scale(myDrawContext,
			iWidth / g_pIconBackgroundBuffer.iWidth,
			iHeight / g_pIconBackgroundBuffer.iHeight);
		cairo_set_source_surface (myDrawContext,
			g_pIconBackgroundBuffer.pSurface,
			0.,
			0.);
		cairo_paint (myDrawContext);
		cairo_restore (myDrawContext);
	}
	
	int x_pad = (iWidth - myData.iItemSize * myData.iNbColumns) / 2;  // pour centrer le dessin.
	int y_pad = (iHeight - myData.iItemSize * myData.iNbLines) / 2;
	
	// draw each active item, in lines, from left to right.
	int i = 0, j = 0;  // ligne, colonne
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pItem->pSurface != NULL && pItem->iStatus != CD_STATUS_PASSIVE)
		{
			cairo_set_source_surface (myDrawContext,
				pItem->pSurface,
				x_pad + j * myData.iItemSize,
				y_pad + i * myData.iItemSize);
			cairo_paint (myDrawContext);
			
			j ++;
			if (j == myData.iNbColumns)  // ligne suivante.
			{
				j = 0;
				i ++;
			}
		}
	}
	
	// update cairo/opengl
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)  // we are lazy, we could do an opengl rendering.
	{
		cairo_dock_update_icon_texture (myIcon);
	}
	else if (myDock)  // les reflets pour cairo.
	{
		CD_APPLET_UPDATE_REFLECT_ON_MY_ICON;
	}
	
	CD_APPLET_REDRAW_MY_ICON;
}


void cd_satus_notifier_reload_compact_mode (void)
{
	//g_print ("%s ()\n", __func__);
	// re-compute the grid.
	int iPrevSize = myData.iItemSize;
	cd_satus_notifier_compute_grid ();
	
	// reload surfaces if their size has changed.
	//g_print (" size: %d -> %d\n", iPrevSize, myData.iItemSize);
	CDStatusNotifierItem *pItem;
	GList *it;
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pItem->iStatus != CD_STATUS_PASSIVE)
		{
			if (iPrevSize != myData.iItemSize || pItem->pSurface == NULL)
			{
				gchar *cIconPath = cd_satus_notifier_search_item_icon_s_path (pItem);
				if (cIconPath != NULL)
				{
					if (pItem->pSurface != NULL)
						cairo_surface_destroy (pItem->pSurface);
					pItem->pSurface = cairo_dock_create_surface_from_icon (cIconPath, myData.iItemSize, myData.iItemSize);
					g_free (cIconPath);
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
	
	int iMouseX, iMouseY;
	iMouseX = myContainer->iMouseX - myIcon->fDrawX;
	iMouseY = myContainer->iMouseY - myIcon->fDrawY;
	
	//g_print ("%s (%d;%d)\n", __func__, iMouseX, iMouseY);
	// get index on the grid.
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	int x_pad = (iWidth - myData.iItemSize * myData.iNbColumns) / 2;
	int y_pad = (iHeight - myData.iItemSize * myData.iNbLines) / 2;
	
	int line, col;  // line, column
	col = (iMouseX - x_pad) / myData.iItemSize;
	line = (iMouseY - y_pad) / myData.iItemSize;
	
	// get item from index.
	CDStatusNotifierItem *pItem, *pFoundItem = NULL;
	GList *it;
	int i=0, j=0;  // line, column
	for (it = myData.pItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		if (pItem->pSurface != NULL && pItem->iStatus != CD_STATUS_PASSIVE)
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
		gchar *cIconPath = cd_satus_notifier_search_item_icon_s_path (pItem);
		if (cIconPath != NULL)
		{
			if (pItem->pSurface != NULL)
				cairo_surface_destroy (pItem->pSurface);
			pItem->pSurface = cairo_dock_create_surface_from_icon (cIconPath, myData.iItemSize, myData.iItemSize);
			g_free (cIconPath);
		}
		cd_satus_notifier_draw_compact_icon ();
	}
	else
	{
		Icon *pIcon = cd_satus_notifier_get_icon_from_item (pItem);
		if (pIcon != NULL && pIcon->pIconBuffer != NULL)
		{
			cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
			cairo_dock_set_image_on_icon (pIconContext,
				pItem->iStatus == CD_STATUS_NEEDS_ATTENTION ? pItem->cAttentionIconName : pItem->cIconName,
				pIcon, CD_APPLET_MY_ICONS_LIST_CONTAINER);
			cairo_destroy (pIconContext);
		}
	}
}
