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

void cd_shortcuts_on_bookmarks_event (CairoDockFMEventType iEventType, const gchar *cURI, GldiModuleInstance *myApplet)
{
	static int iTime = 0;
	iTime ++;
	CD_APPLET_ENTER;
	//g_print ("%s (%d)\n", __func__, iEventType);
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	Icon *icon;
	GList *ic;
	for (ic = pIconsList; ic != NULL; ic = ic->next)  // optimization: skip the disks and networks, and point on the first bookmark.
	{
		icon = ic->data;
		if (icon->iGroup == (CairoDockIconGroup) CD_BOOKMARK_GROUP)
			break;
	}
	pIconsList = ic;  // Note that since the first bookmark is always the Home Folder, 'pIconsList' will never change when inserting/removing a bookmark.
	GldiContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	CD_APPLET_LEAVE_IF_FAIL (pContainer != NULL);
	
	if (iEventType == CAIRO_DOCK_FILE_CREATED || iEventType == CAIRO_DOCK_FILE_MODIFIED)  // le fichier des bookmarks a ete modifie.
	{
		cd_message ("The bookmarks list has changed");
		
		//\____________________ On lit le fichier des signets.
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
			
			//\____________________ On parcourt le contenu.
			double fCurrentOrder = 1.;  // bookmarks are listed in the order of the file; we need to reorder each icon in case a bookmark has changed its place, or if a new one appeared (the first one is always the Home Folder).
			gchar *cOneBookmark;
			Icon *pNewIcon;
			gchar *cName, *cRealURI, *cIconName, *cUserName;
			gboolean bIsDirectory;
			int iVolumeID;
			double fOrder;
			int i;
			for (i = 0; cBookmarksList[i] != NULL; i ++)
			{
				cOneBookmark = cBookmarksList[i];
				if (*cOneBookmark == '\0' || *cOneBookmark == '#')
				{
					g_free (cOneBookmark);
					continue;
				}
				
				// on recupere le nom a afficher.
				cUserName = NULL;
				if (cOneBookmark != NULL && *cOneBookmark == '/')  // ne devrait pas arriver si on ajoute les signets via le dock ou Nautilus.
				{
					gchar *tmp = cOneBookmark;
					cOneBookmark = g_strconcat ("file://", cOneBookmark, NULL);  // sinon launch_uri() ne marche pas sous Gnome.
					g_free (tmp);
				}
				else  // c'est une URI valide, on regarde si il y'a un nom utilisateur.
				{
					gchar *str = strchr (cOneBookmark, ' ');  // pas d'espace dans une URI, donc le 1er espace signifie la separation entre URI et nom utilisateur.
					if (str != NULL)
					{
						cUserName = str + 1;
						*str = '\0';
					}
				}
				
				// on cree une icone pour le signet si aucune n'existe ou qu'il a change.
				Icon *pExistingIcon = cairo_dock_get_icon_with_base_uri (pIconsList, cOneBookmark);
				if (pExistingIcon != NULL)
				{
					if ((cUserName && cairo_dock_strings_differ (pExistingIcon->cName, cUserName))
					|| cURI == NULL)  // signet inexistant ou qui a change => on le cree. 'cUserName' may be NULL if the user has never set a user-name yet, but once he does, 'cUserName' is not NULL. so if 'cUserName' is NULL, it has not changed.
					{
						//g_print ("le signet '%s' a change, on le recree\n", pExistingIcon->cName);
						CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pExistingIcon);
						pExistingIcon = NULL;
					}
					else
					{
						CDDiskUsage *pDiskUsage = CD_APPLET_GET_MY_ICON_DATA (pExistingIcon);
						if (! pDiskUsage)
						{
							pDiskUsage = g_new0 (CDDiskUsage, 1);
							CD_APPLET_SET_MY_ICON_DATA (pExistingIcon, pDiskUsage);
						}
						pDiskUsage->iLastCheckTime = iTime;
						pExistingIcon->fOrder = fCurrentOrder ++;
					}
				}
				if (pExistingIcon == NULL)
				{
					//g_print ("new bookmark : '%s'\n", cOneBookmark);
					
					cName = NULL;
					cRealURI = NULL;
					cIconName = NULL;
					if (cairo_dock_fm_get_file_info (cOneBookmark, &cName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, CAIRO_DOCK_FM_SORT_BY_NAME))
					{
						cd_message (" + 1 bookmark : %s", cOneBookmark);
						if (cUserName != NULL)
						{
							g_free (cName);
							cName = g_strdup (cUserName);
						}
						else if (cName == NULL)  // cas d'un bookmark situe sur un volume non monte.
						{
							gchar *cGuessedName = g_path_get_basename (cOneBookmark);
							cairo_dock_remove_html_spaces (cGuessedName);
							cName = g_strdup_printf ("%s\n[%s]", cGuessedName, D_("Unmounted"));
							g_free (cGuessedName);
						}
						if (cRealURI == NULL)
							cRealURI = g_strdup (cOneBookmark);
						if (cIconName == NULL)
							cIconName = cairo_dock_search_icon_s_path (CD_SHORTCUT_DEFAULT_DIRECTORY_ICON_FILENAME, CAIRO_DOCK_DEFAULT_ICON_SIZE);
						
						pNewIcon = cairo_dock_create_dummy_launcher (cName,
							cIconName,
							cRealURI,
							NULL,
							fCurrentOrder++);
						pNewIcon->iGroup = CD_BOOKMARK_GROUP;
						pNewIcon->cBaseURI = cOneBookmark;
						pNewIcon->iVolumeID = iVolumeID;
						CDDiskUsage *pDiskUsage = g_new0 (CDDiskUsage, 1);
						pDiskUsage->iLastCheckTime = iTime;
						CD_APPLET_SET_MY_ICON_DATA (pNewIcon, pDiskUsage);
						
						CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pNewIcon);
					}
					else
					{
						cd_warning ("couldn't get info on bookmark '%s'", cOneBookmark);
						g_free (cOneBookmark);
					}
				}
			}
			g_free (cBookmarksList);
			
			//\____________________ remove the old bookmarks.
			///pIconsList = CD_APPLET_MY_ICONS_LIST;
			GList *next_ic;
			for (ic = pIconsList; ic != NULL; ic = next_ic)
			{
				next_ic = ic->next;
				icon = ic->data;
				if (icon->iGroup == (CairoDockIconGroup) CD_BOOKMARK_GROUP)
				{
					CDDiskUsage *pDiskUsage = CD_APPLET_GET_MY_ICON_DATA (icon);
					if (! pDiskUsage || pDiskUsage->iLastCheckTime < iTime)
					{
						cd_debug ("this bookmark is too old (%s)", icon->cName);
						CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (icon);
					}
				}
			}
			pIconsList = CD_APPLET_MY_ICONS_LIST;
			cairo_dock_sort_icons_by_order (pIconsList);  // again, since 'Home Folder' is always the first bookmark, the head of the list won't change even if there are only bookmarks (so we don't need to re-assigne it to the container).
		}
	}
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
			if ((str && strncmp (cOneBookmark, cURI, str - cOneBookmark) == 0) || (!str && strcmp (cOneBookmark, cURI) == 0))
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

static Icon * _cd_shortcuts_get_icon (gchar *cFileName, const gchar *cUserName, double fCurrentOrder)
{
	cd_debug ("New icon: %s, %s, %f", cFileName, cUserName, fCurrentOrder);
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
		if (pDiskUsage) pDiskUsage->iLastCheckTime = 1e9;  // so that this bookmark will never be considered old, and therefore removed.
		pBookmarkIconList = g_list_append (pBookmarkIconList, pNewIcon);
	}

	gchar *cContent = NULL;
	gsize length = 0;
	GError *erreur = NULL;
	g_file_get_contents  (cBookmarkFilePath, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s\n  no bookmark will be available", erreur->message);
		g_error_free (erreur);
	}
	else
	{
		gchar **cBookmarksList = g_strsplit (cContent, "\n", -1);
		g_free (cContent);
		
		gchar *cOneBookmark, *cUserName;
		int i = 0;
		for (i = 0; cBookmarksList[i] != NULL; i ++)
		{
			cOneBookmark = cBookmarksList[i];
			cUserName = NULL;
			if (cOneBookmark != NULL && *cOneBookmark == '/')  // ne devrait pas arriver si on ajoute les signets via le dock ou Nautilus.
			{
				gchar *tmp = g_strconcat ("file://", cOneBookmark, NULL);  // sinon launch_uri() ne marche pas sous Gnome.
				g_free (cOneBookmark);
				cOneBookmark = tmp;
			}
			else  // c'est une URI valide, on regarde si il y'a un nom utilisateur.
			{
				gchar *str = strchr (cOneBookmark, ' ');  // pas d'espace dans une URI, donc le 1er espace signifie la separation entre URI et nom utilisateur.
				if (str != NULL)
				{
					cUserName = str + 1;
					*str = '\0';
				}
			}
			if (*cOneBookmark != '\0' && *cOneBookmark != '#')
			{
				cd_message (" + 1 bookmark : %s", cOneBookmark);
				pNewIcon = _cd_shortcuts_get_icon (cOneBookmark, cUserName, fCurrentOrder++);
				if (pNewIcon)
					pBookmarkIconList = g_list_append (pBookmarkIconList, pNewIcon);
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
