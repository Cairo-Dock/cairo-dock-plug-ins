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
#include "applet-backend-web.h"

// sub-listing

// fill entry
static gboolean _cd_do_fill_web_entry (CDEntry *pEntry);
// actions
static void _cd_do_web_search (CDEntry *pEntry);

  //////////
 // INIT //
//////////


  ////////////////
 // FILL ENTRY //
////////////////

static gboolean _cd_do_fill_web_entry (CDEntry *pEntry)
{
	if (pEntry->cIconName && pEntry->pIconSurface == NULL)
	{
		cairo_t* pSourceContext = cairo_dock_create_context_from_container (CAIRO_CONTAINER (g_pMainDock));
		gchar *cImagePath = g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/", pEntry->cIconName, NULL);
		pEntry->pIconSurface = cairo_dock_create_surface_from_icon (cImagePath,
			pSourceContext,
			myDialogs.dialogTextDescription.iSize + 2,
			myDialogs.dialogTextDescription.iSize + 2);
		cairo_destroy (pSourceContext);
		g_free (cImagePath);
		return TRUE;
	}
	return FALSE;
}


  /////////////
 // ACTIONS //
/////////////

static void _cd_do_web_search (CDEntry *pEntry)
{
	gchar *cEscapedText = g_uri_escape_string (myData.cSearchText ? myData.cSearchText : myData.sCurrentText->str,
		"",
		TRUE);
	g_print ("cEscapedText : %s\n", cEscapedText);
	gchar *cURI = g_strdup_printf (pEntry->cPath, cEscapedText);
	cairo_dock_fm_launch_uri (cURI);
	g_free (cURI);
	g_free (cEscapedText);
}


  /////////////////
 // SUB-LISTING //
/////////////////


  ////////////
 // SEARCH //
////////////

static GList* search (const gchar *cText, int iFilter, gboolean bSearchAll, int *iNbEntries)
{
	g_print ("%s (%s)\n", __func__, cText);
	
	GList *pEntries = NULL;
	CDEntry *pEntry;
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup ("http://en.wikipedia.org/w/index.php?title=Special:Search&go=Go&search=%s");
	pEntry->cName = g_strdup (D_("Wikipedia"));
	pEntry->cIconName = g_strdup ("wikipedia.png");
	pEntry->fill = _cd_do_fill_web_entry;
	pEntry->execute = _cd_do_web_search;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup ("http://search.yahoo.com/search?p=%s&ie=utf-8");
	pEntry->cName = g_strdup (D_("Yahoo!"));
	pEntry->cIconName = g_strdup ("yahoo.png");
	pEntry->fill = _cd_do_fill_web_entry;
	pEntry->execute = _cd_do_web_search;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup ("http://www.google.fr/search?q=%s&ie=utf-8");
	pEntry->cName = g_strdup (D_("Google"));
	pEntry->cIconName = g_strdup ("google.png");
	pEntry->fill = _cd_do_fill_web_entry;
	pEntry->execute = _cd_do_web_search;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	if (! bSearchAll)  // on arrete la, et gogol se retrouve en 1er.
	{
		pEntry = g_new0 (CDEntry, 1);
		pEntry->cPath = g_strdup ("http://www.google.fr/search?q=%s&ie=utf-8");
		pEntry->cName = g_strdup (D_("Search on the web"));
		pEntry->cIconName = g_strdup ("internet.png");
		pEntry->bMainEntry = TRUE;
		pEntry->execute = _cd_do_web_search;
		pEntry->fill = _cd_do_fill_web_entry;
		pEntry->list = cd_do_list_main_sub_entry;
		pEntries = g_list_prepend (pEntries, pEntry);
		
		*iNbEntries = 4;
	}
	else  // on liste les autres, qui se retrouvent en 1er.
	{
		pEntry = g_new0 (CDEntry, 1);
		pEntry->cPath = g_strdup ("http://www.mediadico.com/dictionnaire/definition/%s/1");
		pEntry->cName = g_strdup (D_("Mediadico"));
		pEntry->cIconName = g_strdup ("mediadico.png");
		pEntry->fill = _cd_do_fill_web_entry;
		pEntry->execute = _cd_do_web_search;
		pEntries = g_list_prepend (pEntries, pEntry);
		
		pEntry = g_new0 (CDEntry, 1);
		pEntry->cPath = g_strdup ("http://www.amazon.com/s?ie=UTF8&index=blended&link_code=qs&field-keywords=%s");
		pEntry->cName = g_strdup (D_("Amazon"));
		pEntry->cIconName = g_strdup ("amazon.png");
		pEntry->fill = _cd_do_fill_web_entry;
		pEntry->execute = _cd_do_web_search;
		pEntries = g_list_prepend (pEntries, pEntry);
		
		*iNbEntries = 5;
	}
	
	return pEntries;
}


  //////////////
 // REGISTER //
//////////////

void cd_do_register_web_backend (void)
{
	CDBackend *pBackend = g_new0 (CDBackend, 1);
	pBackend->cName = "Web";
	pBackend->bIsThreaded = FALSE;
	pBackend->bStaticResults = TRUE;
	pBackend->init =(CDBackendInitFunc) NULL;
	pBackend->search = (CDBackendSearchFunc) search;
	myData.pBackends = g_list_prepend (myData.pBackends, pBackend);
}
