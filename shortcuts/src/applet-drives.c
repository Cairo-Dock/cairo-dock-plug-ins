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

#define CD_SHORTCUT_DEFAULT_DRIVE_ICON_FILENAME "drive-harddisk"


void cd_shortcuts_add_progress_bar (Icon *pIcon, GldiModuleInstance *myApplet)
{
	// set a progress bar to display the disk space
	CairoProgressBarAttribute attr;
	memset (&attr, 0, sizeof (CairoProgressBarAttribute));
	if (myConfig.iDisplayType == CD_SHOW_USED_SPACE || myConfig.iDisplayType == CD_SHOW_USED_SPACE_PERCENT)
	{
		attr.bInverted = TRUE;
	}
	CairoDataRendererAttribute *pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
	pRenderAttr->cModelName = "progressbar";
	cairo_dock_add_new_data_renderer_on_icon (pIcon, pIcon->pContainer, pRenderAttr);
}


void _init_disk_usage (Icon *pIcon, GldiModuleInstance *myApplet)
{
	// ensure the applet has a valid icon, in case the VFS didn't give us one.
	if (pIcon->cFileName == NULL)
		pIcon->cFileName = cairo_dock_search_icon_s_path (CD_SHORTCUT_DEFAULT_DRIVE_ICON_FILENAME, CAIRO_DOCK_DEFAULT_ICON_SIZE);
		
	if (pIcon->cCommand)
	{
		// set our private data for disk usage
		CDDiskUsage *pDiskUsage = g_new0 (CDDiskUsage, 1);
		pDiskUsage->iPrevAvail = -1;  // data not yet retrieved (0 bytes is a valid value for non writable disks).
		CD_APPLET_SET_MY_ICON_DATA (pIcon, pDiskUsage);
		
		// get the current disk usage
		cd_shortcuts_get_fs_stat (pIcon->cCommand, pDiskUsage);
	}
}

static void _manage_event_on_drive (CairoDockFMEventType iEventType, const gchar *cBaseURI, GList *pIconsList, GldiContainer *pContainer, GldiModuleInstance *myApplet)
{
	gchar *cURI = (g_strdup (cBaseURI));
	cairo_dock_remove_html_spaces (cURI);
	cd_debug (" * event %d on '%s'", iEventType, cURI);
	
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_DELETED :  // a mount point has been disconnected
		{
			Icon *pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIconsList, cURI);
			if (pConcernedIcon == NULL)  // search by name
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
			g_free (myData.cLastDeletedUri);
			myData.cLastDeletedUri = g_strdup (cURI);
		}
		break ;
		
		case CAIRO_DOCK_FILE_CREATED :  // a mount point has been connected
		{
			//\_______________________does it already exist?
			Icon *pSameIcon = cairo_dock_get_icon_with_base_uri (pIconsList, cURI);
			if (pSameIcon != NULL)
			{
				cd_warning ("this mount point (%s) already exists.", pSameIcon->cName);
				return;  // do nothing, certainly an useless/double signal
			}
			
			//\_______________________ create a new icon
			Icon *pNewIcon = cairo_dock_fm_create_icon_from_URI (cURI, pContainer, CAIRO_DOCK_FM_SORT_BY_NAME);
			if (pNewIcon == NULL)
			{
				cd_warning ("couldn't create an icon for this mount point");
				return ;
			}
			pNewIcon->iGroup = CD_DRIVE_GROUP;
			
			//\_______________________ place it at the right position (by name)
			cd_shortcuts_set_icon_order_by_name (pNewIcon, pIconsList);
			cd_debug (" new drive : %s, %s", pNewIcon->cName, pNewIcon->cCommand);
			
			//\_______________________ added in the list
			CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNewIcon);
			_init_disk_usage (pNewIcon, myApplet);
			if (pNewIcon->cCommand)
			{
				cd_shortcuts_add_progress_bar (pNewIcon, myApplet);
				cd_shortcuts_display_disk_usage (pNewIcon, myApplet);
			}
			
			//\_______________________ display a notification
			gboolean bIsMounted = FALSE;
			gchar *cUri = cairo_dock_fm_is_mounted (pNewIcon->cBaseURI, &bIsMounted);
			g_free (cUri);
			gldi_dialog_show_temporary_with_icon_printf (
				bIsMounted ? D_("%s is now mounted") : D_("%s has been connected"),
				pNewIcon, pContainer,
				4000,
				NULL,  // it's icon is not already loaded
				pNewIcon->cName);
			g_free (myData.cLastCreatedUri);
			myData.cLastCreatedUri = g_strdup (cURI);
		}
		break ;
		
		case CAIRO_DOCK_FILE_MODIFIED :  // a mount point has been (un)mounted
		{
			//\_______________________ search the right icon
			Icon *pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIconsList, cURI);
			if (pConcernedIcon == NULL)  // search by using the name
			{
				pConcernedIcon = cairo_dock_get_icon_with_name (pIconsList, cURI);
			}
			if (pConcernedIcon == NULL)
			{
				cd_warning ("  an unknown mount point was modified");
				return ;
			}
			cd_debug (" %s is modified (%s)", pConcernedIcon->cName, pConcernedIcon->cCommand);
			
			//\_______________________ grab current info
			Icon *pNewIcon = cairo_dock_fm_create_icon_from_URI (cURI, pContainer, CAIRO_DOCK_FM_SORT_BY_NAME);
			if (pNewIcon == NULL)
			{
				cd_warning ("couldn't create an icon for this mount point");
				return ;
			}
			pNewIcon->iGroup = CD_DRIVE_GROUP;
			
			//\_______________________ replace the icon if smthg has changed
			if (cairo_dock_strings_differ (pConcernedIcon->cName, pNewIcon->cName)
			    || cairo_dock_strings_differ (pConcernedIcon->cFileName, pNewIcon->cFileName))
			{
				//g_print (" '%s' -> '%s'\n'%s' -> '%s'\n", pConcernedIcon->cName, pNewIcon->cName, pConcernedIcon->cFileName, pNewIcon->cFileName);
				
				CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pConcernedIcon);
				pIconsList = CD_APPLET_MY_ICONS_LIST;
				
				cd_shortcuts_set_icon_order_by_name (pNewIcon, pIconsList);
				CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNewIcon);
				_init_disk_usage (pNewIcon, myApplet);
				if (pNewIcon->cCommand)
				{
					cd_shortcuts_add_progress_bar (pNewIcon, myApplet);
				}

				// pConcernedIcon has been distroyed, assigned the new one to display a dialogue just after
				pConcernedIcon = pNewIcon;
			}
			else
			{
				gldi_object_unref (GLDI_OBJECT (pNewIcon));
				pNewIcon = NULL;
			}
			
			cd_shortcuts_display_disk_usage (pConcernedIcon, myApplet);
			
			//\_______________________ display a notification
			gldi_dialogs_remove_on_icon (pConcernedIcon); // avoid multiple dialogues
			gboolean bIsMounted = FALSE;
			gchar *cUri = cairo_dock_fm_is_mounted (pConcernedIcon->cBaseURI, &bIsMounted);
			g_free (cUri);
			gldi_dialog_show_temporary_with_icon_printf (
				bIsMounted ? D_("%s is now mounted") : D_("%s is now unmounted"),
				pConcernedIcon, pContainer,
				4000,
				"same icon",  // it's possible to not have the right icon, not so important
				pConcernedIcon->cName);
			// disk has been unmounted but a icon is needed
			if (! bIsMounted && pNewIcon == NULL)
			{
				CDDiskUsage *pDiskUsage = CD_APPLET_GET_MY_ICON_DATA (pConcernedIcon);
				if (pDiskUsage != NULL)
				{
					if (pDiskUsage->iTotal != 0)
					{
						pDiskUsage->iTotal = 0;
						pDiskUsage->iAvail = 0;
						// remove the quick-info because it has been unmounted
						gldi_icon_set_quick_info (pConcernedIcon, NULL);
					}
				}
			}
		}
		break ;
		case CAIRO_DOCK_NB_EVENT_ON_FILES :
		break ;
	}
	g_free (cURI);
}

void cd_shortcuts_on_drive_event (CairoDockFMEventType iEventType, const gchar *cURI, GldiModuleInstance *myApplet)
{
	g_return_if_fail (cURI != NULL);
	CD_APPLET_ENTER;
	//\________________ Manage event about this mount point
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	GldiContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	CD_APPLET_LEAVE_IF_FAIL (pContainer != NULL);
	
	_manage_event_on_drive (iEventType, cURI, pIconsList, pContainer, myApplet);
	
	//\________________ Update bookmarks which are linked to this mount point
	if (!myConfig.bListBookmarks || pIconsList == NULL)
	{
		CD_APPLET_LEAVE();
	}
	GList *ic;
	Icon *icon;
	gboolean bIsMounted;
	gchar *cTargetURI = cairo_dock_fm_is_mounted (cURI, &bIsMounted);
	if (cTargetURI != NULL)  // optimized version.
	{
		//g_print ("test bookmarks in '%s'...\n", cTargetURI);
		pIconsList = CD_APPLET_MY_ICONS_LIST;
		for (ic = pIconsList; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->iGroup == (CairoDockIconGroup) CD_BOOKMARK_GROUP)
			{
				if (strncmp (cTargetURI, icon->cBaseURI, strlen (cTargetURI)) == 0)
				{
					//g_print ("bookmark '%s' is located in a mount point which has changed (%s)\n", icon->cBaseURI, cTargetURI);
					gchar *cName = NULL, *cRealURI = NULL, *cIconName = NULL;
					int iVolumeID = 0;
					gboolean bIsDirectory = FALSE;
					double fOrder;
					if (cairo_dock_fm_get_file_info (icon->cBaseURI, &cName,
					     &cRealURI, &cIconName, &bIsDirectory, &iVolumeID,
					     &fOrder, CAIRO_DOCK_FM_SORT_BY_NAME))
					{
						//g_print (" -> %s (%d)\n", cIconName, bIsMounted);
						if (bIsMounted/** || cIconName == NULL*/)
						{
							gchar *str;
							/* if it was previously an unmounted bookmark, just
							 * remove the 'unmounted' part to avoid changing the
							 * name (when mounted, gvfs returns the path whereas
							 * we want to display the bookmark name). Note that
							 * the icon might also changes (it was NULL when the
							 * bookmark was not mounted), and we can use this
							 * new one which is probably more accurate.
							 */
							if ((str = strchr (icon->cName, '\n')) != NULL)
								*str = '\0';
							else
							{
								g_free (icon->cName);
								icon->cName = cName;
							}
						}
						else
						{
							g_free (icon->cName);
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
	
	//\_______________________ Get the list of mount points.
	pIconList = cairo_dock_fm_list_directory (CAIRO_DOCK_FM_VFS_ROOT,
		CAIRO_DOCK_FM_SORT_BY_NAME, CD_DRIVE_GROUP, FALSE, 100, &cFullURI);
	cd_message ("  cFullURI : %s", cFullURI);
	if (pIconList == NULL)
	{
		cd_warning ("couldn't detect any drives");  // continue: for bookmarks
	}
	/// TODO: if ! bListBookmarks, then we should add the Home in the drives list, to have the disk space information...
	
	pSharedMemory->cDisksURI = cFullURI;
	
	//\_______________________ Initialize disk usages.
	Icon *pIcon;
	GList *ic;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		_init_disk_usage (pIcon, pSharedMemory->pApplet);
	}
	
	return pIconList;
}
