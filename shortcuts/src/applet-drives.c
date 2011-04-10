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
#include "applet-load-icons.h"
#include "applet-drives.h"


static void _load_disk_icon (Icon *pIcon)
{
	CairoDockModuleInstance *myApplet = pIcon->pAppletOwner;
	int iWidth = pIcon->iImageWidth;
	int iHeight = pIcon->iImageHeight;
	if (pIcon->cFileName)  // icone possedant une image, on affiche celle-ci.
	{
		gchar *cIconPath = cairo_dock_search_icon_s_path (pIcon->cFileName);
		if (cIconPath != NULL && *cIconPath != '\0')
		{
			pIcon->pIconBuffer = cairo_dock_create_surface_from_image_simple (cIconPath,
				iWidth,
				iHeight);
			if (pIcon->pIconBuffer != NULL && myConfig.bDrawBar)
			{
				CDDiskUsage *pDiskUsage = CD_APPLET_GET_MY_ICON_DATA (pIcon);
				if (pDiskUsage != NULL && pDiskUsage->iTotal != 0)
				{
					double fValue;
					switch (myConfig.iDisplayType)
					{
						case CD_SHOW_FREE_SPACE :
							fValue = (double) pDiskUsage->iAvail / pDiskUsage->iTotal;
						break ;
						case CD_SHOW_USED_SPACE :
							fValue = (double) - pDiskUsage->iUsed / pDiskUsage->iTotal;  // <0 => du vert au rouge.
						break ;
						case CD_SHOW_FREE_SPACE_PERCENT :
							fValue = (double) pDiskUsage->iAvail / pDiskUsage->iTotal;
						break ;
						case CD_SHOW_USED_SPACE_PERCENT :
							fValue = (double) - pDiskUsage->iUsed / pDiskUsage->iTotal;  // <0 => du vert au rouge.
						break ;
						default:
							fValue = 0.;
						break;
					}
					cairo_t *pCairoContext = cairo_create (pIcon->pIconBuffer);
					cairo_dock_draw_bar_on_icon (pCairoContext, fValue, pIcon);
					cairo_destroy (pCairoContext);
				}
			}
		}
		g_free (cIconPath);
	}
}

static void _init_disk_usage (Icon *pIcon, CairoDockModuleInstance *myApplet)
{
	pIcon->iface.load_image = _load_disk_icon;
	pIcon->pAppletOwner = myApplet;
	CDDiskUsage *pDiskUsage = g_new0 (CDDiskUsage, 1);
	CD_APPLET_SET_MY_ICON_DATA (pIcon, pDiskUsage);
	if (pIcon->cCommand)
		cd_shortcuts_get_fs_stat (pIcon->cCommand, pDiskUsage);
}

static void _manage_event_on_drive (CairoDockFMEventType iEventType, const gchar *cBaseURI, GList *pIconsList, CairoContainer *pContainer, CairoDockModuleInstance *myApplet)
{
	gchar *cURI = (g_strdup (cBaseURI));
	cairo_dock_remove_html_spaces (cURI);
	//g_print (" * event %d on '%s'\n", iEventType, cURI);
	
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_DELETED :  // un point de montage a ete deconnecte.
		{
			Icon *pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIconsList, cURI);
			if (pConcernedIcon == NULL)  // on cherche par nom.
			{
				pConcernedIcon = cairo_dock_get_icon_with_name (pIconsList, cURI);
			}
			if (pConcernedIcon == NULL)
			{
				cd_warning ("  an unknown mount point was removed");
				return ;
			}
			//g_print (" %s will be removed\n", pConcernedIcon->cName);
			
			CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pConcernedIcon);
		}
		break ;
		
		case CAIRO_DOCK_FILE_CREATED :  // un point de montage a ete connecte.
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
				cd_warning ("couldn't create an icon for this mount point");
				return ;
			}
			pNewIcon->iGroup = CD_DRIVE_GROUP;
			_init_disk_usage (pNewIcon, myApplet);
			
			//\_______________________ on la place au bon endroit suivant son nom.
			cd_shortcuts_set_icon_order_by_name (pNewIcon, pIconsList);
			//g_print (" new drive : %s, order = %.2f\n", pNewIcon->cName, pNewIcon->fOrder);
			
			//\_______________________ on l'insere dans la liste.
			CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNewIcon);
			
			//\_______________________ on affiche un message.
			gboolean bIsMounted = FALSE;
			gchar *cUri = cairo_dock_fm_is_mounted (pNewIcon->cBaseURI, &bIsMounted);
			g_free (cUri);
			cairo_dock_show_temporary_dialog_with_icon_printf (
				bIsMounted ? D_("%s is now mounted") : D_("%s has been connected"),
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
				cd_warning ("  an unknown mount point was modified");
				return ;
			}
			//g_print (" %s is modified\n", pConcernedIcon->cName);
			
			//\_______________________ on recupere les infos actuelles.
			Icon *pNewIcon = cairo_dock_fm_create_icon_from_URI (cURI, pContainer, CAIRO_DOCK_FM_SORT_BY_NAME);
			if (pNewIcon == NULL)
			{
				cd_warning ("couldn't create an icon for this mount point");
				return ;
			}
			pNewIcon->iGroup = CD_DRIVE_GROUP;
			
			//\_______________________ on remplace l'icone si des choses ont change.
			if (cairo_dock_strings_differ (pConcernedIcon->cName, pNewIcon->cName) || cairo_dock_strings_differ (pConcernedIcon->cFileName, pNewIcon->cFileName))
			{
				//g_print (" '%s' -> '%s'\n'%s' -> '%s'\n", pConcernedIcon->cName, pNewIcon->cName, pConcernedIcon->cFileName, pNewIcon->cFileName);
				
				CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pConcernedIcon);
				
				cd_shortcuts_set_icon_order_by_name (pNewIcon, pIconsList);
				_init_disk_usage (pNewIcon, myApplet);
				CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNewIcon);
				
				pConcernedIcon = pNewIcon;  // pConcernedIcon a ete detruite, on pointe sur la nouvelle pour pouvoir afficher un dialogue juste apres.
			}
			else
			{
				cairo_dock_free_icon (pNewIcon);
				pNewIcon = NULL;
			}
			
			//\_______________________ on affiche un message.
			cairo_dock_remove_dialog_if_any (pConcernedIcon);  // on empeche la multiplication des dialogues de (de)montage.
			gboolean bIsMounted = FALSE;
			gchar *cUri = cairo_dock_fm_is_mounted (pConcernedIcon->cBaseURI, &bIsMounted);
			g_free (cUri);
			cairo_dock_show_temporary_dialog_with_icon_printf (
				bIsMounted ? D_("%s is now mounted") : D_("%s is now unmounted"),
				pConcernedIcon, pContainer,
				4000,
				"same icon",  // petit risque de n'avoir pas encore d'image a afficher, pas bien grave.
				pConcernedIcon->cName);
			if (! bIsMounted && pNewIcon == NULL)  // le disque s'est fait demonte, mais l'icone ne s'est pas faite remplacee.
			{
				CDDiskUsage *pDiskUsage = CD_APPLET_GET_MY_ICON_DATA (pConcernedIcon);
				if (pDiskUsage != NULL)
				{
					if (pDiskUsage->iTotal != 0)
					{
						pDiskUsage->iTotal = 0;
						pDiskUsage->iAvail = 0;
						cairo_dock_set_quick_info (pConcernedIcon, pContainer, NULL);  // on lui enleve son quick-info (ses infos n'etant plus valides, son quick-info ne sera pas mis a jour).
					}
				}
			}
		}
		break ;
	}
	g_free (cURI);
}

void cd_shortcuts_on_drive_event (CairoDockFMEventType iEventType, const gchar *cURI, CairoDockModuleInstance *myApplet)
{
	g_return_if_fail (cURI != NULL);
	CD_APPLET_ENTER;
	//\________________ On gere l'evenement sur le point de montage.
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	CD_APPLET_LEAVE_IF_FAIL (pContainer != NULL);
	
	_manage_event_on_drive (iEventType, cURI, pIconsList, pContainer, myApplet);
	
	//\________________ On met a jour les signets qui pointeraient sur un repertoire du point de montage nouvellement (de)monte.
	if (!myConfig.bListBookmarks || pIconsList == NULL)
	{
		CD_APPLET_LEAVE();
	}
	GList *ic;
	Icon *icon;
	gboolean bIsMounted;
	gchar *cTargetURI = cairo_dock_fm_is_mounted (cURI, &bIsMounted);
	if (cTargetURI == NULL)  // version bourrinne.
	{
		//g_print ("couldn't guess target URi of mount point '%s'\n", cURI);
		cd_shortcuts_on_bookmarks_event (CAIRO_DOCK_FILE_MODIFIED, NULL, myApplet);  // NULL <=> on recharge tout.
	}
	else  // version optimisee.
	{
		//g_print ("test bookmarks in '%s'...\n", cTargetURI);
		pIconsList = CD_APPLET_MY_ICONS_LIST;
		for (ic = pIconsList; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->iGroup == CD_BOOKMARK_GROUP)
			{
				if (strncmp (cTargetURI, icon->cBaseURI, strlen (cTargetURI)) == 0)
				{
					//g_print ("le signet '%s' est situe sur un point de montage ayant change (%s)\n", icon->cBaseURI, cTargetURI);
					gchar *cName = NULL, *cRealURI = NULL, *cIconName = NULL, *cUserName = NULL;
					int iVolumeID = 0;
					gboolean bIsDirectory = FALSE;
					double fOrder;
					if (cairo_dock_fm_get_file_info (icon->cBaseURI, &cName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, CAIRO_DOCK_FM_SORT_BY_NAME))
					{
						//g_print (" -> %s (%d)\n", cIconName, bIsMounted);
						g_free (icon->cName);
						if (bIsMounted || cIconName == NULL)
							icon->cName = cName;
						else
						{
							icon->cName = g_strdup_printf ("%s\n[%s]", cName, D_("Unmounted"));
							g_free (cName);
						}
						g_free (icon->cCommand);
						icon->cCommand = cRealURI;
						g_free (icon->cFileName);
						icon->cFileName = cIconName;
						icon->iVolumeID = iVolumeID;
						cairo_dock_load_icon_buffers (icon, pContainer);
					}
				}
			}
		}
		g_free (cTargetURI);
	}
	CD_APPLET_LEAVE();
}


GList * cd_shortcuts_list_drives (CDSharedMemory *pSharedMemory)
{
	GList *pIconList = NULL;
	gchar *cFullURI = NULL;
	
	//\_______________________ On recupere la liste des points de montage.
	pIconList = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_VFS_ROOT, CAIRO_DOCK_FM_SORT_BY_NAME, CD_DRIVE_GROUP, FALSE, 100, &cFullURI);
	cd_message ("  cFullURI : %s", cFullURI);
	if (pIconList == NULL)
	{
		cd_warning ("couldn't detect any drives");  // on decide de poursuivre malgre tout, pour les signets.
	}
	
	pSharedMemory->cDisksURI = cFullURI;
	
	//\_______________________ On initialise les usages disque.
	Icon *pIcon;
	GList *ic;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		_init_disk_usage (pIcon, pSharedMemory->pApplet);
	}
	
	return pIconList;
}
