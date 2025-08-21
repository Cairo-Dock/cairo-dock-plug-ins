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
#include <implementations/cairo-dock-wayland-manager.h> // gldi_wayland_manager_have_layer_shell


typedef struct _EntryInfo {
	GDesktopAppInfo  *pAppInfo;
	GtkWidget *pMenuItem;
	gboolean   bKeepMenu; // flag to not destroy the menu item if it's reused after
	} EntryInfo;

static GList *s_pEntries = NULL;    // a list with all matches apps
static gint s_iNbSearchEntries = 0; // the number of elements in this list (to not iterate over the whole list)
static gint s_iNbOtherEntries = 0;  // the number of hidden entries (applications menu)
static GtkWidget *s_pLaunchCommand = NULL; // widget to launch the command

static gint _compare_apps (const EntryInfo *a, const EntryInfo *b)
{
	// ignore cases: some apps don't have capital letters for the first char
	return g_ascii_strcasecmp (g_app_info_get_name (G_APP_INFO (a->pAppInfo)),
		g_app_info_get_name (G_APP_INFO (b->pAppInfo)));
}

static gboolean _on_button_release_menu (GtkWidget *pMenuItem, GdkEventButton *pEvent,
	GDesktopAppInfo *pAppInfo)
{
	// need to explicitly disable the tooltip on Wayland to avoid a race condition (see below)
	if (gldi_wayland_manager_have_layer_shell ())
		gtk_widget_set_tooltip_text (pMenuItem, NULL);
	cairo_dock_launch_app_info (pAppInfo);
	return FALSE; // pass the signal: hide the menu
}

static void _on_map_entry (GtkWidget *pMenuItem, gpointer data)
{
	if (data) gtk_widget_set_tooltip_text (pMenuItem, (const gchar*)data);
}

static void _weak_free_helper (gpointer ptr, GObject*)
{
	g_free (ptr);
}

// the GtkLabel should always be the only one in the list but be secure :)
static GtkLabel * _get_label_from_menu_item (GtkWidget *pMenuItem)
{
	GtkWidget *pWidget = gtk_bin_get_child (GTK_BIN (pMenuItem));
	if (GTK_IS_LABEL (pWidget))
	{
		return GTK_LABEL (pWidget);
	}
	else
	{
		return NULL;
	}
}

// limit to X menu entries? But how many? And it should not have too many results
static void _add_results_in_menu (GldiModuleInstance *myApplet)
{
	// sort list
	s_pEntries = g_list_sort (s_pEntries, (GCompareFunc)_compare_apps);

	gint i = 0;
	EntryInfo *pInfo;
	GList *pList;
	for (pList = s_pEntries; pList != NULL; pList = pList->next)
	{
		pInfo = pList->data;
		if (pInfo->pMenuItem) // we already have the menu entry, just change the index
			gtk_menu_reorder_child (GTK_MENU (myData.pMenu),
				pInfo->pMenuItem,
				s_iNbOtherEntries + s_iNbSearchEntries + i);
				/* s_iNbOtherEntries: items from the applications menu are hidden
				 * s_iNbSearchEntries: other entries are still not removed
				 * i: current entry
				 * + 3: there are 2 menu entries before + LaunchCommand (hidden)
				 */
		else
		{
			const gchar *cDescription = g_app_info_get_description (G_APP_INFO (pInfo->pAppInfo));

			// create the new entry: a label, an icon and a tooltip
			if (myConfig.bDisplayDesc)
			{
				gchar *cShortDesc = cDescription ?
					cairo_dock_cut_string (cDescription, 60) :
					NULL;
				gchar *cLabel = g_markup_printf_escaped ("<b>%s</b>\n%s",
					g_app_info_get_display_name (G_APP_INFO (pInfo->pAppInfo)),
					cShortDesc ? cShortDesc : "");
				pInfo->pMenuItem = gldi_menu_item_new (cLabel, "");
				g_free (cLabel);
				g_free (cShortDesc);

				GtkLabel *pLabel = _get_label_from_menu_item (pInfo->pMenuItem);
				if (pLabel != NULL)
					gtk_label_set_use_markup (pLabel, TRUE);
				else // should not happen... but be secure with Gtk :)
					gtk_menu_item_set_label (GTK_MENU_ITEM (pInfo->pMenuItem),
						g_app_info_get_display_name (G_APP_INFO (pInfo->pAppInfo)));
			}
			else
				pInfo->pMenuItem = gldi_menu_item_new (g_app_info_get_name (G_APP_INFO (pInfo->pAppInfo)), "");

			GIcon *pIcon = g_app_info_get_icon (G_APP_INFO (pInfo->pAppInfo));
			if (pIcon)
			{
				GtkWidget *pImage = gtk_image_new_from_gicon (pIcon,
					GTK_ICON_SIZE_LARGE_TOOLBAR);
				gtk_image_set_pixel_size (GTK_IMAGE (pImage),
					myData.iPanelDefaultMenuIconSize); // force size
				gldi_menu_item_set_image (pInfo->pMenuItem, pImage);
			}

			if (cDescription)
			{
				if (gldi_wayland_manager_have_layer_shell ())
				{
					/** Need to manage the tooltip ourselves, see e.g.
					 * https://github.com/wmww/gtk-layer-shell/issues/207 */
					gchar *tmp = g_strdup (cDescription);
					g_signal_connect (G_OBJECT (pInfo->pMenuItem), "map", G_CALLBACK (_on_map_entry), tmp);
					g_object_weak_ref (G_OBJECT (pInfo->pMenuItem), _weak_free_helper, tmp);
				}
				else gtk_widget_set_tooltip_text (pInfo->pMenuItem, cDescription);
			}

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
	s_iNbSearchEntries = i;

	// no entry: Launch this command
	if (s_pEntries == NULL)
	{
		gtk_widget_show (s_pLaunchCommand);
		gtk_menu_shell_select_item (GTK_MENU_SHELL (myData.pMenu), s_pLaunchCommand);
	}
	else
	{
		gtk_widget_hide (s_pLaunchCommand);
		// if there are results and no selection, select the first entry
		GtkWidget *pMenuItem = gtk_menu_shell_get_selected_item (
			GTK_MENU_SHELL (myData.pMenu));
		if (pMenuItem == NULL || pMenuItem == s_pLaunchCommand)
			gtk_menu_shell_select_item (GTK_MENU_SHELL (myData.pMenu),
				((EntryInfo *)s_pEntries->data)->pMenuItem);
	}
}

// to not recreate a menu entry each time and to not loose the selection
static GtkWidget * _menu_match (GDesktopAppInfo *pAppInfo, GList *pEntryList)
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
static gboolean _app_match (GDesktopAppInfo *pAppInfo, const gchar *key)
{
	int n = strlen (key);
	const gchar *prop = g_app_info_get_executable (G_APP_INFO (pAppInfo)); // transmission
	if (!prop || g_ascii_strncasecmp (prop, key, n) != 0)
	{
		prop = g_app_info_get_name (G_APP_INFO (pAppInfo)); // Transmission
		if (!prop || g_ascii_strncasecmp (prop, key, n) != 0)
		{
			gchar *lower_key = g_ascii_strdown (key , -1);
			if (!lower_key)
				return FALSE;

			prop = g_app_info_get_display_name (G_APP_INFO (pAppInfo)); // BitTorrent Client Transmission
			gchar *lower_prop;
			if (prop) // avoid warnings even if it should not happen
				lower_prop = g_ascii_strdown (prop, -1);
			else
				lower_prop = NULL;

			gboolean bDisplayMatch;
			if (! lower_prop)
				bDisplayMatch = FALSE;
			else if (n < 3) // 1 or 2 chars: compare the first chars
				bDisplayMatch = strncmp (lower_prop, lower_key, n) == 0;
			else // locate a substring in this property (if there is at least 3 chars)
				bDisplayMatch = strstr (lower_prop, lower_key) != NULL;

			if (!bDisplayMatch)
			{
				g_free (lower_prop);
				if (n < 3) // check the description when min 3 chars to avoid very very long lists
				{
					g_free (lower_key);
					return FALSE;
				}
				prop = g_app_info_get_description (G_APP_INFO (pAppInfo));
				if (!prop)
				{
					g_free (lower_key);
					return FALSE;
				}
				lower_prop = g_ascii_strdown (prop, -1);
				if (!lower_prop || strstr (lower_prop, lower_key) == NULL)
				{
					g_free (lower_key);
					g_free (lower_prop);
					return FALSE;
				}
			}
			g_free (lower_prop);
			g_free (lower_key);
		}
	}
	return TRUE;
}


static void _create_filtered_list (GDesktopAppInfo *pAppInfo, gpointer *data)
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

	// Insert Launch this command
	gtk_menu_shell_insert (GTK_MENU_SHELL (myData.pMenu), s_pLaunchCommand, 2);

	GtkWidget *pCurrentWidget;
	GtkContainer *pContainer = GTK_CONTAINER (myData.pMenu);
	GList *pList, *pContainerList = gtk_container_get_children (pContainer);
	// skip the three first entries: GtkEntry + Separator + Launch this command
	for (pList = pContainerList->next->next->next; pList != NULL; pList = pList->next)
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
	// remove s_pLaunchCommand
	gtk_container_remove (GTK_CONTAINER (myData.pMenu), s_pLaunchCommand);
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

	// reposition the menu (e.g. if we are on the bottom/right)
	gtk_menu_reposition (GTK_MENU (myData.pMenu));

	return FALSE;
}

// needed to know if we press 'return' key when the entry container is selected
static GtkWidget *s_pEntryContainer = NULL;

static void _launch_app_of_selected_item (GtkWidget *pMenu)
{
	GtkWidget *pMenuItem = gtk_menu_shell_get_selected_item (GTK_MENU_SHELL (pMenu));

	if (pMenuItem == s_pEntryContainer) // 'entry' selected
		pMenuItem = ((EntryInfo *)s_pEntries->data)->pMenuItem; // select the first item

	if (pMenuItem != NULL && pMenuItem != s_pLaunchCommand)
	{
		GDesktopAppInfo *pAppInfo = g_object_get_data (G_OBJECT (pMenuItem), "info");
		cairo_dock_launch_app_info (pAppInfo);
	}
	else // no item or s_pLaunchCommand, we launch the command
	{
		// note: we have to parse this as a command line, since there could be arguments given
		cairo_dock_launch_command_full (gtk_entry_get_text (GTK_ENTRY (myData.pEntry)), NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
		gtk_widget_hide (myData.pMenu);
	}
}

// needed to redirect the signal (and launch the selected app)
static gboolean _on_key_pressed_menu (GtkWidget *pMenu, GdkEventKey *pEvent,
	GldiModuleInstance *myApplet)
{
	if (s_pOtherEntries != NULL)
	{
		switch (pEvent->keyval)
		{
			// Launch when we list search result entries
			case GDK_KEY_Return :
			case GDK_KEY_KP_Enter : // second Enter
				_launch_app_of_selected_item (pMenu);
			break ;
			// space key when searching, do not deactivate the menu: command or desc.
			case GDK_KEY_space :
			{
				int pos = gtk_editable_get_position (GTK_EDITABLE (myData.pEntry));
				gtk_editable_insert_text (GTK_EDITABLE (myData.pEntry), " ", 1, &pos);
				gtk_editable_set_position (GTK_EDITABLE (myData.pEntry), pos);
				return TRUE;
			}
		}
	}

	// pass the signal to the menu (for navigation by arrows)
	return FALSE;
}

static void _on_menu_deactivated (GtkWidget *pMenu, G_GNUC_UNUSED gpointer data)
{
	gtk_entry_set_text (GTK_ENTRY (myData.pEntry), "");
	// remove_result + show_other entries: see _on_entry_changed, text == '\0'
}

static gboolean _on_button_release_launch_command (G_GNUC_UNUSED GtkWidget *pMenu,
	GdkEventButton *pEvent, G_GNUC_UNUSED gpointer data)
{
	cairo_dock_launch_command_full (gtk_entry_get_text (GTK_ENTRY (myData.pEntry)), NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
	return FALSE; // pass the signal: hide the menu
}

void cd_menu_append_entry (void)
{
	// menu item at the top of the menu with a GtkImage and a GtkEntry
	GtkWidget *pMenuItem = gldi_menu_item_new_full (NULL, GLDI_ICON_NAME_EXECUTE, FALSE, GTK_ICON_SIZE_LARGE_TOOLBAR);
	
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
	s_pEntryContainer = pMenuItem;

	// a separator
	pMenuItem = gtk_separator_menu_item_new ();
	gtk_widget_show (pMenuItem);
	gtk_menu_shell_append (GTK_MENU_SHELL (myData.pMenu), pMenuItem);

	// Launch this command (create the widget but we don't insert it now)
	s_pLaunchCommand = gldi_menu_item_new_full (D_("Launch this command"), GLDI_ICON_NAME_EXECUTE, FALSE, GTK_ICON_SIZE_LARGE_TOOLBAR);
	g_signal_connect (s_pLaunchCommand, "button-release-event",
		G_CALLBACK (_on_button_release_launch_command), NULL);
	g_object_ref (s_pLaunchCommand);
}

// free the list of result and the list of the applications menu
void cd_menu_free_entry (void)
{
	if (s_pEntries)
		g_list_free_full (s_pEntries, g_free);
	if (s_pOtherEntries)
		g_list_free (s_pOtherEntries);
	if (s_pLaunchCommand)
		g_object_unref (s_pLaunchCommand);
}
