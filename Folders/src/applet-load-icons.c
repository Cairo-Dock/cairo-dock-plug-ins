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

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-load-icons.h"

static void _cd_folders_remove_all_icons (CairoDockModuleInstance *myApplet);

void cd_shortcuts_set_icon_order (Icon *pNewIcon, GList *pIconsList, GCompareFunc comp)
{
	if (comp == NULL)
		return;
	g_print ("%s (%s)\n", __func__, pNewIcon->cName);
	// on cherche la 1ere icone du meme type.
	GList *ic;
	Icon *pIcon;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->iType == pNewIcon->iType)
			break;
	}
	GList *ic0 = ic;
	if (! ic0)  // si non trouve, on arrete la.
	{
		pNewIcon->fOrder = 0;
		return;
	}
	
	pIcon = ic0->data;
	if (comp (pNewIcon, pIcon) <= 0)
	{
		pNewIcon->fOrder = pIcon->fOrder - 1;
		g_print ("name : %s <= %s -> %.2f\n", pNewIcon->cName, pIcon->cName, pNewIcon->fOrder);
		return;
	}
	
	pNewIcon->fOrder = 0;
	for (ic = ic0; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		g_print ("  compare with %s (%.2f)\n", pIcon->cName, pIcon->fOrder);
		if (pIcon->iType != pNewIcon->iType)
		{
			g_print ("  type differ, break\n");
			break;
		}
		if (comp (pNewIcon, pIcon) < 0)
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
		g_print ("  fOrder <- %.2f\n", pNewIcon->fOrder);
	}
}


static void _manage_event_on_file (CairoDockFMEventType iEventType, const gchar *cBaseURI, GList *pIconsList, CairoContainer *pContainer, CairoDockModuleInstance *myApplet)
{
	gchar *cURI = (g_strdup (cBaseURI));
	cairo_dock_remove_html_spaces (cURI);
	g_print (" * event %d on '%s'\n", iEventType, cURI);
	
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_DELETED :  // un fichier a ete supprime (ce peut etre du a un renommage).
		{
			if (strcmp (myConfig.cDirPath, cBaseURI) == 0)
			{
				g_print ("our folder has been removed\n");
				_cd_folders_remove_all_icons (myApplet);
				return;
			}
			
			Icon *pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIconsList/**myData.pAllFiles*/, cURI);
			if (pConcernedIcon == NULL)  // on cherche par nom.
			{
				pConcernedIcon = cairo_dock_get_icon_with_name (pIconsList/**myData.pAllFiles*/, cURI);
			}
			if (pConcernedIcon == NULL)
			{
				cd_warning ("  an unknown file was removed");
				return ;
			}	
			g_print (" %s will be removed\n", pConcernedIcon->cName);
			
			// on l'enleve de la liste.
			///myData.pAllFiles = g_list_remove (myData.pAllFiles, pConcernedIcon);
			
			// on l'enleve du container.
			gboolean bInContainer = CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pConcernedIcon);  // detruit l'icone.
			
			// on la remplace par la 1ere sur la liste d'attente.
			/**if (bInContainer)
			{
				Icon *pNextIcon = g_list_nth_data (myData.pAllFiles, myConfig.iNbIcons - 1);
				g_print ("  insert next icon : %s\n", pNextIcon?pNextIcon->cBaseURI:"none");
				if (pNextIcon)
					CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNextIcon);
			}*/
		}
		break ;
		
		case CAIRO_DOCK_FILE_CREATED :  // un point de montage a ete connecte.
		{
			if (strcmp (myConfig.cDirPath, cBaseURI) == 0)
			{
				g_print ("our folder has been re-created\n");
				cairo_dock_launch_task (myData.pTask);
				return;
			}
			
			//\_______________________ on verifie qu'elle n'existe pas deja.
			Icon *pSameIcon = cairo_dock_get_icon_with_base_uri (pIconsList/**myData.pAllFiles*/, cURI);
			if (pSameIcon != NULL)
			{
				cd_warning ("this file (%s) already exists", pSameIcon->cName);
				return;  // on decide de ne rien faire, c'est surement un signal inutile.
			}
			
			//\_______________________ on cree une icone pour cette nouvelle URI.
			Icon *pNewIcon = cairo_dock_fm_create_icon_from_URI (cURI, pContainer, myConfig.iSortType);
			if (pNewIcon == NULL)
			{
				cd_warning ("couldn't create an icon for this file");
				return ;
			}
			pNewIcon->iType = (myConfig.bFoldersFirst && pNewIcon->iVolumeID == -1 ? 6 : 8);
			
			//\_______________________ on la place au bon endroit suivant son nom.
			cd_shortcuts_set_icon_order (pNewIcon, pIconsList/**myData.pAllFiles*/, myData.comp);
			g_print (" new file : %s, order = %.2f\n", pNewIcon->cName, pNewIcon->fOrder);
			
			//\_______________________ on l'insere dans la liste.
			///myData.pAllFiles = g_list_insert_sorted (myData.pAllFiles, pNewIcon, (GCompareFunc)cairo_dock_compare_icons_order);
			
			//\_______________________ si l'ordre est < la derniere icone du container, on l'insere dans celui-ci.
			/**Icon *pLastIcon = cairo_dock_get_last_icon (pIconsList);
			if (pLastIcon)  // container non vide.
			{
				guint iCurrentNbIcons = g_list_length (pIconsList);
				if (pNewIcon->fOrder < pLastIcon->fOrder || iCurrentNbIcons+1 <= myConfig.iNbIcons)
				{
					g_print (" on l'affiche\n");
					CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNewIcon);
					pIconsList = CD_APPLET_MY_ICONS_LIST;
					
					if (iCurrentNbIcons + 1 > myConfig.iNbIcons)  // trop d'icones, on enleve la derniere.
					{
						g_print ("trop d'icones, on enleve la derniere (%s)\n", pLastIcon->cName);
						CD_APPLET_DETACH_ICON_FROM_MY_ICONS_LIST (pLastIcon);
					}
				}
				else
					g_print ("ordre >= %.2f => on ne l'affiche pas\n", pLastIcon->fOrder);
			}
			else*/ // container vide, on l'insere juste.
			{
				CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNewIcon);
			}
		}
		break ;
		
		case CAIRO_DOCK_FILE_MODIFIED :  // un point de montage a ete (de)monte
		{
			Icon *pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIconsList, cURI);
			if (pConcernedIcon == NULL)  // on cherche par nom.
			{
				pConcernedIcon = cairo_dock_get_icon_with_name (pIconsList, cURI);
			}
			if (pConcernedIcon == NULL)
			{
				cd_warning ("  an unknown file was modified");
				return ;
			}
			g_print (" %s is modified\n", pConcernedIcon->cName);
			
			//\_______________________ on recupere les infos actuelles.
			Icon *pNewIcon = cairo_dock_fm_create_icon_from_URI (cURI, pContainer, myConfig.iSortType);
			if (pNewIcon == NULL)
			{
				cd_warning ("couldn't create an icon for this file");
				return ;
			}
			pNewIcon->iType = (myConfig.bFoldersFirst && pNewIcon->iVolumeID == -1 ? 6 : 8);
			double fCurrentOrder = pConcernedIcon->fOrder;
			if (myConfig.iSortType == 1 || myConfig.iSortType == 2)  // sort by date or size.
				pConcernedIcon->fOrder = pNewIcon->fOrder;
			
			//\_______________________ on gere le changement de nom.
			if (cairo_dock_strings_differ (pConcernedIcon->cName, pNewIcon->cName))  // le nom a change.
			{
				g_print ("  name changed : '%s' -> '%s'\n", pConcernedIcon->cName, pNewIcon->cName);
				cairo_dock_set_icon_name (pNewIcon->cName, pConcernedIcon, pContainer);
				cd_shortcuts_set_icon_order (pConcernedIcon, pIconsList/**myData.pAllFiles*/, myData.comp);
			}
			
			//\_______________________ on gere le changement d'image.
			if (cairo_dock_strings_differ (pConcernedIcon->cFileName, pNewIcon->cFileName))
			{
				g_print ("  image changed : '%s' -> '%s'\n", pConcernedIcon->cFileName, pNewIcon->cFileName);
				g_free (pConcernedIcon->cFileName);
				pConcernedIcon->cFileName = g_strdup (pNewIcon->cFileName);
				
				if (pConcernedIcon->pIconBuffer != NULL)
					cairo_dock_load_icon_image (pConcernedIcon, pContainer);
			}
			
			//\_______________________ on gere le changement d'ordre (du au changement de nom, d'extension, ou de taille, suivant le classement utilise).
			if (pConcernedIcon->fOrder != fCurrentOrder)
			{
				g_print ("  order changed : %.2f -> %.2f\n", fCurrentOrder, pConcernedIcon->fOrder);
				
				// on la detache.
				///myData.pAllFiles = g_list_remove (myData.pAllFiles, pConcernedIcon);
				gboolean bInContainer = CD_APPLET_DETACH_ICON_FROM_MY_ICONS_LIST (pConcernedIcon);
				pIconsList = CD_APPLET_MY_ICONS_LIST;
				
				/**if (bInContainer)
				{
					Icon *pNextIcon = g_list_nth_data (myData.pAllFiles, myConfig.iNbIcons);
					if (!pNextIcon || pConcernedIcon->fOrder < pNextIcon->fOrder)
					{
						g_print (" on reinsere l'icone\n");
						CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pConcernedIcon);
					}
				}
				else
				{
					Icon *pLastIcon = cairo_dock_get_last_icon (pIconsList);
					if (!pLastIcon || pConcernedIcon->fOrder < pLastIcon->fOrder || g_list_length (pIconsList) < myConfig.iNbIcons)
					{
						g_print (" on reinsere l'icone\n");
						CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pConcernedIcon);
					}
				}
				myData.pAllFiles = g_list_insert_sorted (myData.pAllFiles, pConcernedIcon, (GCompareFunc)cairo_dock_compare_icons_order);*/
				CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pConcernedIcon);
			}
			cairo_dock_free_icon (pNewIcon);
		}
		break ;
	}
	g_free (cURI);
}

static void _cd_folders_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, CairoDockModuleInstance *myApplet)
{
	g_return_if_fail (cURI != NULL);
	CD_APPLET_ENTER;
	
	//\________________ On gere l'evenement sur le fichier.
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	CD_APPLET_LEAVE_IF_FAIL (pContainer != NULL);
	
	_manage_event_on_file (iEventType, cURI, pIconsList, pContainer, myApplet);
	
	CD_APPLET_LEAVE();
}


void cd_folders_get_data (CairoDockModuleInstance *myApplet)
{
	//\_______________________ On recupere les fichiers.
	gchar *cCommand = NULL;
	myData.pIconList = cairo_dock_fm_list_directory (myConfig.cDirPath, myConfig.iSortType, 8, myConfig.bShowHiddenFiles, 1e4, &cCommand);
	g_free (cCommand);
	
	//\_______________________ on classe les icones.
	if (myConfig.bFoldersFirst)
	{
		Icon *pIcon;
		GList *ic;
		for (ic = myData.pIconList; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->iVolumeID != 0)  // repertoire
				pIcon->iType = 6;
		}
	}
	
	if (myConfig.iSortType == 0)  // sort by name
	{
		myData.pIconList = g_list_sort (myData.pIconList, (GCompareFunc) cairo_dock_compare_icons_name);
	}
	else if (myConfig.iSortType == 3)  // sort by type
	{
		myData.pIconList = g_list_sort (myData.pIconList, (GCompareFunc) cairo_dock_compare_icons_extension);
	}
	else  // sort by date or size
	{
		myData.pIconList = g_list_sort (myData.pIconList, (GCompareFunc) cairo_dock_compare_icons_order);
	}
	
	//g_print ("=== files to display: ===\n");
	Icon *pIcon;
	int iOrder = 0;
	GList *ic;
	for (ic = myData.pIconList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		//g_print ("  %s (%d)\n", pIcon->cName, pIcon->iVolumeID);
		pIcon->fOrder = iOrder ++;
	}
}

	
gboolean cd_folders_load_icons_from_data (CairoDockModuleInstance *myApplet)
{
	g_return_val_if_fail (myIcon != NULL, FALSE);  // paranoia
	CD_APPLET_ENTER;
	
	//\_______________________ On efface l'ancienne liste.
	CD_APPLET_DELETE_MY_ICONS_LIST;
	
	//\_______________________ On charge la nouvelle liste.
	/**myData.pAllFiles = myData.pIconList;
	myData.pIconList = NULL;
	
	Icon *pIcon;
	guint i;
	GList *ic, *pList = NULL;
	for (i = 0, ic = myData.pAllFiles; i < myConfig.iNbIcons && ic != NULL; i++, ic = ic->next)
	{
		pIcon = ic->data;
		pList = g_list_prepend (pList, pIcon);
	}
	pList = g_list_reverse (pList);*/
	CD_APPLET_LOAD_MY_ICONS_LIST (myData.pIconList/**pList*/, myConfig.cRenderer, "Viewport", NULL);
	///myData.iNbIcons = myConfig.iNbIcons;
	myData.pIconList = NULL;
	
	//\_______________________ On se place en ecoute.
	cairo_dock_fm_add_monitor_full (myConfig.cDirPath, TRUE, NULL, (CairoDockFMMonitorCallback) _cd_folders_on_file_event, myApplet);
	
	CD_APPLET_LEAVE (TRUE);
}


static void _cd_folders_remove_all_icons (CairoDockModuleInstance *myApplet)
{
	//\_______________________ On stoppe la tache.
	cairo_dock_stop_task (myData.pTask);
	
	if (myData.pIconList != NULL)  // des donnees ont ete recuperees et non utilisees, on les libere.
	{
		g_list_foreach (myData.pIconList, (GFunc)g_free, NULL);
		g_list_free (myData.pIconList);
		myData.pIconList = NULL;
	}
	
	//\_______________________ On detruit d'abord les icones non chargees dans le container.
	/**Icon *pIcon;
	GList *ic = g_list_nth (myData.pAllFiles, myData.iNbIcons);
	for (;ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		g_print (" remove %s\n", pIcon->cName);
		cairo_dock_free_icon (pIcon);
	}
	
	g_list_free (myData.pAllFiles);
	myData.pAllFiles = NULL;*/
	
	//\_______________________ On detruit ensuite les icones chargees dans le container.
	CD_APPLET_DELETE_MY_ICONS_LIST;  // si le container a change entre-temps, le ModuleManager se chargera de nettoyer derriere nous.
	///myData.iNbIcons = 0;
}
void cd_folders_free_all_data (CairoDockModuleInstance *myApplet)
{
	//\_______________________ On arrete de surveiller le repertoire.
	cairo_dock_fm_remove_monitor_full (myConfig.cDirPath, TRUE, NULL);
	
	_cd_folders_remove_all_icons (myApplet);
	
	cairo_dock_free_task (myData.pTask);
	myData.pTask = NULL;
	
	cd_folders_free_apps_list (myApplet);
}


void cd_folders_sort_icons (CairoDockModuleInstance *myApplet, CairoDockFMSortType iSortType)
{
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	
	switch (iSortType)
	{
		case CAIRO_DOCK_FM_SORT_BY_NAME:
			
		break;
		case CAIRO_DOCK_FM_SORT_BY_DATE:
			
		break;
		case CAIRO_DOCK_FM_SORT_BY_SIZE:
			
		break;
		case CAIRO_DOCK_FM_SORT_BY_TYPE:
			
		break;
		default:
		break;
	}
}
