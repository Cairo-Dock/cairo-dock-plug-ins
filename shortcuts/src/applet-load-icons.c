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
		//g_print ("name : %s <= %s -> %.2f\n", pNewIcon->cName, pIcon->cName, pNewIcon->fOrder);
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
			//g_print ("  name : %s < %s -> %.2f\n", pNewIcon->cName, pIcon->cName, pNewIcon->fOrder);
			break;
		}
		pNewIcon->fOrder = pIcon->fOrder + 1;
	}
}


static void _cd_shortcuts_on_network_event (CairoDockFMEventType iEventType, const gchar *cURI, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	
	//g_print (" * event %d on network '%s'\n", iEventType, cURI);
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	CD_APPLET_LEAVE_IF_FAIL (pContainer != NULL);
	
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_DELETED :  // un reseau a ete deconnecte.
		{
			Icon *pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIconsList, cURI);
			if (pConcernedIcon == NULL)  // on cherche par nom.
			{
				pConcernedIcon = cairo_dock_get_icon_with_name (pIconsList, cURI);
			}
			if (pConcernedIcon == NULL)
			{
				cd_warning ("  an unknown network was removed");
				return ;
			}
			//g_print (" %s will be removed\n", pConcernedIcon->cName);
			
			CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pConcernedIcon);
		}
		break ;
		
		case CAIRO_DOCK_FILE_CREATED :  // un reseau a ete connecte.
		{
			//\_______________________ on verifie qu'elle n'existe pas deja.
			Icon *pSameIcon = cairo_dock_get_icon_with_base_uri (pIconsList, cURI);
			if (pSameIcon != NULL)
			{
				cd_warning ("this mount point (%s) already exists.", pSameIcon->cName);
				return;  // on decide de ne rien faire, c'est surement un signal inutile.
			}
			
			//\_______________________ on cree une icone pour cette nouvelle URI.
			Icon *pNewIcon = cairo_dock_fm_create_icon_from_URI (cURI, pContainer, CAIRO_DOCK_FM_SORT_BY_NAME);
			if (pNewIcon == NULL)
			{
				cd_warning ("couldn't create an icon for this network");
				return ;
			}
			pNewIcon->iType = CD_NETWORK_GROUP;
			
			//\_______________________ on la place au bon endroit suivant son nom.
			cd_shortcuts_set_icon_order_by_name (pNewIcon, pIconsList);
			//g_print (" new network : %s, order = %.2f\n", pNewIcon->cName, pNewIcon->fOrder);
			
			//\_______________________ on l'insere dans la liste.
			CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNewIcon);
			
			//\_______________________ on affiche un message.
			cairo_dock_show_temporary_dialog_with_icon_printf (
				D_("%s has been connected"),
				pNewIcon, pContainer,
				4000,
				NULL,  // son icone n'est pas encore chargee
				pNewIcon->cName);
		}
		
		case CAIRO_DOCK_FILE_MODIFIED :  // un point de montage a ete (de)monte
		{
			//\_______________________ on cherche l'icone concernee.
			Icon *pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIconsList, cURI);
			if (pConcernedIcon == NULL)  // on cherche par nom.
			{
				pConcernedIcon = cairo_dock_get_icon_with_name (pIconsList, cURI);
			}
			if (pConcernedIcon == NULL)
			{
				cd_warning ("  an unknown network was modified");
				return ;
			}
			//g_print (" %s is modified\n", pConcernedIcon->cName);
			
			//\_______________________ on recupere les infos actuelles.
			Icon *pNewIcon = cairo_dock_fm_create_icon_from_URI (cURI, pContainer, CAIRO_DOCK_FM_SORT_BY_NAME);
			if (pNewIcon == NULL)
			{
				cd_warning ("couldn't create an icon for this network");
				return ;
			}
			pNewIcon->iType = CD_NETWORK_GROUP;
			
			//\_______________________ on remplace l'icone si des choses ont change.
			if (cairo_dock_strings_differ (pConcernedIcon->cName, pNewIcon->cName) || cairo_dock_strings_differ (pConcernedIcon->cFileName, pNewIcon->cFileName))
			{
				//g_print (" '%s' -> '%s'\n'%s' -> '%s'\n", pConcernedIcon->cName, pNewIcon->cName, pConcernedIcon->cFileName, pNewIcon->cFileName);
				
				CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pConcernedIcon);
				
				cd_shortcuts_set_icon_order_by_name (pNewIcon, pIconsList);
				CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNewIcon);
			}
			else
			{
				cairo_dock_free_icon (pNewIcon);
			}
		}
	}
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
