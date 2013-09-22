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

#include "applet-struct.h"
#include "applet-entry.h"

#include <gdk/gdkkeysyms.h> // needed for 'GDK_KEY_Return'


typedef struct _EntryInfo {
	GAppInfo  *pAppInfo;
	GtkWidget *pMenuItem;
	gboolean   bKeepMenu; // flag to not destroy the menu item if it's reused after
	} EntryInfo;

static GList *s_pEntries = NULL;    // a list with all matches apps
static gint s_iNbSearchEntries = 0; // the number of elements in this list (to not iterate over the whole list)
static gint s_iNbOtherEntries = 0;  // the number of hidden entries (applications menu)

static gint _compare_apps (const EntryInfo *a, const EntryInfo *b)
{
	// ignore cases: some apps don't have capital letters for the first char
	return g_ascii_strcasecmp (g_app_info_get_name (a->pAppInfo),
		g_app_info_get_name (b->pAppInfo));
}

static gboolean _on_button_release_menu (GtkWidget *pMenu, GdkEventButton *pEvent,
	GAppInfo *pAppInfo)
{
	// left click
	if (pEvent->button == 1 && pEvent->type == GDK_BUTTON_RELEASE)
		g_app_info_launch (pAppInfo, NULL, NULL, NULL);
	return FALSE; // pass the signal: hide the menu
}

// the GtkLabel should always be the only one in the list but be secure :)
static GtkLabel * _get_label_from_menu_item (GtkWidget *pMenuItem)
{
	GList *pContainerList = gtk_container_get_children (GTK_CONTAINER (pMenuItem));
	GtkWidget *pWidget;
	GList *pList;
	for (pList = pContainerList; pList != NULL; pList = pList->next)
	{
		pWidget = pList->data;
		if (GTK_IS_LABEL (pWidget))
		{
			g_list_free (pContainerList); // only one item
			return (GTK_LABEL (pWidget));
		}
	}
	g_list_free (pContainerList);
	return NULL;
}

// limit to X menu entries? But how many? And it should not have too many results
static void _add_results_in_menu (GldiModuleInstance *myApplet)
{
	// sort list
	s_pEntries = g_list_sort (s_pEntries, (GCompareFunc)_compare_apps);

	gint i = 2; // there are 2 menu entries before
	EntryInfo *pInfo;
	GList *pList;
	for (pList = s_pEntries; pList != NULL; pList = pList->next)
	{
		pInfo = pList->data;
		if (pInfo->pMenuItem) // we already have the menu entry, just change the index
			gtk_menu_reorder_child (GTK_MENU (myData.pMenu),
				pInfo->pMenuItem,
				s_iNbOtherEntries + s_iNbSearchEntries + i);
				// items from the applications menu are hidden are other entries are still not removed
		else
		{
			const gchar *cDescription = g_app_info_get_description (pInfo->pAppInfo);

			// create the new entry: a label, an icon and a tooltip
			if (myConfig.bDisplayDesc)
			{
				gchar *cShortDesc = cDescription ?
					cairo_dock_cut_string (cDescription, 60) :
					NULL;
				gchar *cLabel = g_markup_printf_escaped ("<b>%s</b>\n%s",
					g_app_info_get_name (pInfo->pAppInfo),
					cShortDesc ? cShortDesc : "");
				pInfo->pMenuItem = gtk_image_menu_item_new_with_label (cLabel);
				g_free (cLabel);
				g_free (cShortDesc);

				GtkLabel *pLabel = _get_label_from_menu_item (pInfo->pMenuItem);
				if (pLabel != NULL)
					gtk_label_set_use_markup (pLabel, TRUE);
				else // should not happen... but be secure with Gtk :)
					gtk_menu_item_set_label (GTK_MENU_ITEM (pInfo->pMenuItem),
						g_app_info_get_name (pInfo->pAppInfo));
			}
			else
				pInfo->pMenuItem = gtk_image_menu_item_new_with_label (
					g_app_info_get_name (pInfo->pAppInfo));

			GIcon *pIcon = g_app_info_get_icon (pInfo->pAppInfo);
			if (pIcon)
			{
				GtkWidget *pImage = gtk_image_new_from_gicon (pIcon,
					GTK_ICON_SIZE_LARGE_TOOLBAR);
				_gtk_image_menu_item_set_image (
					GTK_IMAGE_MENU_ITEM (pInfo->pMenuItem), pImage);
			}

			if (cDescription)
				gtk_widget_set_tooltip_text (pInfo->pMenuItem, cDescription);

			gtk_widget_show (pInfo->pMenuItem);

			gtk_menu_shell_append (GTK_MENU_SHELL (myData.pMenu),
				pInfo->pMenuItem);

			// needed to know which menu we want to launch
			g_object_set_data (G_OBJECT (pInfo->pMenuItem), "info", pInfo->pAppInfo);

			// click with the mouse
			g_signal_connect (pInfo->pMenuItem, "button-release-event",
				G_CALLBACK (_on_button_release_menu),
				pInfo->pAppInfo);
			// activate with Return key => _on_key_pressed_menu (entry has focus)
		}
		i++;
	}
	s_iNbSearchEntries = i - 2;

	// if there are results and no selection, select the first entry
	if (s_pEntries != NULL
	   && gtk_menu_shell_get_selected_item (GTK_MENU_SHELL (myData.pMenu)) == NULL)
		gtk_menu_shell_select_item (GTK_MENU_SHELL (myData.pMenu),
			((EntryInfo *)s_pEntries->data)->pMenuItem);
}

// to not recreate a menu entry each time and to not loose the selection
static GtkWidget * _menu_match (GAppInfo *pAppInfo, GList *pEntryList)
{
	EntryInfo *pInfo;
	GList *pList;
	for (pList = pEntryList; pList != NULL; pList = pList->next)
	{
		pInfo = pList->data;
		if (pInfo->pAppInfo == pAppInfo)
		{
			pInfo->bKeepMenu = TRUE;
			return pInfo->pMenuItem;
		}
	}
	return NULL;
}

/* We need to always compare two strings ignoring case of chars because both
 * strings (property and key) can have capital letters
 */
static gboolean _app_match (GAppInfo *pAppInfo, const gchar *key)
{
	int n = strlen (key);
	const gchar *prop = g_app_info_get_executable (pAppInfo);
	if (!prop || g_ascii_strncasecmp (prop, key, n) != 0)
	{
		prop = g_app_info_get_name (pAppInfo);
		if (!prop || g_ascii_strncasecmp (prop, key, n) != 0)
		{
			prop = g_app_info_get_display_name (pAppInfo);
			if (!prop || g_ascii_strncasecmp (prop, key, n) != 0)
			{
				if (n < 3) // check the description when min 3 chars to avoid very very long lists
					return FALSE;
				prop = g_app_info_get_description (pAppInfo);
				if (!prop)
					return FALSE;
				gchar *lower_prop = g_ascii_strdown (prop, -1);
				gchar *lower_key  = g_ascii_strdown (key , -1);
				if (!lower_prop || !lower_key
				   || strstr (lower_prop, lower_key) == NULL)
				{
					g_free (lower_key);
					g_free (lower_prop);
					return FALSE;
				}
				g_free (lower_key);
				g_free (lower_prop);
			}
		}
	}
	return TRUE;
}


static void _create_filtered_list (GAppInfo *pAppInfo, gpointer *data)
{
	const gchar *cText = data[0];
	GList *pList = data[1]; // the previous list
	if (_app_match (pAppInfo, cText))
	{
		EntryInfo *pInfo = g_new (EntryInfo, 1);
		pInfo->pAppInfo = pAppInfo;
		pInfo->bKeepMenu = FALSE;
		pInfo->pMenuItem = _menu_match (pAppInfo, pList);
		s_pEntries = g_list_prepend (s_pEntries, pInfo);
	}
}

static void _remove_results_in_menu (GList *pList, GldiModuleInstance *myApplet)
{
	if (! pList)
		return;

	EntryInfo *pInfo;
	GList *pTmpList;
	while (pList != NULL)
	{
		pInfo = pList->data;
		if (! pInfo->bKeepMenu) // only if we no longer need them for the current menu
			gtk_widget_destroy (pInfo->pMenuItem);
		g_free (pInfo);

		pTmpList = pList->next;
		g_list_free_1 (pList); // free the list (the element)
		pList = pTmpList;
	}
}

///////////////////////
// Applications Menu //
///////////////////////

// a list with the previous application menu (we will just hide them)
static GList *s_pOtherEntries = NULL;

// hide the previous application menu (except the entry and separator)
static void _hide_other_entries (GldiModuleInstance *myApplet)
{
	if (s_pOtherEntries != NULL)
		return;

	GtkWidget *pCurrentWidget;
	GtkContainer *pContainer = GTK_CONTAINER (myData.pMenu);
	GList *pList, *pContainerList = gtk_container_get_children (pContainer);
	// skip the two first entries: GtkEntry + Separator
	for (pList = pContainerList->next->next; pList != NULL; pList = pList->next)
	{
		pCurrentWidget = pList->data;
		gtk_widget_hide (pCurrentWidget);
		s_pOtherEntries = g_list_prepend (s_pOtherEntries, pCurrentWidget);
		s_iNbOtherEntries++;
	}
	g_list_free (pContainerList);
}

// previous elements was hidden, we can free the list and show items
static void _show_other_entries (GldiModuleInstance *myApplet)
{
	GtkWidget *pCurrentWidget;
	GList *pList;
	while (s_pOtherEntries != NULL)
	{
		pCurrentWidget = s_pOtherEntries->data;
		gtk_widget_show (pCurrentWidget);

		pList = s_pOtherEntries->next;
		g_list_free_1 (s_pOtherEntries); // free the list
		s_pOtherEntries = pList;
	}
	s_iNbOtherEntries = 0;
}

///////////
// Entry //
///////////

// modification of the GtkEntry
static gboolean _on_entry_changed (GtkWidget *pEntry, GldiModuleInstance *myApplet)
{
	const gchar *cText = gtk_entry_get_text (GTK_ENTRY (pEntry));
	if (cText && *cText != '\0')
	{
		/* Hide the list after: first it's maybe better to prepare the list
		 * and then modify the menu
		 * We always search in the list of all apps
		 */
		GList *pList = s_pEntries;
		s_pEntries = NULL;
		gpointer data[2];
		data[0] = (gchar *)cText;
		data[1] = pList;
		g_slist_foreach (myData.pApps, (GFunc)_create_filtered_list, data);

		// destroy previous results (only if ! bKeepMenu)
		_remove_results_in_menu (pList, myApplet);
		// hide the previous applications menu (if it's needed)
		_hide_other_entries (myApplet);
		// create/move menu entries
		_add_results_in_menu (myApplet);
	}
	else // re-add (show) the previous applications menu (if needed)
	{
		_remove_results_in_menu (s_pEntries, myApplet);
		s_pEntries = NULL;
		_show_other_entries (myApplet);
	}

	return FALSE;
}

static void _launch_app_of_selected_item (GtkWidget *pMenu)
{
	GtkWidget *pMenuItem = gtk_menu_shell_get_selected_item (GTK_MENU_SHELL (pMenu));
	if (pMenuItem == myData.pEntry) // 'entry' selected
		pMenuItem = ((EntryInfo *)s_pEntries->data)->pMenuItem; // select the first entry

	if (pMenuItem != NULL)
	{
		GAppInfo *pAppInfo = g_object_get_data (G_OBJECT (pMenuItem), "info");
		g_app_info_launch (pAppInfo, NULL, NULL, NULL);
	}
	else // no item, we launch the command
	{
		cairo_dock_launch_command (gtk_entry_get_text (GTK_ENTRY (myData.pEntry)));
		gtk_widget_hide (myData.pMenu);
	}
}

// needed to redirect the signal (and launch the selected app)
static gboolean _on_key_pressed_menu (GtkWidget *pMenu, GdkEventKey *pEvent,
	GldiModuleInstance *myApplet)
{
	// redirect the signal to the entry
	g_signal_emit_by_name (myData.pEntry, "key-press-event", pEvent, myApplet);

	// Launch when we list search result entries
	if (pEvent->keyval == GDK_KEY_Return && s_pOtherEntries != NULL)
		_launch_app_of_selected_item (pMenu);

	// pass the signal to the menu (for navigation by arrows)
	return FALSE;
}

static void _on_menu_deactivated (GtkWidget *pMenu, G_GNUC_UNUSED gpointer data)
{
	// modify the menu (if needed) to have the applications menu next time
	_remove_results_in_menu (s_pEntries, myApplet);
	s_pEntries = NULL;
	gtk_entry_set_text (GTK_ENTRY (myData.pEntry), "");
}

void cd_menu_append_entry (void)
{
	// menu item at the top of the menu with a GtkImage and a GtkEntry
	GtkWidget *pMenuItem = gtk_image_menu_item_new ();

	GtkWidget *pImage = gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_LARGE_TOOLBAR);
	_gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), pImage);

	GtkWidget *pEntry = gtk_entry_new ();
	gtk_container_add (GTK_CONTAINER (pMenuItem), pEntry);
	
	g_signal_connect (pEntry, "changed",
		G_CALLBACK (_on_entry_changed),
		myApplet);  // to find matching apps while the user is typing
	g_signal_connect (myData.pMenu, "key-press-event",
		G_CALLBACK (_on_key_pressed_menu),
		myApplet);  // to redirect the signal to the event, or it won't get it.

	g_signal_connect (G_OBJECT (myData.pMenu),
		"deactivate",
		G_CALLBACK (_on_menu_deactivated),
		NULL);
	
	gtk_widget_show_all (pMenuItem);
	gtk_menu_shell_append (GTK_MENU_SHELL (myData.pMenu), pMenuItem);
	myData.pEntry = pEntry;  // make it global so that we can grab the focus before we pop the menu up

	// a separator
	pMenuItem = gtk_separator_menu_item_new ();
	gtk_widget_show (pMenuItem);
	gtk_menu_shell_append (GTK_MENU_SHELL (myData.pMenu), pMenuItem);
}

// free the list of result and the list of the applications menu
void cd_menu_free_entry (void)
{
	if (s_pEntries)
		g_list_free_full (s_pEntries, g_free);
	if (s_pOtherEntries)
		g_list_free (s_pOtherEntries);
}
