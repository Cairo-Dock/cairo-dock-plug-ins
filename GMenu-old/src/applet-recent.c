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

// Adapted from the Gnome-panel for Cairo-Dock by Fabrice Rey (for any bug report, please mail me to fabounet@glx-dock.org)

#include "applet-struct.h"
#include "applet-recent.h"
#include "applet-menu-callbacks.h"

static void
recent_documents_activate_cb (GtkRecentChooser *chooser,
			      gpointer          data)
{
	GtkRecentInfo *recent_info = gtk_recent_chooser_get_current_item (chooser);
	const char *uri = gtk_recent_info_get_uri (recent_info);
	cd_debug ("%s (%s) : %s", __func__, uri, gtk_recent_info_get_display_name(recent_info));
	cairo_dock_fm_launch_uri (uri);
	gtk_recent_info_unref (recent_info);
}

static void
panel_recent_manager_changed_cb (GtkRecentManager *manager,
				 GtkWidget        *menu_item)
{
	int size;

	g_object_get (manager, "size", &size, NULL);

	gtk_widget_set_sensitive (menu_item, size > 0);
}

void cd_menu_append_recent_to_menu (GtkWidget *top_menu, GldiModuleInstance *myApplet)
{
	//\_____________ On construit une entree de sous-menu qu'on insere dans le menu principal.
	if (myData.pRecentMenuItem == NULL)
	{
		GtkWidget *pSeparator = gtk_separator_menu_item_new ();
		gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), pSeparator);
		
		GtkWidget *pMenuItem = gtk_image_menu_item_new_with_label (D_("Recent Documents"));

		const gchar *cIconPath = cairo_dock_search_icon_s_path ("document-open-recent", myData.iPanelDefaultMenuIconSize);
		if (cIconPath == NULL)
			cIconPath = MY_APPLET_SHARE_DATA_DIR"/icon-recent.png";

		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cIconPath, myData.iPanelDefaultMenuIconSize, myData.iPanelDefaultMenuIconSize, NULL);
		GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
		g_object_unref (pixbuf);
		_gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), image);
		gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), pMenuItem);
		gtk_widget_show_all (pMenuItem);
		myData.pRecentMenuItem = pMenuItem;
	}
	
	//\_____________ On construit le menu des fichiers recents.
	GtkWidget *recent_menu = gtk_recent_chooser_menu_new_for_manager (myData.pRecentManager);
	
	gtk_recent_chooser_set_show_icons (GTK_RECENT_CHOOSER (recent_menu), TRUE);
	gtk_recent_chooser_set_local_only (GTK_RECENT_CHOOSER (recent_menu), FALSE);
	gtk_recent_chooser_set_show_tips (GTK_RECENT_CHOOSER (recent_menu), TRUE);
	gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (recent_menu), GTK_RECENT_SORT_MRU);  // most recently used
	gtk_recent_chooser_set_limit (GTK_RECENT_CHOOSER (recent_menu), myConfig.iNbRecentItems);
	myData.iNbRecentItems = myConfig.iNbRecentItems;
	
	if (myData.pRecentFilter != NULL)
	{
		gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (recent_menu), myData.pRecentFilter);
	}
	
	//\_____________ les signaux
	g_signal_connect (GTK_RECENT_CHOOSER (recent_menu),
		"item-activated",
		G_CALLBACK (recent_documents_activate_cb),
		NULL);

	g_signal_connect_object (myData.pRecentManager, "changed",
		G_CALLBACK (panel_recent_manager_changed_cb),
		 myData.pRecentMenuItem, 0);
	
	//\_____________ On l'insere dans notre entree.
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (myData.pRecentMenuItem), recent_menu);
	
	int size = 0;
	g_object_get (myData.pRecentManager, "size", &size, NULL);
	gtk_widget_set_sensitive (myData.pRecentMenuItem, size > 0);
}



static void _on_answer_clear_recent (int iClickedButton, GtkWidget *pInteractiveWidget, GldiModuleInstance *myApplet, CairoDialog *pDialog)
{
	CD_APPLET_ENTER;
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		gtk_recent_manager_purge_items (myData.pRecentManager, NULL);
	}
	CD_APPLET_LEAVE ();
}
void cd_menu_clear_recent (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	gldi_dialog_show_with_question (D_("Clear the list of the recently used documents?"),
		myIcon, myContainer,
		"same icon",
		(CairoDockActionOnAnswerFunc) _on_answer_clear_recent, myApplet, (GFreeFunc)NULL);
}



static gboolean _recent_uri_filter (const GtkRecentFilterInfo *filter_info, GldiModuleInstance *myApplet)
{
	g_return_val_if_fail (myConfig.cRecentRootDirFilter != NULL, TRUE);
	return (filter_info->uri != NULL && strncmp (myConfig.cRecentRootDirFilter, filter_info->uri, strlen (myConfig.cRecentRootDirFilter)) == 0);
}
void cd_menu_init_recent (GldiModuleInstance *myApplet)
{
	if (myData.pRecentManager == NULL)
		myData.pRecentManager = gtk_recent_manager_get_default ();
	
	if (myConfig.cRecentRootDirFilter != NULL && myData.pRecentFilter == NULL)
	{
		myData.pRecentFilter = gtk_recent_filter_new ();
		gtk_recent_filter_add_custom (myData.pRecentFilter,
			GTK_RECENT_FILTER_URI,
			(GtkRecentFilterFunc) _recent_uri_filter,
			myApplet,
			NULL);
	}
}

void cd_menu_reset_recent (GldiModuleInstance *myApplet)
{
	if (myData.pRecentFilter != NULL)
	{
		if (myData.pRecentMenuItem != NULL)
			gtk_recent_chooser_remove_filter (GTK_RECENT_CHOOSER (myData.pRecentMenuItem), myData.pRecentFilter);
		g_object_unref (myData.pRecentFilter);  // verifier que ca ne pose pas de probleme avec le remove d'avant.
		myData.pRecentFilter = NULL;
	}
}