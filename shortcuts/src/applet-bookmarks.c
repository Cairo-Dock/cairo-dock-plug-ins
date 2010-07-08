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
#include "applet-bookmarks.h"


static void _cd_shortcuts_mark_one_bookmark (Icon *icon, gpointer unused, int *pTime)
{
	icon->iLastCheckTime = *pTime;
}
void cd_shortcuts_on_bookmarks_event (CairoDockFMEventType iEventType, const gchar *cURI, CairoDockModuleInstance *myApplet)
{
	static int iTime = 0;
	iTime ++;
	CD_APPLET_ENTER;
	g_print ("%s (%d)\n", __func__, iEventType);
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	CD_APPLET_LEAVE_IF_FAIL (pContainer != NULL);
	
	if (iEventType == CAIRO_DOCK_FILE_CREATED || iEventType == CAIRO_DOCK_FILE_MODIFIED)  // le fichier des bookmarks a ete modifie.
	{
		cd_message ("  un signet en plus ou en moins");
		
		//\____________________ On lit le fichier des signets.
		gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
		gchar *cContent = NULL;
		gsize length=0;
		GError *erreur = NULL;
		g_file_get_contents  (cBookmarkFilePath, &cContent, &length, &erreur);
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
			gchar *cOneBookmark;
			Icon *pNewIcon;
			gchar *cName, *cRealURI, *cIconName, *cUserName;
			gboolean bIsDirectory;
			int iVolumeID;
			double fOrder, fCurrentOrder = 0;
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
					if (cairo_dock_strings_differ (pExistingIcon->cName, cUserName) || cURI == NULL)  // signet inexistant ou qui a change => on le cree.
					{
						g_print ("le signet '%s' a change, on le recree\n", pExistingIcon->cName);
						CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pExistingIcon);
						pExistingIcon = NULL;
					}
					else
						pExistingIcon->iLastCheckTime = iTime;
				}
				if (pExistingIcon == NULL)
				{
					g_print ("new bookmark : '%s'\n", cOneBookmark);
					
					cName = NULL;
					cRealURI = NULL;
					cIconName = NULL;
					if (cairo_dock_fm_get_file_info (cOneBookmark, &cName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, CAIRO_DOCK_FM_SORT_BY_NAME))
					{
						cd_message (" + 1 signet : %s", cOneBookmark);
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
							cRealURI = g_strdup ("none");
						
						pNewIcon = cairo_dock_create_dummy_launcher (cName,
							cIconName,
							cRealURI,
							NULL,
							0);
						pNewIcon->iType = CD_BOOKMARK_GROUP;
						pNewIcon->cBaseURI = cOneBookmark;
						pNewIcon->iVolumeID = iVolumeID;
						pNewIcon->iLastCheckTime = iTime;
						
						pIconsList = CD_APPLET_MY_ICONS_LIST;
						cd_shortcuts_set_icon_order_by_name (pNewIcon, pIconsList);
						
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
			
			//\____________________ On supprime les vieux signets.
			pIconsList = CD_APPLET_MY_ICONS_LIST;
			gboolean bRemove = TRUE;
			Icon *icon;
			GList *ic;
			while (bRemove)
			{
				bRemove = FALSE;
				for (ic = pIconsList; ic != NULL; ic = ic->next)
				{
					icon = ic->data;
					if (icon->iType == CD_BOOKMARK_GROUP)
					{
						if (icon->iLastCheckTime != iTime)
						{
							g_print ("this bookmark is too old (%s)\n", icon->cName);
							CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (icon);
							bRemove = TRUE;
							break;
						}
					}
				}
			}
			
			//\____________________ On ajoute ou supprime un separateur.
			/**if (myDock)
			{
				Icon *pFirstBookmarkIcon = cairo_dock_get_first_icon_of_type (myIcon->pSubDock->icons, 10);
				if (pFirstBookmarkIcon == NULL && pSeparatorIcon != NULL)
				{
					cd_message ("on enleve l'ancien separateur");
					cairo_dock_detach_icon_from_dock (pSeparatorIcon, myIcon->pSubDock, myConfig.bUseSeparator);
					cairo_dock_free_icon (pSeparatorIcon);
				}
			}*/
		}
		g_free (cBookmarkFilePath);
	}
	CD_APPLET_LEAVE();
}

void cd_shortcuts_remove_one_bookmark (const gchar *cURI)
{
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s)", __func__, cURI);
	
	gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents  (cBookmarkFilePath, &cContent, &length, &erreur);
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
				cBookmarksList[i] = g_strdup ("");
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
			g_file_set_contents (cBookmarkFilePath, cContent, -1, &erreur);
			if (erreur != NULL)
			{
				cd_warning ("while trying to write bookmarks file : %s", erreur->message);
				g_error_free (erreur);
			}
			g_free (cContent);
		}
		g_strfreev (cBookmarksList);
	}
	g_free (cBookmarkFilePath);
}

void cd_shortcuts_rename_one_bookmark (const gchar *cURI, const gchar *cName)
{
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s, %s)", __func__, cURI, cName);
	
	gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents  (cBookmarkFilePath, &cContent, &length, &erreur);
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
			g_file_set_contents (cBookmarkFilePath, cContent, -1, &erreur);
			if (erreur != NULL)
			{
				cd_warning ("while trying to write bookmarks file : %s", erreur->message);
				g_error_free (erreur);
			}
			g_free (cContent);
		}
		g_strfreev (cBookmarksList);
	}
	g_free (cBookmarkFilePath);
}

void cd_shortcuts_add_one_bookmark (const gchar *cURI)
{
	g_return_if_fail (cURI != NULL);
	cd_message ("%s (%s)", __func__, cURI);
	
	gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
	FILE *f = fopen (cBookmarkFilePath, "a");
	if (f != NULL)
	{
		gchar *cNewLine = g_strconcat ("\n", cURI, NULL);
		fputs(cNewLine, f);
		g_free (cNewLine);
		fclose (f);
	}
	g_free (cBookmarkFilePath);
}


GList *cd_shortcuts_list_bookmarks (gchar *cBookmarkFilePath)
{
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents  (cBookmarkFilePath, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s\n  no bookmark will be available", erreur->message);
		g_error_free (erreur);
		return NULL;
	}
	else
	{
		GList *pBookmarkIconList = NULL;
		gchar **cBookmarksList = g_strsplit (cContent, "\n", -1);
		g_free (cContent);
		
		gchar *cOneBookmark;
		Icon *pNewIcon;
		gchar *cName, *cRealURI, *cIconName, *cUserName;
		gboolean bIsDirectory;
		int iVolumeID;
		double fOrder, fCurrentOrder = 0;
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
			cName = NULL;
			cRealURI = NULL;
			cIconName = NULL;
			if (*cOneBookmark != '\0' && *cOneBookmark != '#' && cairo_dock_fm_get_file_info (cOneBookmark, &cName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, CAIRO_DOCK_FM_SORT_BY_NAME))
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
					cRealURI = g_strdup ("none");
				
				pNewIcon = pNewIcon = cairo_dock_create_dummy_launcher (cName,
					cIconName,
					cRealURI,
					NULL,
					fCurrentOrder ++);
				pNewIcon->iType = CD_BOOKMARK_GROUP;
				pNewIcon->cBaseURI = cOneBookmark;
				pNewIcon->iVolumeID = iVolumeID;
				pBookmarkIconList = g_list_append (pBookmarkIconList, pNewIcon);
			}
			else
			{
				g_free (cOneBookmark);
			}
		}
		g_free (cBookmarksList);
		return pBookmarkIconList;
	}
}
