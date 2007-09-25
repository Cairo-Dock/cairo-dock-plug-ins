/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <string.h>

#include <cairo-dock.h>

#include "file-manager-load-directory.h"

extern FileManagerGetFileInfoFunc file_manager_get_file_info;
extern FileManagerListDirectoryFunc file_manager_list_directory;
extern FileManagerAddMonitorFunc file_manager_add_monitor;
extern FileManagerAddMonitorFunc file_manager_remove_monitor;

extern FileManagerSortType my_fm_iSortType;


void file_manager_create_dock_from_directory (Icon *pIcon)
{
	g_free (pIcon->acCommand);
	GList *pIconList = file_manager_list_directory (pIcon->cBaseURI, my_fm_iSortType, &pIcon->acCommand);
	pIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, pIcon->acName);
	
	file_manager_add_monitor (pIcon);
}

void file_manager_alter_icon_if_necessary (Icon *pIcon, CairoDock *pDock)
{
	Icon *pNewIcon = file_manager_create_icon_from_URI (pIcon->cBaseURI, pDock);
	g_return_if_fail (pNewIcon != NULL);
	
	
	if (strcmp (pIcon->acName, pNewIcon->acName) != 0 || strcmp (pIcon->acFileName, pNewIcon->acFileName) != 0 || pIcon->fOrder != pNewIcon->fOrder)
	{
		cairo_dock_remove_one_icon_from_dock (pDock, pIcon);
		
		cairo_dock_insert_icon_in_dock (pNewIcon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);  // on met a jour la taille du dock pour le fXMin/fXMax, et eventuellement la taille de l'icone peut aussi avoir change.
		
		if (pIcon->pSubDock != NULL)
		{
			pNewIcon->pSubDock == pIcon->pSubDock;
			pIcon->pSubDock = NULL;
			
			if (pNewIcon->acName != NULL && strcmp (pIcon->acName, pNewIcon->acName) != 0)
			{
				g_hash_table_steal (g_hDocksTable, pIcon->acName);
				g_hash_table_insert (g_hDocksTable, pNewIcon->acName, pNewIcon->pSubDock);
			}
		}
		
		cairo_dock_free_icon (pIcon);
	}
	else
	{
		cairo_dock_free_icon (pNewIcon);
	}
}
Icon *file_manager_create_icon_from_URI (gchar *cURI, CairoDock *pDock)
{
	Icon *pNewIcon = g_new0 (Icon, 1);
	pNewIcon->iType = CAIRO_DOCK_LAUNCHER;
	pNewIcon->cBaseURI = g_strdup (cURI);
	gboolean bIsDirectory;
	file_manager_get_file_info (cURI, &pNewIcon->acName, &pNewIcon->acCommand, &pNewIcon->acFileName, &bIsDirectory, &pNewIcon->iVolumeID, &pNewIcon->fOrder, my_fm_iSortType);
	if (pNewIcon->acName == NULL)
	{
		cairo_dock_free_icon (pNewIcon);
		return NULL;
	}
	
	if (bIsDirectory)
	{
		g_print ("  c'est un sous-repertoire\n");
	}
	
	if (my_fm_iSortType == FILE_MANAGER_SORT_BY_NAME)
	{
		GList *ic;
		Icon *icon;
		for (ic = pDock->icons; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (strcmp (pNewIcon->acName, icon->acName) < 0)
			{
				if (ic->prev != NULL)
				{
					Icon *prev_icon = ic->prev->data;
					pNewIcon->fOrder = (icon->fOrder + prev_icon->fOrder) / 2;
				}
				else
					pNewIcon->fOrder = icon->fOrder - 1;
				break ;
			}
			else if (ic->next == NULL)
			{
				pNewIcon->fOrder = icon->fOrder + 1;
			}
		}
	}
	cairo_dock_load_one_icon_from_scratch (pNewIcon, pDock);
	
	return pNewIcon;
}
void file_monitor_action_on_event (FileManagerEventType iEventType, const gchar *cURI, Icon *pIcon)
{
	g_print ("%s ()\n", __func__);
	g_print ("%s (%d sur %s)\n", __func__, iEventType, cURI);
	
	if (iEventType == FILE_MANAGER_ICON_DELETED)
	{
		Icon *pConcernedIcon;
		CairoDock *pParentDock;
		if (pIcon->pSubDock != NULL)
		{
			pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIcon->pSubDock->icons, cURI);
			g_return_if_fail (pConcernedIcon != NULL);
			pParentDock = pIcon->pSubDock;
		}
		else
		{
			pConcernedIcon = pIcon;
			pParentDock = cairo_dock_search_container_from_icon (pIcon);
		}
		g_print ("%s est supprimee\n", pConcernedIcon->acName);
		
		cairo_dock_remove_one_icon_from_dock (pParentDock, pConcernedIcon);
		cairo_dock_update_dock_size (pParentDock, pParentDock->iMaxIconHeight, pParentDock->iMinDockWidth);
		cairo_dock_free_icon (pConcernedIcon);
		/*if (! pIcon->pSubDock->bInside && pIcon->pSubDock->bAtBottom)
			pConcernedIcon->fPersonnalScale = .05;
		else
			pConcernedIcon->fPersonnalScale = 1.0;
		if (pParentDock->iSidShrinkDown == 0)
			pParentDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pParentDock);*/
	}
	else if (iEventType == FILE_MANAGER_ICON_CREATED)
	{
		Icon *pNewIcon = file_manager_create_icon_from_URI (cURI, pIcon->pSubDock);
		
		cairo_dock_insert_icon_in_dock (pNewIcon, pIcon->pSubDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
		/*if (! pIcon->pSubDock->bInside && g_bAutoHide && pIcon->pSubDock->bAtBottom)
			pNewIcon->fPersonnalScale = - 0.05;
		if (pIcon->pSubDock->iSidShrinkDown == 0)
			pIcon->pSubDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pIcon->pSubDock);*/
	}
	else if (iEventType == FILE_MANAGER_ICON_MODIFIED)
	{
		Icon *pConcernedIcon;
		CairoDock *pParentDock;
		if (strcmp (pIcon->cBaseURI, cURI) == 0)
		{
			pConcernedIcon = pIcon;
			pParentDock = cairo_dock_search_container_from_icon (pIcon);
		}
		else if (pIcon->pSubDock != NULL)
		{
			pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIcon->pSubDock->icons, cURI);
			g_return_if_fail (pConcernedIcon != NULL);
			pParentDock = pIcon->pSubDock;
		}
		g_print ("%s est modifiee\n", pConcernedIcon->acName);
		
		file_manager_alter_icon_if_necessary (pConcernedIcon, pParentDock);
	}
}


void file_manager_reload_directories (gchar *cName, CairoDock *pDock, gpointer data)
{
	GList *ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->cBaseURI != NULL)
		{
			if (icon->pSubDock != NULL && icon->pSubDock->icons == NULL)
			{
				g_free (icon->acCommand);
				icon->pSubDock->icons = file_manager_list_directory (icon->cBaseURI, my_fm_iSortType, &icon->acCommand);
				cairo_dock_load_buffers_in_one_dock (icon->pSubDock);
				
				file_manager_add_monitor (icon);
			}
			if (icon->iVolumeID > 0)
				file_manager_alter_icon_if_necessary (icon, pDock);
		}
	}
}

void file_manager_unload_directories (gchar *cName, CairoDock *pDock, gpointer data)
{
	GList *ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->cBaseURI != NULL && icon->pSubDock != NULL)  //  && icon->pSubDock->icons != NULL
		{
			file_manager_remove_monitor (icon);
			
			GList *pIconList = icon->pSubDock->icons;
			icon->pSubDock->icons = NULL;
			
			Icon *icon;
			GList *ic;
			for (ic = pIconList; ic != NULL; ic = ic->next)
			{
				icon = ic->data;
				cairo_dock_free_icon (icon);
			}
			g_list_free (pIconList);
		}
	}
}


static int file_manager_sort_by_name (Icon *icon1, Icon *icon2)
{
	gchar *cURI_1 = g_ascii_strdown (icon1->acName, -1);
	gchar *cURI_2 = g_ascii_strdown (icon2->acName, -1);
	int iOrder = strcmp (cURI_1, cURI_2);
	g_free (cURI_1);
	g_free (cURI_2);
	return iOrder;
}
static int file_manager_sort_by_other (Icon *icon1, Icon *icon2)
{
	if (icon1->fOrder < icon2->fOrder)
		return -1;
	else if (icon1->fOrder > icon2->fOrder)
		return 1;
	else
		return 0;
}
GList *file_manager_sort_files (GList *pIconList, FileManagerSortType iSortType)
{
	GList *pSortedIconList;
	if (iSortType == FILE_MANAGER_SORT_BY_NAME)
	{
		pSortedIconList = g_list_sort (pIconList, (GCompareFunc) file_manager_sort_by_name);
		int iOrder = 0;
		Icon *icon;
		GList *ic;
		for (ic = pIconList; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			icon->fOrder = iOrder ++;
		}
	}
	else
	{
		pSortedIconList = g_list_sort (pIconList, (GCompareFunc) file_manager_sort_by_other);
	}
	return pSortedIconList;
}
