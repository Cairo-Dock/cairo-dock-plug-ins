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
#include "applet-backend-web.h"

// sub-listing
static GList *_cd_do_list_web_sub_entries (CDEntry *pEntry, int *iNbEntries);
// fill entry
static gboolean _cd_do_fill_web_entry (CDEntry *pEntry);
// actions
static void _cd_do_web_search (CDEntry *pEntry);

  //////////
 // INIT //
//////////



  /////////////////
 // SUB-LISTING //
/////////////////

#define NB_WEB_ENGINES 3
static GList *_cd_do_list_web_sub_entries (CDEntry *pEntry, int *iNbEntries)
{
	g_print ("%s ()\n", __func__);
	GList *pEntries = NULL;
	CDEntry *pSubEntry;
	
	pSubEntry = g_new0 (CDEntry, 1);
	
	pSubEntry = g_new0 (CDEntry, 1);
	pSubEntry->cPath = g_strdup ("http://www.google.fr/search?q=%s&ie=utf-8");
	pSubEntry->cName = g_strdup (D_("Google"));
	pSubEntry->cIconName = g_strdup ("google.png");
	pSubEntry->fill = _cd_do_fill_web_entry;
	pSubEntry->execute = _cd_do_web_search;
	pEntries = g_list_prepend (pEntries, pSubEntry);
	
	pSubEntry = g_new0 (CDEntry, 1);
	pSubEntry->cPath = g_strdup ("http://en.wikipedia.org/w/index.php?title=Special:Search&go=Go&search=%s");
	pSubEntry->cName = g_strdup (D_("Wikipedia"));
	pSubEntry->cIconName = g_strdup ("wikipedia.png");
	pSubEntry->fill = _cd_do_fill_web_entry;
	pSubEntry->execute = _cd_do_web_search;
	pEntries = g_list_prepend (pEntries, pSubEntry);
	
	pSubEntry = g_new0 (CDEntry, 1);
	pSubEntry->cPath = g_strdup ("http://search.yahoo.com/search?p=%s&ie=utf-8");
	pSubEntry->cName = g_strdup (D_("Yahoo"));
	pSubEntry->cIconName = g_strdup ("yahoo.png");
	pSubEntry->fill = _cd_do_fill_web_entry;
	pSubEntry->execute = _cd_do_web_search;
	pEntries = g_list_prepend (pEntries, pSubEntry);
	
	*iNbEntries = NB_WEB_ENGINES;
	return pEntries;
}


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
	gchar *cEscapedText = g_uri_escape_string (myData.cSearchText,
		"",
		TRUE);
	g_print ("cEscapedText : %s\n", cEscapedText);
	gchar *cURI = g_strdup_printf (pEntry->cPath, cEscapedText);
	cairo_dock_fm_launch_uri (cURI);
	g_free (cURI);
	g_free (cEscapedText);
}


  ////////////
 // SEARCH //
////////////

static GList* search (const gchar *cText, int iFilter, gpointer pData, int *iNbEntries)
{
	g_print ("%s (%s)\n", __func__, cText);
	GList *pEntries = NULL;
	CDEntry *pEntry;
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cPath = g_strdup ("http://www.google.fr/search?q=%s&ie=utf-8");
	pEntry->cName = g_strdup (D_("Search on the web"));
	pEntry->cIconName = g_strdup ("internet.png");
	pEntry->execute = _cd_do_web_search;
	pEntry->fill = _cd_do_fill_web_entry;
	pEntry->list = _cd_do_list_web_sub_entries;
	pEntries = g_list_prepend (pEntries, pEntry);
	
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
