/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-session.h"
#include "applet-listing.h"
#include "applet-search.h"
#include "applet-backend-files.h"

// sub-listing
static GList *_cd_do_list_bookmarks_actions (CDEntry *pEntry, int *iNbEntries)
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
	gchar *cPath = g_strdup_printf ("%s/.mozilla/firefox/Profiles", g_getenv ("HOME"));
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
	CDEntry *pEntry;
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Open"));
	pEntry->cIconName = g_strdup (GTK_STOCK_JUMP_TO);
	pEntry->fill = cd_do_fill_default_entry;
	pEntry->execute = _cd_do_launch_url;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup (cPath);
	pEntry->cName = g_strdup (D_("Copy URL"));
	pEntry->cIconName = g_strdup (GTK_STOCK_COPY);
	pEntry->fill = cd_do_fill_default_entry;
	pEntry->execute = _cd_do_copy_url;
	pEntries = g_list_prepend (pEntries, pEntry);
	
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
		cairo_t* pSourceContext = cairo_dock_create_context_from_container (CAIRO_CONTAINER (g_pMainDock));
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data (const guchar *data,
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
		g_free (pEntry->cIconName);
		pEntry->cIconName = NULL;
		cairo_destroy (pSourceContext);
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

static GList * _build_entries (gchar *cResult, int *iNbEntries)
{
	gchar **pMatchingFiles = g_strsplit (cResult, "\n", 0);
	
	GList *pEntries = NULL;
	CDEntry *pEntry;
	int i;
	for (i = 0; pMatchingFiles[i] != NULL; i ++)
	{
		pEntry = g_new0 (CDEntry, 1);
		pEntry->cPath = pMatchingFiles[i];
		pEntry->cName = g_path_get_basename (pEntry->cPath);
		pEntry->fill = _cd_do_fill_file_entry;
		pEntry->execute = _cd_do_launch_file;
		pEntry->list = _cd_do_list_file_sub_entries;
		pEntries = g_list_prepend (pEntries, pEntry);
	}
	g_free (pMatchingFiles);
	
	*iNbEntries = i;
	return pEntries;
}

static GList* search (const gchar *cText, int iFilter, gpointer pData, int *iNbEntries)
{
	g_print ("%s (%s)\n", __func__, cText);
	g_return_val_if_fail (pData != NULL, NULL);
	gchar *cBookmarks = *pData;
	
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
	gchar *url, *icon, *name;
	
	do
	{
		str2 = strchr (str, '\n');
		if (str2)
		{
			*str2 = '\0';
		}
		
		url = g_strstr_len (str, -1, "<A HREF=");
		if (url)  // il y'a bien une adresse sur cette ligne.
		{
			url += 9;
			name = strrchr (str2 - 5, '>');  // <A a="x" b="y">name</A>
			if (name)
			{
				*name = '\0';
				name ++;
				*(str2 - 4) = '\0';
				/// gerer le filtre "match case"...
				
				if (g_strstr_len (name, -1, cText))  // trouve.
				{
					str = strchr (url+1, '"');
					if (str)
						*str = '\0';
					
					pEntry = g_new0 (CDEntry, 1);
					pEntry->cPath = g_strdup (url);
					pEntry->cName = g_strdup (name);
					pEntry->fill = _cd_do_fill_bookmark_entry;
					pEntry->execute = _cd_do_launch_url;
					pEntry->list = _cd_do_list_bookmarks_actions;
					pEntries = g_list_prepend (pEntries, pEntry);
					i ++;
					
					icon = g_strstr_len (url, -1, "ICON=\"data:");  // ICON="data:image/x-icon;base64,ABCEDF..."
					if (icon)
					{
						icon += 11;
						if (*icon != '"')
						{
							if (strncmp (icon, "image/x-icon;base64,", 20) == 0)
							{
								icon += 20;
							//icon = strchr (icon+1, ',');
							//if (icon)
							//{
								//icon ++;
								str = strchr (icon, '"');
								if (str)
								{
									*str = '\0';
									int n = str - icon;
									g_print ("une icone est presente pour %s : %d base64-char => %d octets (%.2fx%.2f)\n", name, str - icon, (str - icon)/4*3, sqrt ((str - icon)/4*3));
									/*image/x-icon;base64,
									image/jpeg;base64,
									image/gif;base64,
									image/png;base64,*/
									pEntry->cIconName = g_new0 (gchar, (str - icon)/4*3);
									int j, k;
									for (j = 0, k = 0; j < n; j += 4, k += 3)
									{
										memcpy (pEntry->cIconName + k, icon+j, 3);
										k += 3;
									}
									g_print ("icon copiee\n");
								}
							}
						}
					}
				}
			}
		}
		
		str = str2 + 1;
	} while (str2);
	
	
	g_free (cContent);
	*iNbEntries = i;
	return pEntries;
}


  //////////////
 // REGISTER //
//////////////

void cd_do_register_files_backend (void)
{
	CDBackend *pBackend = g_new0 (CDBackend, 1);
	pBackend->cName = "Files";
	pBackend->bIsThreaded = TRUE;
	pBackend->init =(CDBackendInitFunc) init;
	pBackend->search = (CDBackendSearchFunc) search;
	myData.pBackends = g_list_prepend (myData.pBackends, pBackend);
}
