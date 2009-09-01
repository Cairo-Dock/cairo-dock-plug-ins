/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Adapted from the Gnome-panel for Cairo-Dock by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "applet-struct.h"
#include "applet-recent.h"
#include "applet-menu-callbacks.h"

static void
recent_documents_activate_cb (GtkRecentChooser *chooser,
			      gpointer          data)
{
	GtkRecentInfo *recent_info = gtk_recent_chooser_get_current_item (chooser);
	const char *uri = gtk_recent_info_get_uri (recent_info);
	g_print ("%s (%s) : %s\n", __func__, uri, gtk_recent_info_get_display_name(recent_info));
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

void cd_menu_append_recent_to_menu (GtkWidget *top_menu, CairoDockModuleInstance *myApplet)
{
	//\_____________ On construit une entree de sous-menu qu'on insere dans le menu principal.
	if (myData.pRecentMenuItem == NULL)
	{
		GtkWidget *pSeparator = gtk_separator_menu_item_new ();
		gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), pSeparator);
		
		GtkWidget *pMenuItem = gtk_image_menu_item_new_with_label (D_("Recent Documents"));
		const gchar *cIconPath = MY_APPLET_SHARE_DATA_DIR"/icon-recent.png";
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cIconPath, 24, 24, NULL);
		GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
		g_object_unref (pixbuf);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), image);
		gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), pMenuItem);
		gtk_widget_show_all (pMenuItem);
		myData.pRecentMenuItem = pMenuItem;
	}
	
	//\_____________ On construit le menu des fichiers recents.
	GtkWidget *recent_menu = gtk_recent_chooser_menu_new_for_manager (myData.pRecentManager);
	gtk_recent_chooser_set_show_icons (GTK_RECENT_CHOOSER (recent_menu), myConfig.bHasIcons);
	if (myData.pRecentFilter != NULL)
	{
		gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (recent_menu), myData.pRecentFilter);
	}
	
	//\_____________ les signaux
	g_signal_connect (G_OBJECT (recent_menu), "button_press_event",
		  G_CALLBACK (menu_dummy_button_press_event), NULL);  // utile ?
	
	g_signal_connect (GTK_RECENT_CHOOSER (recent_menu),
		"item-activated",
		G_CALLBACK (recent_documents_activate_cb),
		NULL);

	g_signal_connect_object (myData.pRecentManager, "changed",
		G_CALLBACK (panel_recent_manager_changed_cb),
		 myData.pRecentMenuItem, 0);
	
	//\_____________ On le personnalise un peu.
	gtk_recent_chooser_set_local_only (GTK_RECENT_CHOOSER (recent_menu),
		 FALSE);
	gtk_recent_chooser_set_show_tips (GTK_RECENT_CHOOSER (recent_menu),
		TRUE);
	gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (recent_menu),
		GTK_RECENT_SORT_MRU);  // most recently used
	
	//\_____________ On l'insere dans notre entree.
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (myData.pRecentMenuItem), recent_menu);
	
	int size = 0;
	g_object_get (myData.pRecentManager, "size", &size, NULL);
	gtk_widget_set_sensitive (myData.pRecentMenuItem, size > 0);
}



void cd_menu_clear_recent (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	int iAnswer = cairo_dock_ask_question_and_wait (D_("Clear the list of the recently used documents ?"), myIcon, myContainer);
	if (iAnswer == GTK_RESPONSE_YES)
	{
		gtk_recent_manager_purge_items (myData.pRecentManager, NULL);
	}
}



static gboolean _recent_uri_filter (const GtkRecentFilterInfo *filter_info, CairoDockModuleInstance *myApplet)
{
	g_return_val_if_fail (myConfig.cRecentRootDirFilter != NULL, TRUE);
	return (filter_info->uri != NULL && strncmp (myConfig.cRecentRootDirFilter, filter_info->uri, strlen (myConfig.cRecentRootDirFilter)) == 0);
}
void cd_menu_init_recent (CairoDockModuleInstance *myApplet)
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
		if (myConfig.iRecentAge != 0)
			gtk_recent_filter_add_age (myData.pRecentFilter, myConfig.iRecentAge);
	}
}

void cd_menu_reset_recent (CairoDockModuleInstance *myApplet)
{
	if (myData.pRecentFilter != NULL)
	{
		if (myData.pRecentMenuItem != NULL)
			gtk_recent_chooser_remove_filter (GTK_RECENT_CHOOSER (myData.pRecentMenuItem), myData.pRecentFilter);
		g_object_unref (myData.pRecentFilter);  // verifier que ca ne pose pas de probleme avec le remove d'avant.
		myData.pRecentFilter = NULL;
	}
}
