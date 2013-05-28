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

#include <string.h>
#include <cairo-dock.h>
#include <gdk/gdkkeysyms.h>

#include "applet-struct.h"
#include "applet-menu.h"
#include "applet-util.h"
#include "applet-recent.h"
#include "applet-init.h"
#include "applet-menu-callbacks.h"

static guint load_icons_id = 0;
static GList *icons_to_load = NULL;
static GList *icons_to_add = NULL;


void handle_gmenu_tree_changed (GMenuTree *tree,
			   GtkWidget *menu)
{
	cd_message ("%s ()", __func__);
	
	// easy way: rebuild the whole menu.
	if (myData.pMenu != NULL)
	{
		gtk_widget_destroy (myData.pMenu);
		myData.pMenu = NULL;
		myData.pRecentMenuItem = NULL;
	}
	
	myData.pMenu = create_main_menu (myApplet);
	cd_gmenu_preload_icon ();
}

void remove_gmenu_tree_monitor (GtkWidget *menu,
			  GMenuTree  *tree)
{
	cd_message ("%s (%x)", __func__, tree);
	gmenu_tree_remove_monitor (tree,
				  (GMenuTreeChangedFunc) handle_gmenu_tree_changed,
				  menu);
}


void remove_submenu_to_display_idle (gpointer data)
{
	guint idle_id = GPOINTER_TO_UINT (data);

	g_source_remove (idle_id);
}


gboolean submenu_to_display_in_idle (gpointer data)
{
	GtkWidget *menu = GTK_WIDGET (data);
	cd_message ("%s (%x)", __func__, menu);

	g_object_set_data (G_OBJECT (menu), "panel-menu-idle-id", NULL);

	submenu_to_display (menu);

	return FALSE;
}

void submenu_to_display (GtkWidget *menu)
{
	cd_message ("%s (%x)", __func__, menu);
	GMenuTree           *tree;
	GMenuTreeDirectory  *directory;
	const char          *menu_path;
	void               (*append_callback) (GtkWidget *, gpointer);
	gpointer             append_data;

	if (!g_object_get_data (G_OBJECT (menu), "panel-menu-needs-loading"))
	{
		cd_debug ("needs no loading");
		return;
	}

	g_object_set_data (G_OBJECT (menu), "panel-menu-needs-loading", NULL);

	directory = g_object_get_data (G_OBJECT (menu),
				       "panel-menu-tree-directory");
	if (!directory) {
		menu_path = g_object_get_data (G_OBJECT (menu),
					       "panel-menu-tree-path");
		cd_debug ("n'est pas un directory, menu_path : %s", menu_path);
		if (!menu_path)
		{
			cd_warning ("menu_path is empty");
			return;
		}
		
		tree = g_object_get_data (G_OBJECT (menu), "panel-menu-tree");
		if (!tree)
		{
			cd_warning ("no tree found in data");
			return;
		}
		directory = gmenu_tree_get_directory_from_path (tree,
								menu_path);

		g_object_set_data_full (G_OBJECT (menu),
					"panel-menu-tree-directory",
					directory,
					(GDestroyNotify) gmenu_tree_item_unref);
	}
	//g_print ("%s ()\n", __func__);
	if (directory)
		populate_menu_from_directory (menu, directory);

	append_callback = g_object_get_data (G_OBJECT (menu),
					     "panel-menu-append-callback");
	append_data     = g_object_get_data (G_OBJECT (menu),
					     "panel-menu-append-callback-data");
	if (append_callback)
		append_callback (menu, append_data);
}


// == cairo_dock_add_in_menu_with_stock_and_data   with icon size 24
static GtkWidget *cd_menu_append_one_item_to_menu (const gchar *cLabel, const gchar *gtkStock, GFunc pFunction, GtkWidget *pMenu, gpointer pData)
{
	GtkWidget *pMenuItem = gtk_image_menu_item_new_with_label (cLabel);
	if (gtkStock)
	{
		GtkWidget *image = NULL;
		if (*gtkStock == '/')
		{
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (gtkStock, myData.iPanelDefaultMenuIconSize, myData.iPanelDefaultMenuIconSize, NULL);
			image = gtk_image_new_from_pixbuf (pixbuf);
			g_object_unref (pixbuf);
		}
		else
		{
			const gchar *cIconPath = cairo_dock_search_icon_s_path (gtkStock, myData.iPanelDefaultMenuIconSize);
			if (cIconPath == NULL)
			{
				cIconPath = g_strconcat (MY_APPLET_SHARE_DATA_DIR"/", gtkStock, NULL);
				cIconPath = g_strconcat (cIconPath, ".svg", NULL);
			}
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cIconPath, myData.iPanelDefaultMenuIconSize, myData.iPanelDefaultMenuIconSize, NULL);
			image = gtk_image_new_from_pixbuf (pixbuf);
			g_object_unref (pixbuf);
		}
		_gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), image);
	}
	gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem);
	if (pFunction)
		g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK (pFunction), pData);
	return pMenuItem;
}

void cd_menu_append_poweroff_to_menu (GtkWidget *menu, GldiModuleInstance *myApplet)
{
	add_menu_separator (menu);

	if (myConfig.iShowQuit == CD_GMENU_SHOW_QUIT_LOGOUT || myConfig.iShowQuit == CD_GMENU_SHOW_QUIT_BOTH)
		cd_menu_append_one_item_to_menu (D_("Logout"), "system-log-out", (GFunc) cairo_dock_fm_logout, menu, NULL);

	if (myConfig.iShowQuit == CD_GMENU_SHOW_QUIT_SHUTDOWN || myConfig.iShowQuit == CD_GMENU_SHOW_QUIT_BOTH)
		cd_menu_append_one_item_to_menu (D_("Shutdown"), "system-shutdown", (GFunc) cairo_dock_fm_shutdown, menu, NULL);
}


static gchar * _get_settings_menu_name (void)
{
	gchar *cMenuFileName = NULL, *cXdgMenuPath = NULL;
	const gchar *cMenuPrefix = g_getenv ("XDG_MENU_PREFIX"); // e.g. on xfce, it contains "xfce-", nothing on gnome
	gchar **cXdgPath = cd_gmenu_get_xdg_menu_dirs ();

	int i;
	for (i = 0; cXdgPath[i] != NULL; i++)
	{
		g_free (cXdgMenuPath);
		cXdgMenuPath = g_strdup_printf ("%s/menus", cXdgPath[i]);
		if (! g_file_test (cXdgMenuPath, G_FILE_TEST_IS_DIR)) // cXdgPath can contain an invalid dir
			continue;
		gchar *cMenuFilePathWithPrefix = g_strdup_printf ("%s/%ssettings.menu",
			cXdgMenuPath, cMenuPrefix ? cMenuPrefix : "");
		if (g_file_test (cMenuFilePathWithPrefix, G_FILE_TEST_EXISTS))
		{
			g_free (cMenuFilePathWithPrefix);
			cMenuFileName = g_strdup_printf ("%s/%ssettings.menu", cXdgMenuPath,
				cMenuPrefix ? cMenuPrefix : "");
			break;
		}
		g_free (cMenuFilePathWithPrefix);
	}
	cd_debug ("Settings Menu: Found %s in %s (%s)", cMenuFileName, cXdgPath[i], cXdgMenuPath);
	g_strfreev (cXdgPath);
	g_free (cXdgMenuPath);

	if (cMenuFileName == NULL)
		cMenuFileName = g_strdup ("settings.menu");

	return cMenuFileName;
}

void panel_desktop_menu_item_append_menu (GtkWidget *menu, gpointer data)
{
	GldiModuleInstance *myApplet = (GldiModuleInstance *) data;
	if (myConfig.iShowQuit != CD_GMENU_SHOW_QUIT_NONE)
		cd_menu_append_poweroff_to_menu (menu, myApplet);
}

void main_menu_append (GtkWidget *main_menu,
		  gpointer   data)
{
	//g_print ("%s ()\n", __func__);	
	GldiModuleInstance *myApplet;

	myApplet = (GldiModuleInstance *) data;

	GtkWidget *desktop_menu;

	gchar *cSettingsMenuName = _get_settings_menu_name ();

	desktop_menu = create_applications_menu (cSettingsMenuName, NULL, main_menu);
	g_free (cSettingsMenuName);

	g_object_set_data_full (G_OBJECT (desktop_menu),
			"panel-menu-tree-directory",
			NULL, NULL);
	
	g_object_set_data (G_OBJECT (desktop_menu),
			   "panel-menu-append-callback",
			   panel_desktop_menu_item_append_menu);
	g_object_set_data (G_OBJECT (desktop_menu),
			   "panel-menu-append-callback-data",
			   myApplet);

	if (myData.bLoadInThread) // load submenu in a thread
		submenu_to_display (desktop_menu);
	
	if (myConfig.bShowRecent)
	{
		cd_menu_append_recent_to_menu (main_menu, myApplet);
	}
}


static void menu_item_style_set (GtkImage *image,
		     gpointer  data)
{
	GtkWidget   *widget;
	GdkPixbuf   *pixbuf;
	GtkIconSize  icon_size = (GtkIconSize) GPOINTER_TO_INT (data);
	int          icon_height;
	gboolean     is_mapped;

	if (!gtk_icon_size_lookup (icon_size, NULL, &icon_height))
		return;

	pixbuf = gtk_image_get_pixbuf (image);
	if (!pixbuf)
		return;

	if (gdk_pixbuf_get_height (pixbuf) == icon_height)
		return;

	widget = GTK_WIDGET (image);

	#if GTK_CHECK_VERSION (2, 20, 0)
	is_mapped = gtk_widget_get_mapped (widget);  // since gtk-2.20
	#else
	is_mapped = GTK_WIDGET_MAPPED (widget);
	#endif
	if (is_mapped)
		gtk_widget_unmap (widget);

	gtk_image_set_from_pixbuf (image, NULL);
    
	if (is_mapped)
		gtk_widget_map (widget);
}
static void do_icons_to_add (void)
{
	while (icons_to_add) {
		IconToAdd *icon_to_add = icons_to_add->data;

		icons_to_add = g_list_delete_link (icons_to_add, icons_to_add);

		/**if (icon_to_add->stock_id)
			gtk_image_set_from_stock (
				GTK_IMAGE (icon_to_add->image),
				icon_to_add->stock_id,
				icon_to_add->icon_size);
		else {*/
			g_assert (icon_to_add->pixbuf);

			gtk_image_set_from_pixbuf (
				GTK_IMAGE (icon_to_add->image),
				icon_to_add->pixbuf);

			g_signal_connect (icon_to_add->image, "style-set",
					  G_CALLBACK (menu_item_style_set),
					  GINT_TO_POINTER (icon_to_add->icon_size));

			g_object_unref (icon_to_add->pixbuf);
		///}

		g_object_unref (icon_to_add->image);
		g_free (icon_to_add);
	}
}
void icon_to_load_free (IconToLoad *icon)
{
	if (!icon)
		return;

	if (icon->pixmap)
		g_object_unref (icon->pixmap);
	icon->pixmap = NULL;

	/**if (icon->gicon)
		g_object_unref (icon->gicon);
	icon->gicon = NULL;*/

	g_free (icon->image);          icon->image = NULL;
	g_free (icon->fallback_image); icon->fallback_image = NULL;
	g_free (icon);
}
static gboolean load_icons_handler (gpointer data)
{
	IconToLoad *icon;
	gboolean    long_operation = FALSE;

load_icons_handler_again:

	if (!icons_to_load) {
		load_icons_id = 0;
		do_icons_to_add ();

		return FALSE;
	}

	icon = icons_to_load->data;
	icons_to_load->data = NULL;
	/* pop */
	icons_to_load = g_list_delete_link (icons_to_load, icons_to_load);

	/* if not visible anymore, just ignore */
	if ( ! gtk_widget_get_visible (icon->pixmap)) {  // since 2.18
		icon_to_load_free (icon);
		/* we didn't do anything long/hard, so just do this again,
		 * this is fun, don't go back to main loop */
		goto load_icons_handler_again;
	}

	/**if (icon->stock_id) {
		IconToAdd *icon_to_add;

		icon_to_add            = g_new (IconToAdd, 1);
		icon_to_add->image     = g_object_ref (icon->pixmap);
		icon_to_add->stock_id  = icon->stock_id;
		icon_to_add->pixbuf    = NULL;
		icon_to_add->icon_size = icon->icon_size;

		icons_to_add = g_list_prepend (icons_to_add, icon_to_add);
	}
	#ifdef HAVE_GIO
	else if (icon->gicon) {
		IconToAdd *icon_to_add;
		char      *icon_name;
		GdkPixbuf *pb;
		int        icon_height = myData.iPanelDefaultMenuIconSize;

		gtk_icon_size_lookup (icon->icon_size, NULL, &icon_height);

		icon_name = panel_util_get_icon_name_from_g_icon (icon->gicon);

		if (icon_name) {
			pb = panel_make_menu_icon (icon->icon_theme,
						   icon_name,
						   icon->fallback_image,
						   icon_height,
						   &long_operation);
			g_free (icon_name);
		} else {
			pb = panel_util_get_pixbuf_from_g_loadable_icon (icon->gicon, icon_height);
			if (!pb && icon->fallback_image) {
				pb = panel_make_menu_icon (icon->icon_theme,
							   NULL,
							   icon->fallback_image,
							   icon_height,
							   &long_operation);
			}
		}

		if (!pb) {
			icon_to_load_free (icon);
			if (long_operation)
				// this may have been a long operation so jump
				// back to the main loop for a while 
				return TRUE;
			else
				// we didn't do anything long/hard, so just do
				// this again, this is fun, don't go back to
				// main loop
				goto load_icons_handler_again;
		}

		icon_to_add            = g_new (IconToAdd, 1);
		icon_to_add->image     = g_object_ref (icon->pixmap);
		icon_to_add->stock_id  = NULL;
		icon_to_add->pixbuf    = pb;
		icon_to_add->icon_size = icon->icon_size;

		icons_to_add = g_list_prepend (icons_to_add, icon_to_add);
	}
	#endif
	else {*/
		IconToAdd *icon_to_add;
		GdkPixbuf *pb;
		int        icon_height = myData.iPanelDefaultMenuIconSize;

		gtk_icon_size_lookup (icon->icon_size, NULL, &icon_height);

		pb = panel_make_menu_icon (icon->icon_theme,
					   icon->image,
					   icon->fallback_image,
					   icon_height,
					   &long_operation);
		if (!pb) {
			icon_to_load_free (icon);
			if (long_operation)
				/* this may have been a long operation so jump back to
				 * the main loop for a while */
				return TRUE;
			else
				/* we didn't do anything long/hard, so just do this again,
				 * this is fun, don't go back to main loop */
				goto load_icons_handler_again;
		}

		icon_to_add            = g_new (IconToAdd, 1);
		icon_to_add->image     = g_object_ref (icon->pixmap);
		///icon_to_add->stock_id  = NULL;
		icon_to_add->pixbuf    = pb;
		icon_to_add->icon_size = icon->icon_size;

		icons_to_add = g_list_prepend (icons_to_add, icon_to_add);
	///}

	icon_to_load_free (icon);

	if (!long_operation)
		/* we didn't do anything long/hard, so just do this again,
		 * this is fun, don't go back to main loop */
		goto load_icons_handler_again;

	/* if still more we'll come back */
	return TRUE;
}
static GList * find_in_load_list (GtkWidget *image)
{
	GList *li;
	for (li = icons_to_load; li != NULL; li = li->next) {
		IconToLoad *icon = li->data;
		if (icon->pixmap == image)
			return li;
	}
	return NULL;
}
static IconToLoad * icon_to_load_copy (IconToLoad *icon)
{
	IconToLoad *retval;

	if (!icon)
		return NULL;

	retval = g_new0 (IconToLoad, 1);

	retval->pixmap         = g_object_ref (icon->pixmap);
	/**if (icon->gicon)
		retval->gicon  = g_object_ref (icon->gicon);
	else
		retval->gicon  = NULL;*/
	retval->image          = g_strdup (icon->image);
	retval->fallback_image = g_strdup (icon->fallback_image);
	///retval->stock_id       = icon->stock_id;
	retval->icon_size      = icon->icon_size;

	return retval;
}
void image_menu_shown (GtkWidget *image, gpointer data)
{
	IconToLoad *new_icon;
	IconToLoad *icon;
	
	icon = (IconToLoad *) data;

	/* if we've already handled this */
	if (gtk_image_get_storage_type (GTK_IMAGE (image)) != GTK_IMAGE_EMPTY)
		return;

	if (find_in_load_list (image) == NULL) {
		new_icon = icon_to_load_copy (icon);
		new_icon->icon_theme = gtk_icon_theme_get_for_screen (gtk_widget_get_screen (image));
		icons_to_load = g_list_append (icons_to_load, new_icon);
	}
	if (load_icons_id == 0)
	{
		if (myConfig.bLoadIconsAtStartup && myData.bIconsLoaded && myData.pTask)
		{  // bIconsLoaded when the thread is started and pTask is null at the end of the thread
			load_icons_id = 1;
			while ((load_icons_id = load_icons_handler (NULL)) && myApplet); // load it in the thread
		}
		else // load it in the mainloop with a low priority
			load_icons_id = g_idle_add_full (G_PRIORITY_LOW, load_icons_handler, NULL, NULL);
	}
}

void activate_app_def (GtkWidget      *menuitem,
		  GMenuTreeEntry *entry)
{
	const char       *path;

	path = gmenu_tree_entry_get_desktop_file_path (entry);
	panel_menu_item_activate_desktop_file (menuitem, path);
}


void  drag_data_get_menu_cb (GtkWidget        *widget,
		       GdkDragContext   *context,
		       GtkSelectionData *selection_data,
		       guint             info,
		       guint             time,
		       GMenuTreeEntry   *entry)
{
	const char *path;
	char       *uri;
	char       *uri_list;

	path = gmenu_tree_entry_get_desktop_file_path (entry);
	uri = g_filename_to_uri (path, NULL, NULL);
	uri_list = g_strconcat (uri, "\r\n", NULL);
	g_free (uri);

	gtk_selection_data_set (selection_data,
				gtk_selection_data_get_target (selection_data), 8, (guchar *)uri_list,
				strlen (uri_list));
	g_free (uri_list);
}
