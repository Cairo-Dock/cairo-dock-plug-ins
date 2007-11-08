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
extern FileManagerIsMountingPointFunc file_manager_is_mounting_point;

extern FileManagerSortType my_fm_iSortType;
extern Icon *my_fm_pIcon;


void file_manager_create_dock_from_directory (Icon *pIcon)
{
	g_print ("%s ()\n", __func__);
	g_free (pIcon->acCommand);
	GList *pIconList = file_manager_list_directory (pIcon->cBaseURI, my_fm_iSortType, &pIcon->acCommand);
	pIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, pIcon->acName);
	
	file_manager_add_monitor (pIcon);
}

static Icon *file_manager_alter_icon_if_necessary (Icon *pIcon, CairoDock *pDock)
{
	Icon *pNewIcon = file_manager_create_icon_from_URI (pIcon->cBaseURI, pDock);
	g_return_val_if_fail (pNewIcon != NULL && pNewIcon->acName != NULL, NULL);
	
	if (strcmp (pIcon->acName, pNewIcon->acName) != 0 || strcmp (pIcon->acFileName, pNewIcon->acFileName) != 0 || pIcon->fOrder != pNewIcon->fOrder)
	{
		g_print ("  on remplace %s\n", pIcon->acName);
		cairo_dock_remove_one_icon_from_dock (pDock, pIcon);
		if (pIcon->acDesktopFileName != NULL)
			file_manager_remove_monitor (pIcon);
		
		pNewIcon->acDesktopFileName = g_strdup (pIcon->acDesktopFileName);
		pNewIcon->cParentDockName = g_strdup (pIcon->cParentDockName);
		if (pIcon->pSubDock != NULL)
		{
			pNewIcon->pSubDock == pIcon->pSubDock;
			pIcon->pSubDock = NULL;
			
			if (pNewIcon->acName != NULL && strcmp (pIcon->acName, pNewIcon->acName) != 0)
			{
				g_hash_table_steal (g_hDocksTable, pIcon->acName);
				g_hash_table_insert (g_hDocksTable, pNewIcon->acName, pNewIcon->pSubDock);
			}  // else : detruire le sous-dock.
		}
		pNewIcon->fX = pIcon->fX;
		pNewIcon->fXAtRest = pIcon->fXAtRest;
		pNewIcon->fDrawX = pIcon->fDrawX;
		
		cairo_dock_insert_icon_in_dock (pNewIcon, pDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);  // on met a jour la taille du dock pour le fXMin/fXMax, et eventuellement la taille de l'icone peut aussi avoir change.
		
		cairo_dock_redraw_my_icon (pNewIcon, pDock);
		
		if (pNewIcon->acDesktopFileName != NULL)
			file_manager_add_monitor (pNewIcon);
		
		cairo_dock_free_icon (pIcon);
		return pNewIcon;
	}
	else
	{
		cairo_dock_free_icon (pNewIcon);
		return pIcon;
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
	g_return_if_fail (cURI != NULL && pIcon != NULL);
	g_print ("%s (%d sur %s)\n", __func__, iEventType, cURI);
	
	if (iEventType == FILE_MANAGER_ICON_DELETED)
	{
		Icon *pConcernedIcon;
		CairoDock *pParentDock;
		if (strcmp (cURI, pIcon->cBaseURI) == 0)
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
		else
		{
			g_print ("  on n'aurait pas du recevoir cet evenement !\n");
			return ;
		}
		g_print ("  %s sera supprimee\n", pConcernedIcon->acName);
		
		cairo_dock_remove_one_icon_from_dock (pParentDock, pConcernedIcon);
		if (pConcernedIcon->acDesktopFileName != NULL)  // alors elle a un moniteur.
			file_manager_remove_monitor (pConcernedIcon);
		cairo_dock_update_dock_size (pParentDock);
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
		if (strcmp (cURI, pIcon->cBaseURI) != 0 && pIcon->pSubDock != NULL)  // dans des cas foirreux, il se peut que le fichier soit cree alors qu'il existait deja dans le dock.
		{
			Icon *pNewIcon = file_manager_create_icon_from_URI (cURI, pIcon->pSubDock);
			
			cairo_dock_insert_icon_in_dock (pNewIcon, pIcon->pSubDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO);
			g_print ("  %s a ete insere\n", pNewIcon->acName);
			/*if (pIcon->pSubDock->iSidShrinkDown == 0)
				pIcon->pSubDock->iSidShrinkDown = g_timeout_add (50, (GSourceFunc) cairo_dock_shrink_down, (gpointer) pIcon->pSubDock);*/
		}
	}
	else if (iEventType == FILE_MANAGER_ICON_MODIFIED)
	{
		Icon *pConcernedIcon;
		CairoDock *pParentDock;
		if (strcmp (pIcon->cBaseURI, cURI) == 0)  // c'est l'icone elle-meme.
		{
			pConcernedIcon = pIcon;
			pParentDock = cairo_dock_search_container_from_icon (pIcon);
		}
		else if (pIcon->pSubDock != NULL)  // c'est a l'interieur du repertoire qu'elle represente.
		{
			pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIcon->pSubDock->icons, cURI);
			g_return_if_fail (pConcernedIcon != NULL);
			pParentDock = pIcon->pSubDock;
		}
		g_print ("  %s est modifiee (iRefCount:%d)\n", pConcernedIcon->acName, pParentDock->iRefCount);
		
		Icon *pNewIcon = file_manager_alter_icon_if_necessary (pConcernedIcon, pParentDock);
		
		if (pNewIcon != NULL && pNewIcon != pConcernedIcon && pNewIcon->iVolumeID > 0)
		{
			gboolean bIsMounted;
			gchar *cActivationURI = file_manager_is_mounting_point (pNewIcon->acCommand, &bIsMounted);
			g_free (cActivationURI);
			gchar *cMessage = g_strdup_printf ("%s is now %s", pNewIcon->acName, (bIsMounted ? "mounted" : "unmounted"));
			
			cairo_dock_show_temporary_dialog (cMessage, pNewIcon, pParentDock, 4000);
			g_free (cMessage);
		}
	}
}


void file_manager_reload_directories (gchar *cName, CairoDock *pDock, gpointer data)
{
	if (my_fm_pIcon != NULL && pDock == my_fm_pIcon->pSubDock)  // il vient d'etre cree, on ne le recharge donc pas.
		return ;
	g_print ("%s (%s)\n", __func__, cName);
	
	GList *ic = pDock->icons, *next_ic;
	Icon *icon;
	while (ic != NULL)
	{
		next_ic = ic->next;
		icon = ic->data;
		if (icon->cBaseURI != NULL)
		{
			g_print ("  on recharge %s\n", icon->cBaseURI);
			if (icon->pSubDock != NULL && icon->pSubDock->icons == NULL)
			{
				g_free (icon->acCommand);
				icon->pSubDock->icons = file_manager_list_directory (icon->cBaseURI, my_fm_iSortType, &icon->acCommand);
				cairo_dock_load_buffers_in_one_dock (icon->pSubDock);
			}
			
			if (icon->iVolumeID > 0)
			{
				g_print ("  iVolumeID:%d\n", icon->iVolumeID);
				Icon *pNewIcon = file_manager_alter_icon_if_necessary (icon, pDock);  // les infos dans le .desktop ne sont pas a jour.
				if (pNewIcon == icon)
					file_manager_add_monitor (pNewIcon);
			}
			else
				file_manager_add_monitor (icon);
		}
		ic = next_ic;
	}
}

void file_manager_unload_directories (gchar *cName, CairoDock *pDock, gpointer data)
{
	GList *ic;
	Icon *icon;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->cBaseURI != NULL)  //  && icon->pSubDock->icons != NULL
		{
			file_manager_remove_monitor (icon);
			
			if (icon->pSubDock != NULL)
			{
				g_print ("  on vide le sous-dock de %s\n", icon->acName);
				GList *pIconList = icon->pSubDock->icons;
				icon->pSubDock->icons = NULL;
				
				Icon *pSubIcon;
				GList *ic;
				for (ic = pIconList; ic != NULL; ic = ic->next)
				{
					pSubIcon = ic->data;
					cairo_dock_free_icon (pSubIcon);
				}
				g_list_free (pIconList);
			}
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
