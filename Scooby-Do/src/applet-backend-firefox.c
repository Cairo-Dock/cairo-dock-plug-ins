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
#include <time.h>
#include <math.h>

#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-session.h"
#include "applet-listing.h"
#include "applet-search.h"
#include "applet-backend-files.h"

typedef struct _CDBookmarkItem{
	gchar *cName;
	gchar *cLowerCaseName;
	gchar *cAddress;
	gchar *cComment;
	gchar *cIcon64;
	GList *pSubItems;
	} CDBookmarkItem;

// sub-listing
static GList *_cd_do_list_bookmarks_actions (CDEntry *pEntry, int *iNbEntries);
static GList *_cd_do_list_bookmarks_folder (CDEntry *pEntry, int *iNbEntries);
// fill entry
static gboolean _cd_do_fill_bookmark_entry (CDEntry *pEntry);
// actions
static void _cd_do_launch_url (CDEntry *pEntry);
static void _cd_do_copy_url (CDEntry *pEntry);

static gchar *s_cBookmarksFile = NULL;
static gchar *s_cBookmarksContent = NULL;
static CDBookmarkItem *s_pRootItem = NULL;

  //////////
 // INIT //
//////////

static void _on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, gpointer data);

static void _free_item (CDBookmarkItem *pItem)
{
	if (pItem == NULL)
		return ;
	g_free (pItem->cName);
	g_free (pItem->cLowerCaseName);
	g_free (pItem->cComment);
	g_free (pItem->cAddress);
	g_list_foreach (pItem->pSubItems, (GFunc)_free_item, NULL);
	g_free (pItem);
}

static gchar *_get_bookmarks_path (void)
{
	gchar *cPath = g_strdup_printf ("%s/.mozilla/firefox", g_getenv ("HOME"));
	GError *erreur = NULL;
	GDir *dir = g_dir_open (cPath, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		g_free (cPath);
		return NULL;
	}
	
	gchar *cBookmarks = NULL;
	const gchar *cFileName;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		cBookmarks = g_strdup_printf ("%s/%s/bookmarks.html", cPath, cFileName);
		if (g_file_test (cBookmarks, G_FILE_TEST_EXISTS))
			break;  // on en prend qu'un.
		else
		{
			g_free (cBookmarks);
			cBookmarks = NULL;
		}
	}
	while (1);
	g_dir_close (dir);
	
	g_free (cPath);
	return cBookmarks;
}

static GList *_parse_folder (gchar *cContent, gchar **cNewPosition)
{
	GList *pList = NULL;
	CDBookmarkItem *pItem = NULL, *pFolderItem = NULL;
	gchar *str, *str2, *ptr=cContent;
	
	do
	{
		str = strchr (ptr, '<');
		if (!str)
			break;
		str ++;
		if (*str == 'H' && *(str+1) == '3')  // nouveau repertoire. <H3 ...>nom</H3>
		{
			str = strchr (str+2, '>');  // fin de la balise <H3>
			str ++;
			
			str2 = strchr (str, '<');  // debut de la balise fermante </H3>
			if (str2 != str)
			{
				pFolderItem = g_new0 (CDBookmarkItem, 1);
				pFolderItem->cName = g_strndup (str, str2-str);
				pFolderItem->cLowerCaseName = g_ascii_strdown (pFolderItem->cName, -1);
				pList = g_list_prepend (pList, pFolderItem);
			}
			ptr = str2 + 5;
		}
		else if (*str == 'D' && *(str+1) == 'L')  // debut de contenu du repertoire. <DL> sub-items </DL>
		{
			pFolderItem->pSubItems = _parse_folder (str+4, &ptr);  // la fonction nous place apres le </DL> correspondant.
			pFolderItem = NULL;
		}
		else if (*str == 'A')  // nouvelle adresse. <A HREF="adresse"> nom </A>
		{
			str = g_strstr_len (str+2, -1, "HREF=\"");  // debut d'adresse.
			str += 6;
			str2 = strchr (str, '"');  // fin de l'adresse.
			pItem = g_new0 (CDBookmarkItem, 1);
			pItem->cAddress = g_strndup (str, str2-str);
			pList = g_list_prepend (pList, pItem);
			
			str = str2 + 1;
			str2 = strchr (str, '>');  // fin de la balise <A>
			gchar *icon = g_strstr_len (str, str2 - str, "ICON=\"data:");  // ICON="data:image/x-icon;base64,ABCEDF..."
			if (icon)
			{
				icon += 11;
				if (*icon != '"')  // sinon aucune donnee.
				{
					icon = strchr (icon+1, ',');
					if (icon)
					{
						icon ++;
						str = strchr (icon, '"');
						pItem->cIcon64 = g_strndup (icon, str-icon);
					}
				}
			}
			
			str = str2 + 1;
			str2 = strchr (str, '<');  // debut de la balise fermante </A>
			pItem->cName = g_strndup (str, str2-str);
			pItem->cLowerCaseName = g_ascii_strdown (pItem->cName, -1);
			
			ptr = str2 + 4;
		}
		else if (*str == '/' && *(str+1) == 'D' && *(str+2) == 'L')  // fin du repertoire. <DL> sub-items </DL>
		{
			ptr = str + 4;
			break;
		}
		else if (*str == 'D' && *(str+1) == 'D')  // balise de commentaire. <DD> commentaire
		{
			str += 4;
			str2 = strchr (str, '<');  // debut d'une autre balise.
			if (pFolderItem != NULL)
				pFolderItem->cComment = g_strndup (str, str2-str);
			else if (pItem != NULL)
				pItem->cComment = g_strndup (str, str2-str);
			ptr = str2;
		}
		else  // balise ininteressante, on la saute.
		{
			str2 = str2 = strchr (str, '>');  // fin de la balise.
			ptr = str2 + 1;
		}
	} while (1);
	
	*cNewPosition = ptr;
	return pList;
}

static CDBookmarkItem *_parse_bookmarks (const gchar *cFilePath)
{
	gsize length = 0;
	gchar *cBookmarksContent = NULL;
	g_file_get_contents (cFilePath,
		&cBookmarksContent,
		&length,
		NULL);
	if (cBookmarksContent == NULL)
	{
		cd_warning ("can't read bookmarks");
		return NULL;
	}
	
	gchar *str = g_strstr_len (cBookmarksContent, -1, "<DL>");
	if (!str)
	{
		cd_warning ("empty bookmarks");
		return NULL;
	}
	CDBookmarkItem *pRootItem = g_new0 (CDBookmarkItem, 1);
	pRootItem->pSubItems = _parse_folder (str+4, &str);
	
	g_free (cBookmarksContent);
	return pRootItem;
}

static gboolean init (void)
{
	// on trouve le fichier des bookmarks.
	s_cBookmarksFile = _get_bookmarks_path ();
	if (s_cBookmarksFile == NULL)
	{
		cd_warning ("no bookmarks");
		return FALSE;
	}
	cd_debug ("found bookmarks '%s'\n", s_cBookmarksFile);
	
	// on parse le fichier.
	s_pRootItem = _parse_bookmarks (s_cBookmarksFile);
	
	// on surveille le fichier.
	cairo_dock_fm_add_monitor_full (s_cBookmarksFile, FALSE, NULL, (CairoDockFMMonitorCallback) _on_file_event, NULL);
	
	return TRUE;
}

static void stop (void)
{
	if (s_cBookmarksFile == NULL)
		return ;
	
	cairo_dock_fm_remove_monitor_full (s_cBookmarksFile, FALSE, NULL);
	g_free (s_cBookmarksFile);
	s_cBookmarksFile = NULL;
	_free_item (s_pRootItem);
	s_pRootItem = NULL;
}

static void _on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, gpointer data)
{
	cd_debug ("bookmarks have changed\n");
	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_DELETED :
		case CAIRO_DOCK_FILE_CREATED :
		case CAIRO_DOCK_FILE_MODIFIED :
			stop ();
			init ();
		break;
		
		default :
		break;
	}
}

  ////////////////
 // FILL ENTRY //
////////////////

static gboolean _cd_do_fill_bookmark_entry (CDEntry *pEntry)
{
	if (pEntry->pIconSurface != NULL || pEntry->cIconName == NULL)
		return FALSE;
	
	gsize out_len = 0;
	//g_print ("icon : %s\n", pEntry->cIconName);
	guchar *icon = g_base64_decode (pEntry->cIconName, &out_len);
	//g_print ("-> out_len : %d\n", out_len);
	g_return_val_if_fail (icon != NULL, FALSE);
	//g_print ("-> data : %d\n", icon);
	
	GInputStream * is = g_memory_input_stream_new_from_data (icon,
		out_len,
		NULL);
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_stream (is,
		NULL,
		NULL);
	g_object_unref (is);
	double fImageWidth=0, fImageHeight=0;
	double fZoomX=0, fZoomY=0;
	pEntry->pIconSurface = cairo_dock_create_surface_from_pixbuf (pixbuf,
		1.,
		myDialogsParam.dialogTextDescription.iSize, myDialogsParam.dialogTextDescription.iSize,
		0,
		&fImageWidth, &fImageHeight,
		&fZoomX, &fZoomY);
	g_object_unref (pixbuf);
	g_free (icon);
	
	return TRUE;
}


  /////////////
 // ACTIONS //
/////////////

static void _cd_do_launch_url (CDEntry *pEntry)
{
	cd_debug ("%s (%s)\n", __func__, pEntry->cPath);
	//cairo_dock_fm_launch_uri (pEntry->cPath);
	cairo_dock_launch_command_printf ("firefox \"%s\"", NULL, pEntry->cPath);
}

static void _cd_do_launch_in_new_window (CDEntry *pEntry)
{
	cd_debug ("%s (%s)\n", __func__, pEntry->cPath);
	cairo_dock_launch_command_printf ("firefox -no-remote \"%s\"", NULL, pEntry->cPath);
}

static void _cd_do_copy_url (CDEntry *pEntry)
{
	cd_debug ("%s (%s)\n", __func__, pEntry->cPath);
	GtkClipboard *pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text (pClipBoard, pEntry->cPath, -1);
}

static void _cd_do_launch_all_url (CDEntry *pEntry)
{
	cd_debug ("%s (%s)\n", __func__, pEntry->cPath);
	cairo_dock_launch_command_printf ("firefox %s", NULL, pEntry->cPath);
}

  /////////////////
 // SUB-LISTING //
/////////////////
#define NB_ACTIONS_ON_BOOKMARKS 3

static GList *_cd_do_list_bookmarks_actions (CDEntry *pEntry, int *iNbEntries)
{
	GList *pEntries = NULL;
	CDEntry *pSubEntry;
	
	pSubEntry = g_new0 (CDEntry, 1);
	pSubEntry->cPath = g_strdup (pEntry->cPath);
	pSubEntry->cName = g_strdup (D_("Open"));
	pSubEntry->cIconName = g_strdup (GTK_STOCK_JUMP_TO);
	pSubEntry->fill = cd_do_fill_default_entry;
	pSubEntry->execute = _cd_do_launch_url;
	pEntries = g_list_prepend (pEntries, pSubEntry);
	
	pSubEntry = g_new0 (CDEntry, 1);
	pSubEntry->cPath = g_strdup (pEntry->cPath);
	pSubEntry->cName = g_strdup (D_("Open in new window"));
	pSubEntry->cIconName = g_strdup (GTK_STOCK_ADD);
	pSubEntry->fill = cd_do_fill_default_entry;
	pSubEntry->execute = _cd_do_launch_in_new_window;
	pEntries = g_list_prepend (pEntries, pSubEntry);
	
	pSubEntry = g_new0 (CDEntry, 1);
	pSubEntry->cPath = g_strdup (pEntry->cPath);
	pSubEntry->cName = g_strdup (D_("Copy URL"));
	pSubEntry->cIconName = g_strdup (GTK_STOCK_COPY);
	pSubEntry->fill = cd_do_fill_default_entry;
	pSubEntry->execute = _cd_do_copy_url;
	pEntries = g_list_prepend (pEntries, pSubEntry);
	
	*iNbEntries = NB_ACTIONS_ON_BOOKMARKS;
	return pEntries;
}

static CDEntry *_make_entry_from_item (CDBookmarkItem *pItem)
{
	CDEntry *pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup (pItem->cAddress);
	pEntry->cName = g_strdup (pItem->cName);
	pEntry->cLowerCaseName = g_strdup (pItem->cLowerCaseName);
	if (pItem->pSubItems == NULL)  // adresse
	{
		pEntry->cIconName = g_strdup (pItem->cIcon64);
		pEntry->fill = _cd_do_fill_bookmark_entry;
		pEntry->execute = _cd_do_launch_url;
		pEntry->list = _cd_do_list_bookmarks_actions;
	}
	else
	{
		pEntry->cIconName = g_strdup ("folder");
		pEntry->fill = cd_do_fill_default_entry;
		pEntry->execute = NULL;
		pEntry->list = _cd_do_list_bookmarks_folder;
		pEntry->data = pItem;
	}
	return pEntry;
}

static GList *_cd_do_list_bookmarks_folder (CDEntry *pEntry, int *iNbEntries)
{
	int i = 0;
	GList *pEntries = NULL;
	CDBookmarkItem *pFolderItem = pEntry->data;
	g_return_val_if_fail (pFolderItem != NULL, NULL);
	
	CDBookmarkItem *pItem;
	CDEntry *pSubEntry;
	GString *sAllUrls = g_string_new ("");
	GList *it;
	for (it = pFolderItem->pSubItems; it != NULL; it = it->next)
	{
		pItem = it->data;
		pSubEntry = _make_entry_from_item (pItem);
		i ++;
		pEntries = g_list_prepend (pEntries, pSubEntry);
		
		g_string_append_printf (sAllUrls, "\"%s\" ", pItem->cAddress);
	}
	
	if (pEntries != NULL)
	{
		pSubEntry = g_new0 (CDEntry, 1);
		pSubEntry->cPath = sAllUrls->str;
		pSubEntry->cName = g_strdup (D_("Open file"));
		pSubEntry->cIconName = g_strdup (GTK_STOCK_OPEN);
		pSubEntry->fill = cd_do_fill_default_entry;
		pSubEntry->execute = _cd_do_launch_all_url;
		pEntries = g_list_prepend (pEntries, pSubEntry);
		i ++;
		g_string_free (sAllUrls, FALSE);
	}
	else
		g_string_free (sAllUrls, TRUE);
	
	*iNbEntries = i;
	return pEntries;
}


  ////////////
 // SEARCH //
////////////

static GList* _search_in_item (CDBookmarkItem *pFolderItem, const gchar *cText, int iFilter, int iNbMax, int *iNbEntries)
{
	GList *pEntries = NULL;
	int i = 0;
	CDBookmarkItem *pItem;
	CDEntry *pEntry;
	GList *it;
	for (it = pFolderItem->pSubItems; it != NULL && iNbMax > 0; it = it->next)
	{
		pItem = it->data;
		if (g_strstr_len (pItem->cLowerCaseName, -1, cText))
		{
			pEntry = _make_entry_from_item (pItem);
			pEntries = g_list_prepend (pEntries, pEntry);
			i ++;
			iNbMax --;
		}
		if (pItem->pSubItems != NULL)
		{
			int j = 0;
			GList *pSubList = _search_in_item (pItem, cText, iFilter, iNbMax, &j);
			i += j;
			iNbMax -= j;
			pEntries = g_list_concat (pEntries, pSubList);
		}
	}
	*iNbEntries = i;
	return pEntries;
}
static GList* search (const gchar *cText, int iFilter, gboolean bSearchAll, int *iNbEntries)
{
	cd_debug ("%s (%s)\n", __func__, cText);
	if (s_pRootItem == NULL)
		return NULL;
	
	int i = 0, iNbMax = (bSearchAll ? 50:3);
	CDEntry *pEntry;
	GList *pEntries = _search_in_item (s_pRootItem, cText, iFilter, iNbMax, &i);
	
	if (i != 0 && ! bSearchAll)
	{
		pEntry = g_new0 (CDEntry, 1);
		pEntry->cPath = NULL;
		pEntry->cName = g_strdup (D_("Firefox bookmarks"));
		pEntry->cIconName = g_strdup (MY_APPLET_SHARE_DATA_DIR"/firefox.png");
		pEntry->bMainEntry = TRUE;
		pEntry->fill = cd_do_fill_default_entry;
		pEntry->list = cd_do_list_main_sub_entry;
		pEntries = g_list_prepend (pEntries, pEntry);
		i ++;
	}
	
	*iNbEntries = i;
	return pEntries;
}


  //////////////
 // REGISTER //
//////////////

void cd_do_register_firefox_backend (void)
{
	CDBackend *pBackend = g_new0 (CDBackend, 1);
	pBackend->cName = "Firefox";
	pBackend->bIsThreaded = FALSE;
	pBackend->init =(CDBackendInitFunc) init;
	pBackend->stop = (CDBackendStopFunc) stop;
	pBackend->search = (CDBackendSearchFunc) search;
	myData.pBackends = g_list_prepend (myData.pBackends, pBackend);
}



/* Chromium :
 * {
    "checksum": "f09d99e389bb87e956c8e4b083647898",
    "roots": {
       "bookmark_bar": {
          "children": [ {
             "date_added": "12905638722230845",
             "id": "2",
             "name": "Packages in \u201Cmatttbe\u201D : matttbe : Matthieu Baerts",
             "type": "url",
             "url": "https://launchpad.net/~matttbe/+archive/ppa/+packages"
          }, {
             "date_added": "12905638722232804",
             "id": "3",
             "name": "Packages in \u201Cexperimental-build\u201D : experimental-build : Matthieu Baerts",
             "type": "url",
             "url": "https://launchpad.net/~matttbe/+archive/experimental/+packages"
          }, {
             "date_added": "12905638722234798",
             "id": "5",
             "name": "Wireless Active Client MAC List",
             "type": "url",
             "url": "http://192.168.1.1/WClient.htm"
          }, {
	* 
	*/
