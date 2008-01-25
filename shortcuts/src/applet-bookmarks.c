
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-bookmarks.h"

extern gboolean my_bUseSeparator;

CD_APPLET_INCLUDE_MY_VARS


static void _cd_shortcuts_detach_one_bookmark (Icon *icon, CairoDock *pDock, GList **pList)
{
	*pList = g_list_append (*pList, icon);
	cairo_dock_detach_icon_from_dock (icon, pDock, my_bUseSeparator);
}
void cd_shortcuts_on_change_bookmarks (CairoDockFMEventType iEventType, const gchar *cURI, gpointer data)
{
	g_print ("%s (%d)\n", __func__, iEventType);
	
	if (iEventType == CAIRO_DOCK_FILE_CREATED || iEventType == CAIRO_DOCK_FILE_MODIFIED)
	{
		g_print ("  un signet en plus ou en moins\n");
		//\____________________ On detache les icones des signets.
		GList *pPrevBookmarkIconList = NULL;
		Icon *pSeparatorIcon = cairo_dock_foreach_icons_of_type (myIcon->pSubDock, 10, (CairoDockForeachIconFunc) _cd_shortcuts_detach_one_bookmark, &pPrevBookmarkIconList);
		
		//\____________________ On lit le fichier des signets.
		gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
		gchar *cContent = NULL;
		gsize length=0;
		GError *tmp_erreur = NULL;
		g_file_get_contents  (cBookmarkFilePath, &cContent, &length, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_print ("Attention : %s\n", tmp_erreur->message);
			g_error_free (tmp_erreur);
		}
		else
		{
			gchar **cBookmarksList = g_strsplit (cContent, "\n", -1);
			g_free (cContent);
			gchar *cOneBookmark;
			Icon *pNewIcon;
			gchar *cName, *cRealURI, *cIconName;
			gboolean bIsDirectory;
			int iVolumeID;
			double fOrder, fCurrentOrder = 0;
			int i = 0;
			for (i = 0; cBookmarksList[i] != NULL; i ++)
			{
				cOneBookmark = cBookmarksList[i];
				
				Icon *pExistingIcon = cairo_dock_get_icon_with_base_uri (pPrevBookmarkIconList, cOneBookmark);
				if (pExistingIcon != NULL)  // on la reinsere a sa place.
				{
					g_print (" = 1 signet : %s\n", cOneBookmark);
					pPrevBookmarkIconList = g_list_remove (pPrevBookmarkIconList, pExistingIcon);
					pExistingIcon->fOrder = fCurrentOrder ++;
					cairo_dock_insert_icon_in_dock (pExistingIcon, myIcon->pSubDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, my_bUseSeparator);
					g_free (cOneBookmark);
				}
				else  // on la cree.
				{
					if (*cOneBookmark != '\0' && *cOneBookmark != '#' && cairo_dock_fm_get_file_info (cOneBookmark, &cName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, g_iFileSortType))
					{
						g_print (" + 1 signet : %s\n", cOneBookmark);
						pNewIcon = g_new0 (Icon, 1);
						pNewIcon->iType = 10;
						pNewIcon->cBaseURI = cOneBookmark;
						pNewIcon->acName = cName;
						pNewIcon->acCommand = cRealURI;
						pNewIcon->acFileName = cIconName;
						pNewIcon->iVolumeID = iVolumeID;
						pNewIcon->fOrder = fCurrentOrder ++;
						
						cairo_dock_load_one_icon_from_scratch (pNewIcon, myIcon->pSubDock);
						cairo_dock_insert_icon_in_dock (pNewIcon, myIcon->pSubDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, my_bUseSeparator);
					}
					else
					{
						g_free (cOneBookmark);
					}
				}
			}
			g_free (cBookmarksList);
			
			//\____________________ On supprime les signets qui restent.
			g_list_foreach (pPrevBookmarkIconList, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (pPrevBookmarkIconList);
			
			//\____________________ On ajoute ou supprime un separateur.
			Icon *pFirstBookmarkIcon = cairo_dock_get_first_icon_of_type (myIcon->pSubDock->icons, 10);
			if (pFirstBookmarkIcon == NULL && pSeparatorIcon != NULL)
			{
				g_print ("on enleve l'ancien separateur\n");
				cairo_dock_detach_icon_from_dock (pSeparatorIcon, myIcon->pSubDock, my_bUseSeparator);
				cairo_dock_free_icon (pSeparatorIcon);
			}
		}
		g_free (cBookmarkFilePath);
		cairo_dock_update_dock_size (myIcon->pSubDock);
	}
}

void cd_shortcuts_remove_one_bookmark (const gchar *cURI)
{
	g_return_if_fail (cURI != NULL);
	g_print ("%s (%s)\n", __func__, cURI);
	
	gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
	gchar *cContent = NULL;
	gsize length=0;
	GError *tmp_erreur = NULL;
	g_file_get_contents  (cBookmarkFilePath, &cContent, &length, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_print ("Attention : %s\n", tmp_erreur->message);
		g_error_free (tmp_erreur);
	}
	else
	{
		GString *sNewContent = g_string_new ("");
		gchar **cBookmarksList = g_strsplit (cContent, "\n", -1);
		g_free (cContent);
		gchar *cOneBookmark;
		int i = 0;
		for (i = 0; cBookmarksList[i] != NULL; i ++)
		{
			cOneBookmark = cBookmarksList[i];
			
			if (*cOneBookmark != '\0' && strcmp (cOneBookmark, cURI) != 0)
			{
				g_string_append (sNewContent, cOneBookmark);
				g_string_append_c (sNewContent, '\n');
			}
			g_free (cOneBookmark);
		}
		g_free (cBookmarksList);
		
		g_file_set_contents (cBookmarkFilePath, sNewContent->str, -1, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_print ("Attention : %s\n", tmp_erreur->message);
			g_error_free (tmp_erreur);
		}
		
		g_string_free (sNewContent, TRUE);
		
	}
	g_free (cBookmarkFilePath);
}

void cd_shortcuts_add_one_bookmark (const gchar *cURI)
{
	g_return_if_fail (cURI != NULL);
	g_print ("%s (%s)\n", __func__, cURI);
	
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
	GError *tmp_erreur = NULL;
	g_file_get_contents  (cBookmarkFilePath, &cContent, &length, &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_print ("Attention : %s\n  no bookmark will be available\n", tmp_erreur->message);
		g_error_free (tmp_erreur);
		return NULL;
	}
	else
	{
		GList *pBookmarkIconList = NULL;
		gchar **cBookmarksList = g_strsplit (cContent, "\n", -1);
		g_free (cContent);
		
		gchar *cOneBookmark;
		Icon *pNewIcon;
		gchar *cName, *cRealURI, *cIconName;
		gboolean bIsDirectory;
		int iVolumeID;
		double fOrder, fCurrentOrder = 0;
		int i = 0;
		for (i = 0; cBookmarksList[i] != NULL; i ++)
		{
			cOneBookmark = cBookmarksList[i];
			if (*cOneBookmark != '\0' && *cOneBookmark != '#' && cairo_dock_fm_get_file_info (cOneBookmark, &cName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, g_iFileSortType))
			{
				g_print (" + 1 signet : %s\n", cOneBookmark);
				pNewIcon = g_new0 (Icon, 1);
				pNewIcon->iType = 10;
				pNewIcon->cBaseURI = cOneBookmark;
				pNewIcon->acName = cName;
				pNewIcon->acCommand = cRealURI;
				pNewIcon->acFileName = cIconName;
				pNewIcon->iVolumeID = iVolumeID;
				pNewIcon->fOrder = fCurrentOrder ++;
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
