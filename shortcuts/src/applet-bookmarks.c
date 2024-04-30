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
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-drives.h"
#include "applet-bookmarks.h"

#define CD_SHORTCUT_DEFAULT_DIRECTORY_ICON_FILENAME "inode-directory"

static const gchar * _get_custom_name_and_uri (gchar *cOneBookmark, gchar **cURI)
{
	const gchar *cUserName = NULL;
	// should not happen if we add bookmarks via the dock or Nautilus
	if (*cOneBookmark == '/')
	{
		// for 'gvfs_launch_uri':
		*cURI = g_strconcat ("file://", cOneBookmark, NULL);
		g_free (cOneBookmark);
	}
	else  // it's a valid URI but does it have a custom name?
	{
		*cURI = cOneBookmark;
		// a custom name is separated with a whitespace (no whitespace in the URI)
		gchar *str = strchr (cOneBookmark, ' ');
		if (str != NULL)
		{
			cUserName = str + 1;
			*str = '\0';
		}
	}
	return cUserName;
}

static Icon * _cd_shortcuts_get_icon (gchar *cFileName, const gchar *cUserName, double fCurrentOrder)
{
	cd_debug ("New icon: %s, %s, %f", cFileName, cUserName, fCurrentOrder);

	/* Nautilus adds custom prefixes which are not supported by gvfs...
	 * gvfs-integration plugin can read x-nautilus-desktop but not others, e.g.:
	 * x-nautilus-search://0/ => specific to Nautilus: open these URI with it.
	 * Note that all these URI have a user-name
	 */
	if (g_str_has_prefix (cFileName, "x-nautilus-")
	    && ! g_str_has_prefix (cFileName, "x-nautilus-desktop://"))
	{
		Icon *pNewIcon = cairo_dock_create_dummy_launcher (
			cUserName ? g_strdup (cUserName) : g_strdup (cFileName),
			cairo_dock_search_icon_s_path (
				CD_SHORTCUT_DEFAULT_DIRECTORY_ICON_FILENAME,
				CAIRO_DOCK_DEFAULT_ICON_SIZE),
			g_strdup_printf ("nautilus %s", cFileName),
			NULL,
			fCurrentOrder);
		pNewIcon->iGroup = CD_BOOKMARK_GROUP;
		pNewIcon->cBaseURI = cFileName;
		pNewIcon->iVolumeID = CD_VOLUME_ID_BOOKMARK_CMD;
		return pNewIcon;
	}

	gchar *cName, *cRealURI, *cIconName;
	gboolean bIsDirectory;
	gint iVolumeID;
	gdouble fOrder;
	if (! cairo_dock_fm_get_file_info (cFileName, &cName, &cRealURI, &cIconName,
		&bIsDirectory, &iVolumeID, &fOrder, CAIRO_DOCK_FM_SORT_BY_NAME))
		return NULL;
	if (cUserName != NULL)
	{
		g_free (cName);
		if (cName == NULL)  // a bookmark on a unmounted system or a folder that doesn't exist any more
			cName = g_strdup_printf ("%s\n[%s]", cUserName, D_("Unmounted"));
		else
			cName = g_strdup (cUserName);
	}
	else if (cName == NULL)  // a bookmark on a unmounted system
	{
		gchar *cGuessedName = g_path_get_basename (cFileName);
		cairo_dock_remove_html_spaces (cGuessedName); // or: g_uri_unescape_string
		cName = g_strdup_printf ("%s\n[%s]", cGuessedName, D_("Unmounted"));
		g_free (cGuessedName);
	}
	if (cRealURI == NULL)
		cRealURI = g_strdup (cFileName);
	if (cIconName == NULL)
		cIconName = cairo_dock_search_icon_s_path (
			CD_SHORTCUT_DEFAULT_DIRECTORY_ICON_FILENAME,
			CAIRO_DOCK_DEFAULT_ICON_SIZE); // should be the default icon

	Icon *pNewIcon = cairo_dock_create_dummy_launcher (cName,
		cIconName,
		cRealURI,
		NULL,
		fCurrentOrder);
	pNewIcon->iGroup = CD_BOOKMARK_GROUP;
	pNewIcon->cBaseURI = cFileName;
	pNewIcon->iVolumeID = iVolumeID;
	return pNewIcon;
}

static GList * _get_item_with_base_uri_icon (GList *pIconList, const gchar *cBaseURI)
{
	GList* ic;
	Icon *pIcon;
	for (ic = pIconList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->cBaseURI != NULL && strcmp (pIcon->cBaseURI, cBaseURI) == 0)
			return ic;
	}
	return NULL;
}

static void _remove_old_icons_and_free_list (GList *pOldBookmarkList)
{
	GList* ic;
	Icon *pIcon;
	for (ic = pOldBookmarkList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pIcon);
	}
	g_list_free (pOldBookmarkList);
}

void cd_shortcuts_on_bookmarks_event (CairoDockFMEventType iEventType, const gchar *cURI, GldiModuleInstance *myApplet)
{
	static int iTime = 0;
	iTime ++;
	CD_APPLET_ENTER;
	//g_print ("%s (%d)\n", __func__, iEventType);
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	GList *pOldBookmarkList;
	Icon *icon;
	GList *ic;
	double fCurrentOrder = 1.;
	// optimization: skip the disks and networks, and point on the first bookmark.
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		icon->fOrder = fCurrentOrder++;
		if (icon->iGroup == (CairoDockIconGroup) CD_BOOKMARK_GROUP)
			break;
	}
	/* Note that since the first bookmark is always the Home Folder,
	 * 'pIconsList' will never change when inserting/removing a bookmark.
	 */
	GldiContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	CD_APPLET_LEAVE_IF_FAIL (pContainer != NULL);

	// make a copy of a sublist that can be manipulated independently
	// of pIconsList which is part of our subdock's icons
	pOldBookmarkList = ic->next;
	if (pOldBookmarkList)
	{
		pOldBookmarkList->prev = NULL;
		pOldBookmarkList = g_list_copy (pOldBookmarkList);
		ic->next->prev = ic;
	}

	// Bookmarks file has been modified
	if (iEventType == CAIRO_DOCK_FILE_CREATED || iEventType == CAIRO_DOCK_FILE_MODIFIED)
	{
		cd_message ("The bookmarks list has changed");
		
		//\____________________ Read bookmarks file
		gchar *cContent = NULL;
		gsize length=0;
		GError *erreur = NULL;
		g_file_get_contents  (myData.cBookmarksURI, &cContent, &length, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("when trying to get the bookmarks : %s", erreur->message);
			g_error_free (erreur);
		}
		else
		{
			gchar **cBookmarksList = g_strsplit (cContent, "\n", -1);
			g_free (cContent);
			
			//\____________________ Read the content.
			/* Bookmarks are listed in the order of the file; we need to
			 * reorder each icon in case a bookmark has changed its place, or
			 * if a new one appeared (the first one is always the Home Folder).
			 */
			gchar *cOneBookmark;
			Icon *pNewIcon, *pExistingIcon;
			GList *pExistingIconNode;
			const gchar *cUserName;
			int i;
			for (i = 0; cBookmarksList[i] != NULL; i ++)
			{
				cOneBookmark = cBookmarksList[i];
				if (*cOneBookmark == '\0' || *cOneBookmark == '#')
				{
					g_free (cOneBookmark);
					continue;
				}
				
				// Grab the custom name if any
				cUserName = _get_custom_name_and_uri (cBookmarksList[i], &cOneBookmark);
				
				// Check if the icon already exists and has changed
				pExistingIconNode = _get_item_with_base_uri_icon (pOldBookmarkList, cOneBookmark);
				if (pExistingIconNode != NULL)
				{
					pExistingIcon = pExistingIconNode->data;
					// move this node to the subdock icons list
					pOldBookmarkList = g_list_delete_link (pOldBookmarkList, pExistingIconNode);
					if (cUserName && g_strcmp0 (pExistingIcon->cName, cUserName) != 0)
					{
						CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pExistingIcon); // will destroy it
						pExistingIcon = NULL;
					}
					else
					{
						pExistingIcon->fOrder = fCurrentOrder++;
						g_free (cOneBookmark);
					}
				}
				else
					pExistingIcon = NULL;

				if (pExistingIcon == NULL)
				{
					pNewIcon = _cd_shortcuts_get_icon (cOneBookmark,
						cUserName, fCurrentOrder);
					if (pNewIcon)
					{
						CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNewIcon);
						fCurrentOrder++;
					}
					else
					{
						cd_warning ("couldn't get info on bookmark '%s'", cOneBookmark);
						g_free (cOneBookmark);
					}
				}
			}
			g_free (cBookmarksList);

			/* Again, since 'Home Folder' is always the first bookmark,
			 * the head of the list won't change even if there are only bookmarks
			 * (so we don't need to re-assigne it to the container).
			 */
			cairo_dock_sort_icons_by_order (pIconsList);
		}
	}
	_remove_old_icons_and_free_list (pOldBookmarkList);
	CD_APPLET_LEAVE();
}

void cd_shortcuts_remove_one_bookmark (const gchar *cURI, GldiModuleInstance *myApplet)
{
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s)", __func__, cURI);
	
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents  (myData.cBookmarksURI, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("while trying to read bookmarks file : %s", erreur->message);
		g_error_free (erreur);
	}
	else
	{
		gchar **cBookmarksList = g_strsplit (cContent, "\n", -1);
		g_free (cContent);
		gchar *cOneBookmark, *str;
		gboolean bFound = FALSE;
		int i = 0;
		for (i = 0; cBookmarksList[i] != NULL; i ++)
		{
			cOneBookmark = cBookmarksList[i];
			if (*cOneBookmark == '\0' || *cOneBookmark == '#')
				continue;
			
			str = strchr (cOneBookmark, ' ');
			if ((str && strncmp (cOneBookmark, cURI, str - cOneBookmark) == 0)
			    || (!str && strcmp (cOneBookmark, cURI) == 0))
			{
				// remove this element from the array
				int j;
				for (j = i; cBookmarksList[j] != NULL; j ++)
				{
					cBookmarksList[j] = cBookmarksList[j+1];
				}
				// free the removed element.
				g_free (cOneBookmark);
				// quit the loop
				bFound = TRUE;
				break;
			}
		}
		
		if (! bFound)
		{
			cd_warning ("bookmark '%s' not found", cURI);
		}
		else
		{
			cContent = g_strjoinv ("\n", cBookmarksList);
			g_file_set_contents (myData.cBookmarksURI, cContent, -1, &erreur);
			if (erreur != NULL)
			{
				cd_warning ("while trying to write bookmarks file : %s", erreur->message);
				g_error_free (erreur);
			}
			g_free (cContent);
		}
		g_strfreev (cBookmarksList);
	}
}

void cd_shortcuts_rename_one_bookmark (const gchar *cURI, const gchar *cName, GldiModuleInstance *myApplet)
{
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s, %s)", __func__, cURI, cName);
	
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents  (myData.cBookmarksURI, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("while trying to read bookmarks file : %s", erreur->message);
		g_error_free (erreur);
	}
	else
	{
		gchar **cBookmarksList = g_strsplit (cContent, "\n", -1);
		g_free (cContent);
		gchar *cOneBookmark, *str;
		int i = 0;
		for (i = 0; cBookmarksList[i] != NULL; i ++)
		{
			cOneBookmark = cBookmarksList[i];
			if (*cOneBookmark == '\0' || *cOneBookmark == '#')
				continue;
			
			str = strchr (cOneBookmark, ' ');
			if ((str && strncmp (cOneBookmark, cURI, str - cOneBookmark) == 0) || (!str && strcmp (cOneBookmark, cURI) == 0))
			{
				cBookmarksList[i] = g_strdup_printf ("%s %s", cURI, cName);
				g_free (cOneBookmark);
				break;
			}
		}
		
		if (cBookmarksList[i] == NULL)
		{
			cd_warning ("bookmark '%s' not found", cURI);
		}
		else
		{
			cContent = g_strjoinv ("\n", cBookmarksList);
			g_file_set_contents (myData.cBookmarksURI, cContent, -1, &erreur);
			if (erreur != NULL)
			{
				cd_warning ("while trying to write bookmarks file : %s", erreur->message);
				g_error_free (erreur);
			}
			g_free (cContent);
		}
		g_strfreev (cBookmarksList);
	}
}

void cd_shortcuts_add_one_bookmark (const gchar *cURI, GldiModuleInstance *myApplet)
{
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s)", __func__, cURI);
	
	// see if we need to add a new line before the new URI.
	gchar *cContent = NULL;
	gsize length = 0;
	g_file_get_contents (myData.cBookmarksURI,
		&cContent,
		&length,
		NULL);
	gboolean bAddNewLine = (cContent && length > 0 && cContent[length-1] != '\n');
	g_free (cContent);
	
	// append the new URI to the file.
	FILE *f = fopen (myData.cBookmarksURI, "a");
	if (f != NULL)
	{
		gchar *cNewLine = g_strdup_printf ("%s%s\n", bAddNewLine ? "\n" : "", cURI);
		fputs(cNewLine, f);
		g_free (cNewLine);
		fclose (f);
	}
}

GList *cd_shortcuts_list_bookmarks (gchar *cBookmarkFilePath, GldiModuleInstance *myApplet)
{
	GList *pBookmarkIconList = NULL;
	Icon *pNewIcon;
	double fCurrentOrder = 0.;

	// Home
	gchar *cHome = g_strdup_printf ("file://%s", g_getenv ("HOME"));
	pNewIcon = _cd_shortcuts_get_icon (cHome, D_("Home Folder"), fCurrentOrder++);
	if (pNewIcon != NULL)
	{
		_init_disk_usage (pNewIcon, myApplet);
		CDDiskUsage *pDiskUsage = CD_APPLET_GET_MY_ICON_DATA (pNewIcon);
		if (pDiskUsage) // so that this bookmark will never be considered old, and therefore removed.
			pDiskUsage->iLastCheckTime = 1e9;
		pBookmarkIconList = g_list_append (pBookmarkIconList, pNewIcon);
	}
	else
		g_free (cHome);

	gchar *cContent = NULL;
	gsize length = 0;
	GError *erreur = NULL;
	g_file_get_contents  (cBookmarkFilePath, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention: %s\n  no bookmark will be available", erreur->message);
		g_error_free (erreur);
	}
	else
	{
		gchar **cBookmarksList = g_strsplit (cContent, "\n", -1);
		g_free (cContent);
		
		gchar *cOneBookmark;
		const gchar *cUserName;
		int i = 0;
		for (i = 0; cBookmarksList[i] != NULL; i ++)
		{
			cUserName = _get_custom_name_and_uri (cBookmarksList[i], &cOneBookmark);
			if (*cOneBookmark != '\0' && *cOneBookmark != '#')
			{
				cd_message (" + 1 bookmark : %s", cOneBookmark);
				pNewIcon = _cd_shortcuts_get_icon (cOneBookmark, cUserName, fCurrentOrder++);
				if (pNewIcon)
					pBookmarkIconList = g_list_append (pBookmarkIconList, pNewIcon);
				else
					g_free (cOneBookmark);
			}
			else
			{
				g_free (cOneBookmark);
			}
		}
		g_free (cBookmarksList);
	}
	return pBookmarkIconList;
}
