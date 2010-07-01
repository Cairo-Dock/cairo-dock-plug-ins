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


static void cd_shortcuts_on_change_drives (CairoDockFMEventType iEventType, const gchar *cURI, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	//\________________ On gere l'evenement sur le point de montage.
	cd_shortcuts_stop_disk_periodic_task (myApplet);
	
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 6, CAIRO_DOCK_FM_SORT_BY_NAME);
	
	cd_shortcuts_launch_disk_periodic_task (myApplet);
	
	//\________________ On met a jour les signets qui pointeraient sur un repertoire du point de montage nouvellement (de)monte.
	if (!myConfig.bListBookmarks)
	{
		CD_APPLET_LEAVE();
		//return;
	}
	if (! myIcon->pSubDock && (!myDesklet || !myDesklet->icons))
	{
		CD_APPLET_LEAVE();
		//return;
	}
	GList *ic;
	Icon *icon;
	gboolean bIsMounted;
	gchar *cTargetURI = cairo_dock_fm_is_mounted (cURI, &bIsMounted);
	if (cTargetURI == NULL)  // version bourrinne.
	{
		cd_shortcuts_on_change_bookmarks (CAIRO_DOCK_FILE_MODIFIED, NULL, myApplet);  // NULL <=> on recharge tout.
	}
	else  // version optimisee.
	{
		for (ic = (myDock ? myIcon->pSubDock->icons : myDesklet->icons); ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->iType == 10)
			{
				if (strncmp (cTargetURI, icon->cBaseURI, strlen (cTargetURI)) == 0)
				{
					cd_message ("le signet %s est situe sur un point de montage ayant change (%s)", icon->cBaseURI, cTargetURI);
					gchar *cName = NULL, *cRealURI = NULL, *cIconName = NULL, *cUserName = NULL;
					int iVolumeID = 0;
					gboolean bIsDirectory = FALSE;
					double fOrder;
					if (cairo_dock_fm_get_file_info (icon->cBaseURI, &cName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, CAIRO_DOCK_FM_SORT_BY_NAME))
					{
						cd_debug (" -> %s (%d)\n", cIconName, bIsMounted);
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
						cairo_dock_load_icon_buffers (icon, (myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer));
					}
				}
			}
		}
		g_free (cTargetURI);
	}
	CD_APPLET_LEAVE();
}
static void cd_shortcuts_on_change_network (CairoDockFMEventType iEventType, const gchar *cURI, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, myIcon, 8, CAIRO_DOCK_FM_SORT_BY_NAME);
	CD_APPLET_LEAVE();
}


static GList * _load_icons (CairoDockModuleInstance *myApplet)
{
	GList *pIconList = NULL;
	gchar *cFullURI = NULL;
	
	if (myConfig.bListDrives)
	{
		pIconList = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_VFS_ROOT, CAIRO_DOCK_FM_SORT_BY_NAME, 6, FALSE, 100, &cFullURI);
		cd_message ("  cFullURI : %s", cFullURI);
		if (pIconList == NULL)
		{
			cd_warning ("couldn't detect any drives");  // on decide de poursuivre malgre tout, pour les signets.
		}
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, TRUE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_drives, myApplet))
			cd_warning ("Shortcuts : can't monitor drives");
		myData.cDisksURI = cFullURI;
	}
	
	if (myConfig.bListNetwork)
	{
		GList *pIconList2 = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_NETWORK, CAIRO_DOCK_FM_SORT_BY_NAME, 8, FALSE, 100, &cFullURI);
		cd_message ("  cFullURI : %s", cFullURI);
		
		if (myConfig.bUseSeparator && myDock && pIconList2 != NULL && pIconList != NULL)
		{
			Icon *pSeparatorIcon = cairo_dock_create_separator_icon (7, NULL);  // NULL => ne charge pas l'icone, car on est dans un thread.
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cFullURI, TRUE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_network, myApplet))
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
		
		if (myConfig.bUseSeparator && myDock && pIconList2 != NULL && pIconList != NULL)
		{
			Icon *pSeparatorIcon = cairo_dock_create_separator_icon (9, NULL);  // NULL => ne charge pas l'icone, car on est dans un thread.
			pIconList = g_list_append (pIconList, pSeparatorIcon);
		}
		
		pIconList = g_list_concat (pIconList, pIconList2);
		
		if (! cairo_dock_fm_add_monitor_full (cBookmarkFilePath, FALSE, NULL, (CairoDockFMMonitorCallback) cd_shortcuts_on_change_bookmarks, myApplet))
			cd_warning ("Shortcuts : can't monitor bookmarks");
		
		myData.cBookmarksURI = cBookmarkFilePath;
	}
	
	return pIconList;
}


void cd_shortcuts_get_shortcuts_data (CairoDockModuleInstance *myApplet)
{
	myData.pIconList = _load_icons (myApplet);
}


static gboolean _launch_disk_periodic_task (CairoDockModuleInstance *myApplet)
{
	cd_shortcuts_launch_disk_periodic_task (myApplet);
	myData.iSidLaunchTask = 0;
	return FALSE;
}
gboolean cd_shortcuts_build_shortcuts_from_data (CairoDockModuleInstance *myApplet)
{
	g_return_val_if_fail (myIcon != NULL, FALSE);  // paranoia
	/*if (myIcon == NULL)
	{
		cd_debug ("annulation du chargement des raccourcis\n");
		g_list_foreach (myData.pIconList, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (myData.pIconList);
		myData.pIconList = NULL;
		return FALSE;
	}*/
	
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
	if (myData.iSidLaunchTask == 0)  // on la lance en idle, car les icones sont chargees en idle.
		myData.iSidLaunchTask = g_idle_add ((GSourceFunc)_launch_disk_periodic_task, myApplet);
	
	return TRUE;
}

