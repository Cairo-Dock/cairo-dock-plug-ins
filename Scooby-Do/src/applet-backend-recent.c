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

#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-session.h"
#include "applet-listing.h"
#include "applet-search.h"
#include "applet-backend-recent.h"

static GtkRecentManager *s_pRecentMgr = NULL;

  //////////
 // INIT //
//////////

static gboolean init (void)
{
	s_pRecentMgr = gtk_recent_manager_get_default ();
}

static gboolean stop (void)
{
	s_pRecentMgr = NULL;  // pas de "unref" a faire.
}

  ////////////////
 // FILL ENTRY //
////////////////

static gboolean _cd_do_fill_recent_entry (CDEntry *pEntry)
{
	if (pEntry->pIconSurface == NULL)
	{
		GtkRecentInfo *pInfo = gtk_recent_manager_lookup_item (s_pRecentMgr,
			pEntry->cPath,
			NULL);
		if (pInfo != NULL)
		{
			GdkPixbuf *pixbuf = gtk_recent_info_get_icon (pInfo, myDialogs.dialogTextDescription.iSize + 2);
			if (pixbuf != NULL)
			{
				double fImageWidth, fImageHeight;
				pEntry->pIconSurface = cairo_dock_create_surface_from_pixbuf (pixbuf,
					1.,
					myDialogs.dialogTextDescription.iSize,  // width
					myDialogs.dialogTextDescription.iSize,  // height
					0,  // modifier
					&fImageWidth, &fImageHeight,
					NULL, NULL);
				g_object_unref (pixbuf);
				return TRUE;
			}
		}
	}
	return FALSE;
}


  /////////////
 // ACTIONS //
/////////////

static void _cd_do_launch_file (CDEntry *pEntry)
{
	cd_debug ("%s (%s)\n", __func__, pEntry->cPath);
	cairo_dock_fm_launch_uri (pEntry->cPath);
}

static void _cd_do_launch_file_with_given_app (CDEntry *pEntry)
{
	cd_debug ("%s (%s)\n", __func__, pEntry->cPath);
	cairo_dock_launch_command (pEntry->cPath);
}

static void _cd_do_show_file_location (CDEntry *pEntry)
{
	cd_debug ("%s (%s)\n", __func__, pEntry->cPath);
	gchar *cPathUp = g_path_get_dirname (pEntry->cPath);
	g_return_if_fail (cPathUp != NULL);
	cairo_dock_fm_launch_uri (cPathUp);
	g_free (cPathUp);
}


  /////////////////
 // SUB-LISTING //
/////////////////

static GList *_cd_do_list_recent_sub_entries (CDEntry *pEntry, int *iNbEntries)
{
	cd_debug ("%s (%s)\n", __func__, pEntry->cPath);
	CDEntry *pSubEntry;
	GList *pEntries = NULL;
	int i = 0;
	
	GtkRecentInfo *pInfo = gtk_recent_manager_lookup_item (s_pRecentMgr,
		pEntry->cPath,
		NULL);
	if (pInfo != NULL)
	{
		gchar **pApps = gtk_recent_info_get_applications (pInfo, NULL);
		if (pApps != NULL)
		{
			int j;
			for (j = 0; pApps[j] != NULL; j ++)
			{
				pSubEntry = g_new0 (CDEntry, 1);
				pSubEntry->cPath = g_strdup_printf ("%s \"%s\"", pApps[j], pEntry->cPath);
				pSubEntry->cName = g_strdup_printf ("Open %s with %s", pEntry->cName, pApps[j]);
				pSubEntry->cIconName = g_strdup (pApps[j]);
				pSubEntry->execute = _cd_do_launch_file_with_given_app;
				pSubEntry->fill = cd_do_fill_default_entry;
				pEntries = g_list_prepend (pEntries, pSubEntry);
				i ++;
			}
			g_strfreev (pApps);
		}
	}
	
	pSubEntry = g_new0 (CDEntry, 1);
	pSubEntry->cPath = g_strdup (pEntry->cPath);
	pSubEntry->cName = g_strdup (D_("Open location"));
	pSubEntry->cIconName = g_strdup (GTK_STOCK_DIRECTORY);
	pSubEntry->execute = _cd_do_show_file_location;
	pSubEntry->fill = cd_do_fill_default_entry;
	pEntries = g_list_prepend (pEntries, pSubEntry);
	i ++;
	
	*iNbEntries = i;
	return pEntries;
}

  ////////////
 // SEARCH //
////////////

static GList* search (const gchar *cText, int iFilter, gboolean bSearchAll, int *iNbEntries)
{
	cd_debug ("%s (%s)\n", __func__, cText);
	
	GList *pEntries = NULL;
	CDEntry *pEntry;
	
	GList * pRecentFiles = gtk_recent_manager_get_items (s_pRecentMgr);
	if (pRecentFiles == NULL)
	{
		*iNbEntries = 0;
		return NULL;
	}
	
	GtkRecentInfo *pInfo;
	const gchar *cName, *cUri;
	gchar *cLowerCaseName;
	int i = 0, iNbMax = (bSearchAll ? 50 : 3);
	GList *rf;
	for (rf = pRecentFiles, i = 0; rf != NULL && i < iNbMax; rf = rf->next)
	{
		pInfo = rf->data;
		
		cName = gtk_recent_info_get_display_name (pInfo);
		cUri = gtk_recent_info_get_uri (pInfo);
		cLowerCaseName = g_ascii_strdown (cName, -1);
		if (cUri != NULL && cLowerCaseName != NULL && g_strstr_len (cLowerCaseName, -1, cText) != NULL)
		{
			pEntry = g_new0 (CDEntry, 1);
			pEntry->cPath = g_strdup (cUri);
			pEntry->cName = g_strdup (cName);
			pEntry->cLowerCaseName = cLowerCaseName;
			pEntry->cIconName = NULL;
			pEntry->execute = _cd_do_launch_file;
			pEntry->fill = _cd_do_fill_recent_entry;
			pEntry->list = _cd_do_list_recent_sub_entries;
			pEntries = g_list_prepend (pEntries, pEntry);
			i ++;
		}
		else
			g_free (cLowerCaseName);
		
		gtk_recent_info_unref (pInfo);
	}
	g_list_free (pRecentFiles);
	
	pEntries = g_list_reverse (pEntries);  // les plus recents en premier.
	
	if (! bSearchAll && i != 0)
	{
		pEntry = g_new0 (CDEntry, 1);
		pEntry->cPath = NULL;
		pEntry->cName = g_strdup (D_("Recent files"));
		pEntry->cIconName = g_strdup (MY_APPLET_SHARE_DATA_DIR"/recent.png");
		pEntry->bMainEntry = TRUE;
		pEntry->execute = NULL;
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

void cd_do_register_recent_backend (void)
{
	CDBackend *pBackend = g_new0 (CDBackend, 1);
	pBackend->cName = "Recent";
	pBackend->bIsThreaded = FALSE;
	pBackend->init = (CDBackendInitFunc) init;
	pBackend->stop = (CDBackendStopFunc) stop;
	pBackend->search = (CDBackendSearchFunc) search;
	myData.pBackends = g_list_prepend (myData.pBackends, pBackend);
}
