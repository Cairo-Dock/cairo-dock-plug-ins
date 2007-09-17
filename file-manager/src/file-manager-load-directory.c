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
extern FileManagerLaunchUriFunc file_manager_launch_uri;
extern FileManagerIsMountingPointFunc file_manager_is_mounting_point;
extern FileManagerMountFunc file_manager_mount;
extern FileManagerUnmountFunc file_manager_unmount;
extern FileManagerAddMonitorFunc file_manager_add_monitor;
extern FileManagerAddMonitorFunc file_manager_remove_monitor;

extern FileManagerSortType g_fm_iSortType;


void file_manager_create_dock_from_directory (Icon *pIcon)
{
	/*CairoDock *pDock = cairo_dock_create_new_dock (GDK_WINDOW_TYPE_HINT_MENU, pIcon->acName);
	cairo_dock_reference_dock (pDock);  // on le fait tout de suite pour avoir la bonne reference avant le 'load'.
	
	pDock->icons = file_manager_list_directory (pIcon->acCommand, g_fm_iSortType);
	
	cairo_dock_load_buffers_in_one_dock (pDock);
	
	pIcon->pSubDock = pDock;
	
	while (gtk_events_pending ())
		gtk_main_iteration ();
	gtk_widget_hide (pDock->pWidget);*/
	GList *pIconList = file_manager_list_directory (pIcon->acCommand, g_fm_iSortType);
	pIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, pIcon->acName);
	
	file_manager_add_monitor (pIcon);
}


static Icon *file_manager_create_icon_from_URI (gchar *cURI, CairoDock *pDock)
{
	Icon *pNewIcon = g_new0 (Icon, 1);
	pNewIcon->iType = CAIRO_DOCK_LAUNCHER;
	pNewIcon->cBaseURI = g_strdup (cURI);
	gboolean bIsDirectory;
	file_manager_get_file_info (cURI, &pNewIcon->acName, &pNewIcon->acCommand, &pNewIcon->acFileName, &bIsDirectory, &pNewIcon->bIsMountingPoint, &pNewIcon->fOrder, g_fm_iSortType);
	
	if (bIsDirectory)
	{
		g_print ("  c'est un sous-repertoire\n");
	}
	
	if (g_fm_iSortType == FILE_MANAGER_SORT_BY_NAME)
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
		g_print ("%s est modifiee\n", pConcernedIcon->acName);
		
		Icon *pNewIcon = file_manager_create_icon_from_URI (cURI, pParentDock);
		
		if (strcmp (pConcernedIcon->acName, pNewIcon->acName) != 0 || strcmp (pConcernedIcon->acFileName, pNewIcon->acFileName) != 0 || pConcernedIcon->fOrder != pNewIcon->fOrder)
		{
			cairo_dock_remove_one_icon_from_dock (pParentDock, pConcernedIcon);
			
			cairo_dock_insert_icon_in_dock (pNewIcon, pIcon->pSubDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
			cairo_dock_update_dock_size (pParentDock, pParentDock->iMaxIconHeight, pParentDock->iMinDockWidth);
			cairo_dock_free_icon (pConcernedIcon);
		}
		else
		{
			cairo_dock_free_icon (pNewIcon);
		}
	}
}


void file_manager_reload_directories (gchar *cName, CairoDock *pDock, gpointer data)
{
	GList *ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->cBaseURI != NULL && icon->pSubDock != NULL && icon->pSubDock->icons == NULL)
		{
			icon->pSubDock->icons = file_manager_list_directory (icon->acCommand, g_fm_iSortType);
			cairo_dock_load_buffers_in_one_dock (icon->pSubDock);
			
			file_manager_add_monitor (icon);
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
