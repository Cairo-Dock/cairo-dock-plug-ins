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

#include "applet-struct.h"
#include "applet-bookmarks.h"
#include "applet-disk-usage.h"
#include "applet-drives.h"
#include "applet-load-icons.h"


void cd_shortcuts_set_icon_order_by_name (Icon *pNewIcon, GList *pIconsList)
{
	GList *ic;
	Icon *pIcon;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->iType == pNewIcon->iType)
			break;
	}
	GList *ic0 = ic;
	if (! ic0)
	{
		pNewIcon->fOrder = 0;
		return;
	}
	
	pIcon = ic0->data;
	if (cairo_dock_compare_icons_name (pNewIcon, pIcon) <= 0)
	{
		pNewIcon->fOrder = pIcon->fOrder - 1;
		g_print ("name : %s <= %s -> %.2f\n", pNewIcon->cName, pIcon->cName, pNewIcon->fOrder);
		return;
	}
	
	pNewIcon->fOrder = 0;
	for (ic = ic0; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->iType != pNewIcon->iType)
			break;
		if (cairo_dock_compare_icons_name (pNewIcon, pIcon) < 0)
		{
			if (ic->prev == NULL)
				pNewIcon->fOrder = pIcon->fOrder - 1;
			else
			{
				Icon *pPrevIcon = ic->prev->data;
				pNewIcon->fOrder = (pIcon->fOrder + pPrevIcon->fOrder) / 2;
			}
			g_print ("  name : %s < %s -> %.2f\n", pNewIcon->cName, pIcon->cName, pNewIcon->fOrder);
			break;
		}
		pNewIcon->fOrder = pIcon->fOrder + 1;
	}
}


static void _cd_shortcuts_on_network_event (CairoDockFMEventType iEventType, const gchar *cURI, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, CD_NETWORK_GROUP, CAIRO_DOCK_FM_SORT_BY_NAME);
	CD_APPLET_LEAVE();
}


static GList * _load_icons (CairoDockModuleInstance *myApplet)
{
	GList *pIconList = NULL;
	gchar *cFullURI = NULL;
	
	if (myConfig.bListDrives)
	{
		pIconList = cd_shortcuts_list_drives (myApplet);
	}
	
	if (myConfig.bListNetwork)
	{
		GList *pIconList2 = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_NETWORK, CAIRO_DOCK_FM_SORT_BY_NAME, CD_NETWORK_GROUP, FALSE, 100, &cFullURI);
		cd_message ("  cFullURI : %s", cFullURI);
		
		/**if (myConfig.bUseSeparator && myDock && pIconList2 != NULL && pIconList != NULL)
		{
			Icon *pSeparatorIcon = cairo_dock_create_separator_icon (7, NULL);  // NULL => ne charge pas l'icone, car on est dans un thread.
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}*/
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, TRUE, NULL, (CairoDockFMMonitorCallback) _cd_shortcuts_on_network_event, myApplet))
			cd_warning ("Shortcuts : can't monitor network");
		myData.cNetworkURI = cFullURI;
	}
		
	if (myConfig.bListBookmarks)
	{
		gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
		if (! g_file_test (cBookmarkFilePath, G_FILE_TEST_EXISTS))  // on le cree pour pouvoir ajouter des signets.
		{
			FILE *f = fopen (cBookmarkFilePath, "a");
			fclose (f);
		}
		
		GList *pIconList2 = cd_shortcuts_list_bookmarks (cBookmarkFilePath);
		
		/**if (myConfig.bUseSeparator && myDock && pIconList2 != NULL && pIconList != NULL)
		{
			Icon *pSeparatorIcon = cairo_dock_create_separator_icon (9, NULL);  // NULL => ne charge pas l'icone, car on est dans un thread.
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}*/
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cBookmarkFilePath, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_bookmarks_event, myApplet))
			cd_warning ("Shortcuts : can't monitor bookmarks");
		
		myData.cBookmarksURI = cBookmarkFilePath;
	}
	
	return pIconList;
}


void cd_shortcuts_get_shortcuts_data (CairoDockModuleInstance *myApplet)
{
	myData.pIconList = _load_icons (myApplet);
}


gboolean cd_shortcuts_build_shortcuts_from_data (CairoDockModuleInstance *myApplet)
{
	g_return_val_if_fail (myIcon != NULL, FALSE);  // paranoia
	CD_APPLET_ENTER;
	
	//\_______________________ On efface l'ancienne liste.
	CD_APPLET_DELETE_MY_ICONS_LIST;
	
	//\_______________________ On charge la nouvelle liste.
	const gchar *cDeskletRendererName = NULL;
	switch (myConfig.iDeskletRendererType)
	{
		case CD_DESKLET_SLIDE :
		default :
			cDeskletRendererName = "Slide";
		break ;
		
		case CD_DESKLET_TREE :
			cDeskletRendererName = "Tree";
		break ;
	}
	CD_APPLET_LOAD_MY_ICONS_LIST (myData.pIconList, myConfig.cRenderer, cDeskletRendererName, NULL);
	myData.pIconList = NULL;
	
	//\_______________________ On lance la tache de mesure des disques.
	cd_shortcuts_launch_disk_periodic_task (myApplet);
	
	CD_APPLET_LEAVE (TRUE);
}
