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
		if (pIcon->iGroup == pNewIcon->iGroup)
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
		if (pIcon->iGroup != pNewIcon->iGroup)
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


static void _cd_shortcuts_on_network_event (CairoDockFMEventType iEventType, const gchar *cURI, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	
	//g_print (" * event %d on network '%s'\n", iEventType, cURI);
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	GldiContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
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
			pNewIcon->iGroup = CD_NETWORK_GROUP;
			
			//\_______________________ on la place au bon endroit suivant son nom.
			cd_shortcuts_set_icon_order_by_name (pNewIcon, pIconsList);
			//g_print (" new network : %s, order = %.2f\n", pNewIcon->cName, pNewIcon->fOrder);
			
			//\_______________________ on l'insere dans la liste.
			CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNewIcon);
			
			//\_______________________ on affiche un message.
			gldi_dialog_show_temporary_with_icon_printf (
				D_("%s has been connected"),
				pNewIcon, pContainer,
				4000,
				NULL,  // son icone n'est pas encore chargee
				pNewIcon->cName);
		}
		break ;

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
			pNewIcon->iGroup = CD_NETWORK_GROUP;
			
			//\_______________________ on remplace l'icone si des choses ont change.
			if (cairo_dock_strings_differ (pConcernedIcon->cName, pNewIcon->cName) || cairo_dock_strings_differ (pConcernedIcon->cFileName, pNewIcon->cFileName))
			{
				//g_print (" '%s' -> '%s'\n'%s' -> '%s'\n", pConcernedIcon->cName, pNewIcon->cName, pConcernedIcon->cFileName, pNewIcon->cFileName);
				
				CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pConcernedIcon);
				pIconsList = CD_APPLET_MY_ICONS_LIST;
				
				cd_shortcuts_set_icon_order_by_name (pNewIcon, pIconsList);
				CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNewIcon);
			}
			else
			{
				gldi_object_unref (GLDI_OBJECT (pNewIcon));
			}
		}
		break ;

		case CAIRO_DOCK_NB_EVENT_ON_FILES :
		break ;
	}
	CD_APPLET_LEAVE();
}


static inline GList * _load_icons (CDSharedMemory *pSharedMemory)
{
	GList *pIconList = NULL;
	
	if (pSharedMemory->bListDrives)
	{
		pIconList = cd_shortcuts_list_drives (pSharedMemory);
	}
	
	if (pSharedMemory->bListNetwork)
	{
		gchar *cFullURI = NULL;
		GList *pIconList2 = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_NETWORK, CAIRO_DOCK_FM_SORT_BY_NAME, CD_NETWORK_GROUP, FALSE, 100, &cFullURI);
		cd_message ("  cFullURI : %s", cFullURI);
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		pSharedMemory->cNetworkURI = cFullURI;
	}
		
	if (pSharedMemory->bListBookmarks)
	{
		// guess the file we should use (from GTK 3.6, the new one should be used, but some system (like Mint-14) didn't switch on time and still use the old one...)
		gchar *cBookmarkFilePath = NULL;
		#if GTK_CHECK_VERSION (3, 6, 0)
		gchar *cBookmarkFilePathNew = g_strdup_printf ("%s/"GTK_BOOKMARKS_PATH, g_getenv ("HOME"));
		gchar *cBookmarkFilePathOld = g_strdup_printf ("%s/"GTK_BOOKMARKS_PATH_OLD, g_getenv ("HOME"));
		if (! g_file_test (cBookmarkFilePathNew, G_FILE_TEST_EXISTS))  // the new file doesn't exist yet, it's either that the old one is used, or that none is used
		{
			if (g_file_test (cBookmarkFilePathOld, G_FILE_TEST_EXISTS))
			{
				cBookmarkFilePath = cBookmarkFilePathOld;
				cBookmarkFilePathOld = NULL;
			}
			else  // none are used, use the new one
			{
				cBookmarkFilePath = cBookmarkFilePathNew;
				cBookmarkFilePathNew = NULL;
			}
		}
		else  // the new one exists -> take it, unless it's empty and the old one exists too (may happen if we created the new one when the old one was still used, in version 3.3.1)
		{
			if (cairo_dock_get_file_size (cBookmarkFilePathNew) == 0 && g_file_test (cBookmarkFilePathOld, G_FILE_TEST_EXISTS) && cairo_dock_get_file_size (cBookmarkFilePathOld) != 0)
			{
				cBookmarkFilePath = cBookmarkFilePathOld;
				cBookmarkFilePathOld = NULL;
			}
			else  // none are used, use the new one
			{
				cBookmarkFilePath = cBookmarkFilePathNew;
				cBookmarkFilePathNew = NULL;
			}
		}
		g_free (cBookmarkFilePathOld);
		g_free (cBookmarkFilePathNew);
		#else
		cBookmarkFilePath = g_strdup_printf ("%s/"GTK_BOOKMARKS_PATH_OLD, g_getenv ("HOME"));
		#endif
		// we create this file if it doesn't exist in order to be able to add bookmarks later
		if (! g_file_test (cBookmarkFilePath, G_FILE_TEST_EXISTS))
		{
			// first, we need to be sure that its directory exists
			char *str = strrchr (cBookmarkFilePath, '/'); // last occurrence of '/'
			*str = '\0';
			g_mkdir_with_parents (cBookmarkFilePath, 7*8*8+7*8+5);
			*str = '/';
			// create the empty file
			FILE *f = fopen (cBookmarkFilePath, "a");
			if (f)
				fclose (f);
		}
		
		GList *pIconList2 = cd_shortcuts_list_bookmarks (cBookmarkFilePath, pSharedMemory->pApplet);
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		pSharedMemory->cBookmarksURI = cBookmarkFilePath;
	}
	
	return pIconList;
}
static void cd_shortcuts_get_shortcuts_data (CDSharedMemory *pSharedMemory)
{
	pSharedMemory->pIconList = _load_icons (pSharedMemory);
}

static gboolean cd_shortcuts_build_shortcuts_from_data (CDSharedMemory *pSharedMemory)
{
	GldiModuleInstance *myApplet = pSharedMemory->pApplet;
	g_return_val_if_fail (myIcon != NULL, FALSE);  // paranoia
	CD_APPLET_ENTER;
	
	//\_______________________ get the result of the thread.
	GList *pIconList = pSharedMemory->pIconList;
	pSharedMemory->pIconList = NULL;
	myData.cDisksURI = pSharedMemory->cDisksURI;
	pSharedMemory->cDisksURI = NULL;
	myData.cNetworkURI = pSharedMemory->cNetworkURI;
	pSharedMemory->cNetworkURI = NULL;
	myData.cBookmarksURI = pSharedMemory->cBookmarksURI;
	pSharedMemory->cBookmarksURI = NULL;
	
	//\_______________________ monitor the sets.
	if (myData.cDisksURI)
	{
		if (! cairo_dock_fm_add_monitor_full (myData.cDisksURI, TRUE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_drive_event, myApplet))
			cd_warning ("Shortcuts : can't monitor drives");
	}
	if (myData.cNetworkURI)
	{
		if (! cairo_dock_fm_add_monitor_full (myData.cNetworkURI, TRUE, NULL, (CairoDockFMMonitorCallback) _cd_shortcuts_on_network_event, myApplet))
			cd_warning ("Shortcuts : can't monitor network");
	}
	if (myData.cBookmarksURI)
	{
		if (! cairo_dock_fm_add_monitor_full (myData.cBookmarksURI, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_bookmarks_event, myApplet))
			cd_warning ("Shortcuts : can't monitor bookmarks");
	}
	
	//\_______________________ On efface l'ancienne liste.
	CD_APPLET_DELETE_MY_ICONS_LIST;
	
	//\_______________________ On charge la nouvelle liste.
	const gchar *cDeskletRendererName = NULL;
	switch (myConfig.iDeskletRendererType)
	{
		case CD_DESKLET_SLIDE :
		default :
			cDeskletRendererName = "Viewport";
		break ;
		
		case CD_DESKLET_TREE :
			cDeskletRendererName = "Tree";
		break ;
	}
	CD_APPLET_LOAD_MY_ICONS_LIST (pIconList, myConfig.cRenderer, cDeskletRendererName, NULL);  // takes ownership of 'pIconList'
	
	//\_______________________ add a progress bar on disk volumes (must be done after inserting the icons into a container)
	pIconList = CD_APPLET_MY_ICONS_LIST;
	Icon *pIcon;
	GList *ic;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (CD_APPLET_GET_MY_ICON_DATA (pIcon) != NULL)  // drive
			cd_shortcuts_add_progress_bar (pIcon, myApplet);
	}
	
	//\_______________________ On lance la tache de mesure des disques.
	cd_shortcuts_launch_disk_periodic_task (myApplet);

	if (myData.bShowMenuPending)
	{
		gldi_object_notify (myContainer, NOTIFICATION_CLICK_ICON, myIcon, myDock, GDK_BUTTON1_MASK);
		myData.bShowMenuPending = FALSE;
	}
	
	gldi_task_discard (myData.pTask);
	myData.pTask = NULL;
	
	CD_APPLET_LEAVE (TRUE);
}

static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	g_free (pSharedMemory->cDisksURI);
	g_free (pSharedMemory->cNetworkURI);
	g_free (pSharedMemory->cBookmarksURI);
	g_list_foreach (pSharedMemory->pIconList, (GFunc)g_free, NULL);
	g_list_free (pSharedMemory->pIconList);
	g_free (pSharedMemory);
}
void cd_shortcuts_start (GldiModuleInstance *myApplet)
{
	if (myData.pTask != NULL)
	{
		gldi_task_discard (myData.pTask);
		myData.pTask = NULL;
	}
	
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	pSharedMemory->bListDrives = myConfig.bListDrives;
	pSharedMemory->bListNetwork = myConfig.bListNetwork;
	pSharedMemory->bListBookmarks = myConfig.bListBookmarks;
	pSharedMemory->pApplet = myApplet;
	
	myData.pTask = gldi_task_new_full (0,
		(GldiGetDataAsyncFunc) cd_shortcuts_get_shortcuts_data,
		(GldiUpdateSyncFunc) cd_shortcuts_build_shortcuts_from_data,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);

	if (cairo_dock_is_loading ())
		gldi_task_launch_delayed (myData.pTask, 0); // 0 <=> g_idle
	else
		gldi_task_launch (myData.pTask);
}

