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
#include "applet-backend-command.h"

// sub-listing
// fill entry
// actions
static void _cd_do_execute_command (CDEntry *pEntry);

  //////////
 // INIT //
//////////



  /////////////////
 // SUB-LISTING //
/////////////////



  ////////////////
 // FILL ENTRY //
////////////////



  /////////////
 // ACTIONS //
/////////////

static void _cd_do_execute_command (CDEntry *pEntry)
{
	gchar *cCommand = g_strdup_printf ("%s/calc.sh '%s'", MY_APPLET_SHARE_DATA_DIR, myData.sCurrentText->str);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	if (cResult != NULL && strcmp (cResult, "0") != 0)
	{
		g_print (" resultat du calcul : '%s'\n", cResult);
		GtkClipboard *pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_set_text (pClipBoard, cResult, -1);
		Icon *pIcon = cairo_dock_get_dialogless_icon ();
		cairo_dock_show_temporary_dialog_with_icon (D_("The value %s has been copied into the clipboard."),
			pIcon,
			CAIRO_CONTAINER (g_pMainDock),
			3000,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			cResult);
	}
	else  // le calcul n'a rien donne, on execute sans chercher.
	{
		g_print (" pas un calcul => on execute '%s'\n", myData.sCurrentText->str);
		cairo_dock_launch_command (myData.sCurrentText->str);
	}
	g_free (cResult);
}


  ////////////
 // SEARCH //
////////////

static GList * search (const gchar *cText, gint iFilter, gpointer pData, int *iNbEntries)
{
	g_print ("%s (%s)\n", __func__, cText);
	GList *pEntries = NULL;
	CDEntry *pEntry;
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cName = g_strdup (D_("Execute"));
	pEntry->cIconName = g_strdup (GTK_STOCK_EXECUTE);
	pEntry->execute = _cd_do_execute_command;
	pEntry->fill = cd_do_fill_default_entry;
	pEntries = g_list_prepend (pEntries, pEntry);
	
	*iNbEntries = 1;
	return pEntries;
}


  //////////////
 // REGISTER //
//////////////

void cd_do_register_command_backend (void)
{
	CDBackend *pBackend = g_new0 (CDBackend, 1);
	pBackend->cName = "Command";
	pBackend->bIsThreaded = FALSE;
	pBackend->bStaticResults = TRUE;
	pBackend->init =(CDBackendInitFunc) NULL;
	pBackend->search = (CDBackendSearchFunc) search;
	myData.pBackends = g_list_prepend (myData.pBackends, pBackend);
}
