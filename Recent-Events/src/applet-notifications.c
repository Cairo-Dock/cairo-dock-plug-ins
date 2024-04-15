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

#include "applet-struct.h"
#include "applet-search.h"
#include "applet-dialog.h"
#include "applet-notifications.h"


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	cd_toggle_dialog ();
CD_APPLET_ON_CLICK_END


//\___________ Same as ON_CLICK, but with middle-click.
/*CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	
CD_APPLET_ON_MIDDLE_CLICK_END*/


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
static GtkWidget *s_pMenu = NULL;
static GtkWidget *s_pSubMenu = NULL;
static GList *s_pEventList = NULL;
void cd_recent_events_reset_uri_list (void)
{
	s_pEventList = NULL;
	return;
	if (s_pEventList != NULL)
	{
		g_list_foreach (s_pEventList, (GFunc) g_free, NULL);
		g_list_free (s_pEventList);
		s_pEventList = NULL;
	}
}
static gboolean _on_delete_menu (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	cd_debug ("*** menu deleted");
	s_pMenu = NULL;
	s_pSubMenu = NULL;
	cd_recent_events_reset_uri_list ();
	return FALSE;
}
static void _open_file (GtkMenuItem *menu_item, gchar *cCommand)
{
	cd_debug ("%s (%s)", __func__, cCommand);
	
	cairo_dock_launch_command (cCommand);
	
	cd_recent_events_reset_uri_list ();
}
static void _on_delete_events (int iNbEvents, gpointer data)
{
	if (iNbEvents > 0)
	{
		gldi_dialog_show_temporary_with_icon_printf (D_("%d event(s) deleted"), myIcon, myContainer, 3e3, "same icon", iNbEvents);
	}
	if (iNbEvents != 0)
	{
		cd_trigger_search ();
	}
	CD_APPLET_STOP_DEMANDING_ATTENTION;
}
static void _clear_today_events (GtkMenuItem *menu_item, gpointer data)
{
	cd_delete_recent_events (1, (CDOnDeleteEventsFunc)_on_delete_events, data);
	CD_APPLET_DEMANDS_ATTENTION ("pulse", 30);
}
static void _clear_all_events (GtkMenuItem *menu_item, gpointer data)
{
	cd_delete_recent_events (1e6, (CDOnDeleteEventsFunc)_on_delete_events, data);  // in Maverick we get the following error: ZeitgeistEngine instance has no attribute 'delete_log'. so we just delete as much events as possible.
	CD_APPLET_DEMANDS_ATTENTION ("pulse", 30);
}
static void _on_find_related_events (ZeitgeistResultSet *pEvents, Icon *pIcon)
{
	cd_debug ("%s ()", __func__);
	cd_recent_events_reset_uri_list ();
	if (s_pMenu == NULL || s_pSubMenu == NULL)
		return;
	
	ZeitgeistEvent     *event;
	ZeitgeistSubject   *subject;
	gint                i,n;
	GtkWidget *pSubMenu = s_pSubMenu;
	const gchar *cEventURI;
	gchar *cName = NULL, *cURI = NULL, *cIconName = NULL, *cIconPath;
	gchar *cCommand;
	double fOrder;
	int iVolumeID;
	gboolean bIsDirectory;
	gint iDesiredIconSize = cairo_dock_search_icon_size (GTK_ICON_SIZE_LARGE_TOOLBAR); // 24px
	GHashTable *pHashTable = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);  // used to prevent doubles
	
	if (!zeitgeist_result_set_has_next (pEvents))
	{
		CD_APPLET_ADD_IN_MENU (_("No related files"), NULL, s_pSubMenu);
	}
	else do {
		#ifdef ZEITGEIST_1_0
		event = zeitgeist_result_set_next (pEvents);
		#else
		event = zeitgeist_result_set_next_value (pEvents);
		#endif
		n = zeitgeist_event_num_subjects (event);
		for (i = 0; i < n; i++)
		{
			subject = zeitgeist_event_get_subject (event, i);
			cEventURI = zeitgeist_subject_get_uri (subject);
			if (g_hash_table_lookup_extended  (pHashTable, cEventURI, NULL, NULL))
				continue;
			cd_debug (" + %s", cEventURI);

			
			gchar *cPath = g_filename_from_uri (cEventURI, NULL, NULL);

			// check it's a file and if yes, if it exists
			if (strncmp (cEventURI, "file://", 7) == 0 && ! g_file_test (cPath, G_FILE_TEST_EXISTS))
			{
				/*
				 * If it doesn't exist, we don't add it in the menu
				 * (we can set the widget as insensitive but why?
				 *  If we add it, it's just an useless entry)
				 */
				g_hash_table_insert (pHashTable, (gchar*)cEventURI, NULL);  // since we've checked it, insert it, even if we don't display it.
				g_free (cPath);
				continue;
			}
			
			cairo_dock_fm_get_file_info (cEventURI, &cName, &cURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, 0);
			
			cCommand = g_strdup_printf ("%s \"%s\"", pIcon->cCommand, cPath); // some programs don't support URI, so we feed them with path.
			g_free (cPath);
			s_pEventList = g_list_prepend (s_pEventList, cCommand);
			
			cIconPath = cairo_dock_search_icon_s_path (cIconName, iDesiredIconSize);
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (zeitgeist_subject_get_text (subject), cIconPath, _open_file, pSubMenu, cCommand);
			g_free (cIconPath);
			g_free (cIconName);
			cIconName = NULL;
			g_free (cName);
			cName = NULL;
			g_free (cURI);
			cURI = NULL;
			
			g_hash_table_insert (pHashTable, (gchar*)cEventURI, NULL);  // cEventURI stays valid in this function.
		}
	} while (zeitgeist_result_set_has_next (pEvents));
	g_hash_table_destroy (pHashTable);
	
	if (pSubMenu)
	{
		gtk_widget_show_all (pSubMenu);  // sinon des fois il n'apparait pas au 1er survol de son entree.
		gtk_widget_show_all (s_pMenu);
		gtk_menu_reposition (GTK_MENU(s_pMenu));  // reposition the menu, since it has already appeared and its height has changed; if Zeitgeist responds quickly enough, it's actually unnoticeable
	}
	cd_debug ("items added");
}
CD_APPLET_ON_BUILD_MENU_PROTO
{
	cd_debug ("%s (%s...)", __func__, CD_APPLET_CLICKED_ICON && CD_APPLET_CLICKED_ICON->pMimeTypes ?CD_APPLET_CLICKED_ICON->pMimeTypes[0] : "");
	CD_APPLET_ENTER;
	
	if (CD_APPLET_CLICKED_ICON != NULL)
	{
		if (CD_APPLET_CLICKED_ICON == myIcon)
		{
			CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);  // because we are called before the main callback.
			
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Delete today's events"), GLDI_ICON_NAME_CLEAR, _clear_today_events, CD_APPLET_MY_MENU, myApplet);
			
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Delete all events"), GLDI_ICON_NAME_DELETE, _clear_all_events, CD_APPLET_MY_MENU, myApplet);
		}
		else if (CD_APPLET_CLICKED_ICON->pMimeTypes != NULL)
		{
			s_pMenu = pAppletMenu;
			CD_APPLET_ADD_SEPARATOR_IN_MENU (s_pMenu);
			s_pSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Recent files"), s_pMenu, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);  // GLDI_ICON_NAME_FILE
			cd_find_recent_related_files ((const gchar **)CD_APPLET_CLICKED_ICON->pMimeTypes, (CDOnGetEventsFunc)_on_find_related_events, CD_APPLET_CLICKED_ICON);
			g_signal_connect (G_OBJECT (pAppletMenu), "destroy", G_CALLBACK (_on_delete_menu), NULL);
		}
	}
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}


void cd_on_shortkey (const char *keystring, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_toggle_dialog ();
	CD_APPLET_LEAVE ();
}
