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
#include "applet-backend-command.h"

// sub-listing
// fill entry
// actions
static void _cd_do_execute_command (CDEntry *pEntry);

  //////////
 // INIT //
//////////


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
		cd_debug (" resultat du calcul : '%s'", cResult);
		GtkClipboard *pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_set_text (pClipBoard, cResult, -1);
		Icon *pIcon = gldi_icons_get_any_without_dialog ();
		gldi_dialog_show_temporary_with_icon_printf (D_("The value %s has been copied into the clipboard."),
			pIcon,
			CAIRO_CONTAINER (g_pMainDock),
			3000,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			cResult);
	}
	else  // le calcul n'a rien donne, on execute sans chercher.
	{
		cd_debug (" pas un calcul => on execute '%s'", myData.sCurrentText->str);
		cairo_dock_launch_command (myData.sCurrentText->str);
	}
	g_free (cResult);
}


  /////////////////
 // SUB-LISTING //
/////////////////


  ////////////
 // SEARCH //
////////////

static GList * search (const gchar *cText, gint iFilter, gboolean bSearchAll, int *iNbEntries)
{
	cd_debug ("%s (%s)", __func__, cText);
	GList *pEntries = NULL;
	CDEntry *pEntry;
	
	pEntry = g_new0 (CDEntry, 1);
	pEntry->cName = g_strdup (D_("Execute"));
	pEntry->cIconName = g_strdup (GLDI_ICON_NAME_EXECUTE);
	pEntry->bMainEntry = TRUE;
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
	pBackend->init = (CDBackendInitFunc) NULL;
	pBackend->search = (CDBackendSearchFunc) search;
	myData.pBackends = g_list_prepend (myData.pBackends, pBackend);
}
