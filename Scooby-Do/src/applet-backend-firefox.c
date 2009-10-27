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

// sub-listing
static GList *_cd_do_list_bookmarks_actions (CDEntry *pEntry, int *iNbEntries);
// fill entry
static gboolean _cd_do_fill_bookmark_entry (CDEntry *pEntry);
// actions
static void _cd_do_launch_url (CDEntry *pEntry);
static void _cd_do_copy_url (CDEntry *pEntry);

  //////////
 // INIT //
//////////

static gboolean init (gpointer *pData)
{
	gchar *cPath = g_strdup_printf ("%s/.mozilla/firefox", g_getenv ("HOME"));
	GError *erreur = NULL;
	GDir *dir = g_dir_open (cPath, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		g_free (cPath);
		return FALSE;
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
	if (cBookmarks != NULL)
	{
		g_print ("found bookmarks '%s'\n", cBookmarks);
		*pData = cBookmarks;  // a voir si on garde le fichier en memoire avec un moniteur dessus ...
		return TRUE;
	}
	else
		return FALSE;
}


  /////////////////
 // SUB-LISTING //
/////////////////

#define NB_ACTIONS_ON_BOOKMARKS 2

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
	pSubEntry->cName = g_strdup (D_("Copy URL"));
	pSubEntry->cIconName = g_strdup (GTK_STOCK_COPY);
	pSubEntry->fill = cd_do_fill_default_entry;
	pSubEntry->execute = _cd_do_copy_url;
	pEntries = g_list_prepend (pEntries, pSubEntry);
	
	*iNbEntries = NB_ACTIONS_ON_BOOKMARKS;
	return pEntries;
}


static GList *_cd_do_list_all_bookmarks (CDEntry *pEntry, int *iNbEntries)
{
	GList *pEntries = NULL;
	CDEntry *pSubEntry;
	
	//gchar *cResult = _locate_files (myData.cSearchText, myData.iCurrentFilter, myConfig.iNbResultMax);
	
	
	*iNbEntries = NB_ACTIONS_ON_BOOKMARKS;
	return pEntries;
}


  ////////////////
 // FILL ENTRY //
////////////////

static gboolean _cd_do_fill_bookmark_entry (CDEntry *pEntry)
{
	if (pEntry->cIconName != NULL && pEntry->pIconSurface == NULL)
	{
		gsize out_len = 0;
		gchar *icon = g_base64_decode (pEntry->cIconName, &out_len);
		g_return_val_if_fail (icon != NULL, FALSE);
		
		cairo_t* pSourceContext = cairo_dock_create_context_from_container (CAIRO_CONTAINER (g_pMainDock));
		GdkPixbuf *pixbuf = NULL;
		gdk_pixbuf_new_from_data (icon,
			GDK_COLORSPACE_RGB,
			FALSE,  // has_alpha
			8,  // bits_per_sample
			16, 16,  // width, height
			16*3,  // rowstride
			NULL,
			NULL);
		double fImageWidth=0, fImageHeight=0;
		double fZoomX=0, fZoomY=0;
		pEntry->pIconSurface = cairo_dock_create_surface_from_pixbuf (pixbuf,
			pSourceContext,
			1.,
			myDialogs.dialogTextDescription.iSize, myDialogs.dialogTextDescription.iSize,
			0,
			&fImageWidth, &fImageHeight,
			&fZoomX, &fZoomY);
		g_object_unref (pixbuf);
		cairo_destroy (pSourceContext);
		g_free (icon);
		return TRUE;
	}
	return FALSE;
}


  /////////////
 // ACTIONS //
/////////////

static void _cd_do_launch_url (CDEntry *pEntry)
{
	g_print ("%s (%s)\n", __func__, pEntry->cPath);
	cairo_dock_fm_launch_uri (pEntry->cPath);
}

static void _cd_do_copy_url (CDEntry *pEntry)
{
	g_print ("%s (%s)\n", __func__, pEntry->cPath);
	GtkClipboard *pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text (pClipBoard, pEntry->cPath, -1);
}


  ////////////
 // SEARCH //
////////////

static GList* search (const gchar *cText, int iFilter, gpointer pData, int *iNbEntries)
{
	g_print ("%s (%s)\n", __func__, cText);
	g_return_val_if_fail (pData != NULL, NULL);
	gchar *cBookmarks = pData;
	
	gsize length = 0;
	gchar *cContent = NULL;
	g_file_get_contents (cBookmarks,
		&cContent,
		&length,
		NULL);
	if (cContent == NULL)
	{
		*iNbEntries = 0;
		return NULL;
	}
	
	GList *pEntries = NULL;
	CDEntry *pEntry;
	int i = 0;
	gchar *str = cContent, *str2;
	gchar *url, *icon, *name, *end_url;
	
	do
	{
		str2 = strchr (str, '\n');
		if (str2)
		{
			*str2 = '\0';
		}
		
		url = g_strstr_len (str, -1, "<A HREF=\"");
		if (!url)
		{
			str = str2 + 1;
			continue;
		}
		url += 9;
		
		str = strchr (url, '"');
		if (!str)
		{
			str = str2 + 1;
			continue;
		}
		*str = '\0';
		end_url = str + 1;
		//g_print ("url : '%s'\n", url);
		
		name = strchr (end_url+1, '>');  // <A a="x" b="y">name</A>
		if (!name)
		{
			str = str2 + 1;
			continue;
		}
		*name = '\0';
		name ++;
		*(str2 - 4) = '\0';
		//g_print ("name : '%s'\n", name);
		/// gerer le filtre "match case"...
		
		if (g_strstr_len (name, -1, cText))  // trouve.
		{
			pEntry = g_new0 (CDEntry, 1);
			pEntry->cPath = g_strdup (url);
			pEntry->cName = g_strdup (name);
			pEntry->fill = _cd_do_fill_bookmark_entry;
			pEntry->execute = _cd_do_launch_url;
			pEntry->list = _cd_do_list_bookmarks_actions;
			pEntries = g_list_prepend (pEntries, pEntry);
			i ++;
			
			icon = g_strstr_len (end_url, -1, "ICON=\"data:");  // ICON="data:image/x-icon;base64,ABCEDF..."
			if (!icon)
			{
				str = str2 + 1;
				continue;
			}
			icon += 11;
			
			if (*icon == '"')  // aucune donnee.
			{
				str = str2 + 1;
				continue;
			}
			
			icon = strchr (icon+1, ',');
			if (!icon)
			{
				str = str2 + 1;
				continue;
			}
			icon ++;
			
			str = strchr (icon, '"');
			if (!str)
			{
				str = str2 + 1;
				continue;
			}
			*str = '\0';
			
			pEntry->cIconName = g_strdup (icon);
		}
		str = str2 + 1;
	} while (str2 && i < 3);
	
	if (i != 0)
	{
		pEntry = g_new0 (CDEntry, 1);
		pEntry->cPath = NULL;
		pEntry->cName = g_strdup (D_("Firefox bookmarks"));
		pEntry->cIconName = g_strdup ("firefox");
		pEntry->bMainEntry = TRUE;
		pEntry->fill = cd_do_fill_default_entry;
		pEntry->execute = NULL;
		pEntry->list = _cd_do_list_all_bookmarks;
		pEntries = g_list_prepend (pEntries, pEntry);
		i ++;
	}
	
	g_free (cContent);
	*iNbEntries = i;
	return pEntries;
}


  //////////////
 // REGISTER //
//////////////

void cd_do_register_firefox_backend (void)
{
	CDBackend *pBackend = g_new0 (CDBackend, 1);
	pBackend->cName = "Files";
	pBackend->bIsThreaded = TRUE;
	pBackend->init =(CDBackendInitFunc) init;
	pBackend->search = (CDBackendSearchFunc) search;
	myData.pBackends = g_list_prepend (myData.pBackends, pBackend);
}
