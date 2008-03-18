/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-draw.h"
#include "applet-bookmarks.h"

extern AppletConfig myConfig;
extern AppletData myData;

CD_APPLET_INCLUDE_MY_VARS


static GList * _cd_shortcuts_detach_icon_from_list (Icon *icon, GList *pIconList, gboolean bUseSeparator)
{
	pIconList = g_list_remove (pIconList, icon);  // on s'en fout des separateurs en mode desklet.
	return pIconList;
}

static void _cd_shortcuts_detach_one_bookmark (Icon *icon, GList **pList)
{
	*pList = g_list_append (*pList, icon);
	if (myIcon->pSubDock != NULL)
		cairo_dock_detach_icon_from_dock (icon, myIcon->pSubDock, myConfig.bUseSeparator);
	else
	{
		myData.pDeskletIconList = _cd_shortcuts_detach_icon_from_list (icon, myData.pDeskletIconList, myConfig.bUseSeparator);
	}
}
void cd_shortcuts_on_change_bookmarks (CairoDockFMEventType iEventType, const gchar *cURI, gpointer data)
{
	cd_message ("%s (%d)", __func__, iEventType);
	g_return_if_fail (myIcon->pSubDock != NULL || myDesklet);
	
	if (iEventType == CAIRO_DOCK_FILE_CREATED || iEventType == CAIRO_DOCK_FILE_MODIFIED)
	{
		cd_message ("  un signet en plus ou en moins");
		//\____________________ On detache les icones des signets.
		GList *pPrevBookmarkIconList = NULL;
		Icon *pSeparatorIcon = cairo_dock_foreach_icons_of_type ((myDock ? myIcon->pSubDock->icons : myData.pDeskletIconList), 10, (CairoDockForeachIconFunc) _cd_shortcuts_detach_one_bookmark, &pPrevBookmarkIconList);
		
		//\____________________ On lit le fichier des signets.
		gchar *cBookmarkFilePath = g_strdup_printf ("%s/.gtk-bookmarks", g_getenv ("HOME"));
		gchar *cContent = NULL;
		gsize length=0;
		GError *erreur = NULL;
		g_file_get_contents  (cBookmarkFilePath, &cContent, &length, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
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
					cd_message (" = 1 signet : %s", cOneBookmark);
					pPrevBookmarkIconList = g_list_remove (pPrevBookmarkIconList, pExistingIcon);
					pExistingIcon->fOrder = fCurrentOrder ++;
					if (myDock)
						cairo_dock_insert_icon_in_dock (pExistingIcon, myIcon->pSubDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, myConfig.bUseSeparator);
					else
						myData.pDeskletIconList = g_list_append (myData.pDeskletIconList, pExistingIcon);
					g_free (cOneBookmark);
				}
				else  // on la cree.
				{
					if (*cOneBookmark != '\0' && *cOneBookmark != '#' && cairo_dock_fm_get_file_info (cOneBookmark, &cName, &cRealURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, g_iFileSortType))
					{
						cd_message (" + 1 signet : %s", cOneBookmark);
						pNewIcon = g_new0 (Icon, 1);
						pNewIcon->iType = 10;
						pNewIcon->cBaseURI = cOneBookmark;
						pNewIcon->acName = cName;
						pNewIcon->acCommand = cRealURI;
						pNewIcon->acFileName = cIconName;
						pNewIcon->iVolumeID = iVolumeID;
						pNewIcon->fOrder = fCurrentOrder ++;
						if (myDesklet)
						{
							pNewIcon->fWidth = 48 * MIN (myData.fTreeWidthFactor, myData.fTreeHeightFactor);
							pNewIcon->fHeight = 48 * MIN (myData.fTreeWidthFactor, myData.fTreeHeightFactor);
						}
						
						cairo_dock_load_one_icon_from_scratch (pNewIcon, (myDock ? CAIRO_DOCK_CONTAINER (myIcon->pSubDock) : myContainer));
						if (myDock)
							cairo_dock_insert_icon_in_dock (pNewIcon, myIcon->pSubDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, myConfig.bUseSeparator);
						else
						{
							myData.pDeskletIconList = g_list_append (myData.pDeskletIconList, pNewIcon);
							myDesklet->icons = myData.pDeskletIconList;
						}
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
			if (myDock)
			{
				Icon *pFirstBookmarkIcon = cairo_dock_get_first_icon_of_type (myIcon->pSubDock->icons, 10);
				if (pFirstBookmarkIcon == NULL && pSeparatorIcon != NULL)
				{
					cd_message ("on enleve l'ancien separateur");
					cairo_dock_detach_icon_from_dock (pSeparatorIcon, myIcon->pSubDock, myConfig.bUseSeparator);
					cairo_dock_free_icon (pSeparatorIcon);
				}
			}
		}
		g_free (cBookmarkFilePath);
		if (myDock)
			cairo_dock_update_dock_size (myIcon->pSubDock);
		else
		{
			cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
			cd_shortcuts_load_tree (myData.pDeskletIconList, pCairoContext);
			cairo_destroy (pCairoContext);
			gtk_widget_queue_draw (myDesklet->pWidget);
		}
	}
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
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
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
		
		g_file_set_contents (cBookmarkFilePath, sNewContent->str, -1, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
		
		g_string_free (sNewContent, TRUE);
		
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
				cd_message (" + 1 signet : %s\n", cOneBookmark);
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
