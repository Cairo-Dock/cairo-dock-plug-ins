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

#include "applet-struct.h"
#include "applet-menu.h"  // start/stop
#include "applet-apps.h"
#include "applet-tree.h"

#define CD_FOLDER_DEFAULT_ICON "folder"

static void cd_populate_menu_from_directory (GtkWidget *menu, GMenuTreeDirectory *directory);


  /////////////////
 /// CALLBACKS ///
/////////////////

static void _on_tree_changed (GMenuTree *tree, G_GNUC_UNUSED gpointer data)
{
	cd_message ("%s ()", __func__);
	cd_menu_stop ();
	cd_menu_start ();
}

static void _on_activate_entry (GtkWidget *menuitem, GMenuTreeEntry *entry)
{
	GDesktopAppInfo *pAppInfo = gmenu_tree_entry_get_app_info (entry);
	g_app_info_launch (G_APP_INFO (pAppInfo), NULL, NULL, NULL);
}

static void _on_drag_data_get (GtkWidget *widget,
	GdkDragContext   *context,
	GtkSelectionData *selection_data,
	guint             info,
	guint             time,
	GMenuTreeEntry   *entry)
{
	const char *cDesktopFile = gmenu_tree_entry_get_desktop_file_path (entry);
	gchar *cUri = g_filename_to_uri (cDesktopFile, NULL, NULL);
	gchar *uri_list = g_strconcat (cUri, "\r\n", NULL);
	gtk_selection_data_set (selection_data,
		gtk_selection_data_get_target (selection_data), 8, (guchar *)uri_list,
		strlen (uri_list));
	g_free (uri_list);
	g_free (cUri);
}


  //////////////////////
 /// MENU FROM TREE ///
//////////////////////

/* Not really needed with libgnome-menu-3
static void _load_one_icon (GtkWidget *image)
{
	// this actually loads the pixbuf of the gicon
	GtkRequisition requisition;
	#if (GTK_MAJOR_VERSION < 3)
	gtk_widget_size_request (image, &requisition);
	#else
	gtk_widget_get_preferred_size (image, &requisition, NULL);
	#endif
}*/

static void add_image_to_menu_item (GtkWidget *image_menu_item,
	GIcon *pIcon,
	const char *fallback_image_filename)
{
	// make a GtkImage
	GtkWidget *image = gtk_image_new ();
	gtk_widget_set_size_request (image, myData.iPanelDefaultMenuIconSize, myData.iPanelDefaultMenuIconSize);
	
	if (pIcon)  // this just sets the gicon on the image, it doesn't load the pixbuf.
		gtk_image_set_from_gicon (GTK_IMAGE (image), pIcon, GTK_ICON_SIZE_LARGE_TOOLBAR);
	else if (fallback_image_filename)  // same
		gtk_image_set_from_icon_name (GTK_IMAGE (image), fallback_image_filename, GTK_ICON_SIZE_LARGE_TOOLBAR);

	// insert the image in the menu-item
	gldi_menu_item_set_image (image_menu_item, image);
	gtk_widget_show (image);
}

/**static gchar * menu_escape_underscores_and_prepend (const char *text)
{
	GString    *escaped_text;
	const char *src;
	int         inserted;
	
	if (!text)
		return g_strdup (text);

	escaped_text = g_string_sized_new (strlen (text) + 1);
	g_string_printf (escaped_text, "_%s", text);

	src = text;
	inserted = 1;

	while (*src) {
		gunichar c;

		c = g_utf8_get_char (src);

		if (c == (gunichar)-1) {
			g_warning ("Invalid input string for underscore escaping");
			return g_strdup (text);
		} else if (c == '_') {
			g_string_insert_c (escaped_text,
					   src - text + inserted, '_');
			inserted++;
		}

		src = g_utf8_next_char (src);
	}

	return g_string_free (escaped_text, FALSE);
}*/

static GtkWidget * add_menu_separator (GtkWidget *menu)
{
	GtkWidget *menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	
	gtk_widget_show (menuitem);
	
	return menuitem;
}

static GtkWidget * create_submenu_entry (GtkWidget *menu,
	GMenuTreeDirectory *directory)
{
	if (gmenu_tree_directory_get_is_nodisplay (directory))
		return NULL;
	
	GtkWidget *menuitem = gldi_menu_item_new (gmenu_tree_directory_get_name (directory), "");
	
	GIcon *pIcon = gmenu_tree_directory_get_icon (directory);  // transfer: None
	add_image_to_menu_item (menuitem,
		pIcon,
		CD_FOLDER_DEFAULT_ICON);
	
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	return menuitem;
}

static void create_submenu (GtkWidget *menu,
	GMenuTreeDirectory *directory,
	GMenuTreeDirectory *alias_directory)
{
	// create an entry
	GtkWidget *menuitem;
	if (alias_directory)
		menuitem = create_submenu_entry (menu, alias_directory);
	else
		menuitem = create_submenu_entry (menu, directory);
	if (!menuitem)
		return;
	
	// create a sub-menu for it
	GtkWidget *submenu = gldi_submenu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), submenu);
	
	// populate the sub-menu with the directory
	cd_populate_menu_from_directory (submenu, directory);
}

static void create_header (GtkWidget *menu,
	GMenuTreeHeader *header)
{
	// create an entry
	GMenuTreeDirectory *directory = gmenu_tree_header_get_directory (header);
	create_submenu_entry (menu, directory);
	gmenu_tree_item_unref (directory);
}

static void create_menuitem (GtkWidget *menu,
	GMenuTreeEntry *entry,
	GMenuTreeDirectory *alias_directory)
{
	// register the application
	GDesktopAppInfo *pAppInfo = gmenu_tree_entry_get_app_info (entry);
	cd_menu_register_app (pAppInfo);
	
	// ignore entry that are not shown in the menu
	if (gmenu_tree_entry_get_is_excluded (entry))
		return;
	if (! cd_menu_app_should_show (pAppInfo))
		return;
	
	// create an entry
	const gchar *cName = NULL;
	if (alias_directory)
		cName = gmenu_tree_directory_get_name (alias_directory);
	if (!cName)
		cName = g_app_info_get_name (G_APP_INFO (pAppInfo));
	GtkWidget *menuitem = gldi_menu_item_new (cName, "");
	
	const gchar *cComment = NULL;
	if (alias_directory)
		cComment = gmenu_tree_directory_get_comment (alias_directory);
	if (cComment == NULL)
		cComment = g_app_info_get_description (G_APP_INFO (pAppInfo));
	if (cComment)
		gtk_widget_set_tooltip_text (menuitem, cComment);
	
	// load icon
	GIcon *pIcon = NULL;
	if (alias_directory)
		pIcon = gmenu_tree_directory_get_icon (alias_directory);
	if (!pIcon)
		pIcon = g_app_info_get_icon (G_APP_INFO (pAppInfo));
	add_image_to_menu_item (menuitem,
		pIcon,
		NULL);
	
	// Drag and drop
	static GtkTargetEntry menu_item_targets[] = {
		{ (gchar*)"text/uri-list", 0, 0 }
	};
	gtk_drag_source_set (menuitem,
		GDK_BUTTON1_MASK | GDK_BUTTON2_MASK,
		menu_item_targets, 1,
		GDK_ACTION_COPY);
	if (pIcon != NULL)
	{
		gchar *cIcon = g_icon_to_string (pIcon);
		gtk_drag_source_set_icon_name (menuitem, cIcon);
		g_free (cIcon);
	}
	
	g_signal_connect (menuitem, "drag_data_get",
		G_CALLBACK (_on_drag_data_get), entry);
	
	// add to the menu
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	
	g_signal_connect (menuitem, "activate",
		G_CALLBACK (_on_activate_entry), entry);
	
	g_object_set_data_full (G_OBJECT (menuitem),
		"cd-entry",
		gmenu_tree_item_ref (entry),
		(GDestroyNotify) gmenu_tree_item_unref);  // stick the entry on the menu-item, which allows us to ref it and be sure to unref when the menu is destroyed.
}

static void create_menuitem_from_alias (GtkWidget *menu,
	GMenuTreeAlias *alias)
{
	GMenuTreeItemType iType = gmenu_tree_alias_get_aliased_item_type (alias);
	GMenuTreeDirectory *src = gmenu_tree_alias_get_directory (alias);
	switch (iType)
	{
		case GMENU_TREE_ITEM_DIRECTORY:
		{
			GMenuTreeDirectory *directory = gmenu_tree_alias_get_aliased_directory (alias);
			create_submenu (menu,
				directory,
				src);
			gmenu_tree_item_unref (directory);
		}
		break;

		case GMENU_TREE_ITEM_ENTRY:
		{
			GMenuTreeEntry *entry = gmenu_tree_alias_get_aliased_entry (alias);
			create_menuitem (menu,
				gmenu_tree_alias_get_aliased_entry (alias),
				src);
			gmenu_tree_item_unref (entry);
		}
		break;

		default:
		break;
	}
	gmenu_tree_item_unref (src);
}

static void cd_populate_menu_from_directory (GtkWidget *menu, GMenuTreeDirectory *directory)
{
	GMenuTreeIter *iter = gmenu_tree_directory_iter (directory);
	GMenuTreeItemType next_type;
	while ((next_type = gmenu_tree_iter_next (iter)) != GMENU_TREE_ITEM_INVALID)
	{
		gpointer item = NULL;
		switch (next_type)
		{
			case GMENU_TREE_ITEM_DIRECTORY:  // we suppose that unicity is assured.
				item = gmenu_tree_iter_get_directory (iter);
				create_submenu (menu, item, NULL);
				break;

			case GMENU_TREE_ITEM_ENTRY:
				item = gmenu_tree_iter_get_entry (iter);
				create_menuitem (menu, item, NULL);
				break;

			case GMENU_TREE_ITEM_SEPARATOR :
				add_menu_separator (menu);
				break;

			case GMENU_TREE_ITEM_ALIAS:
				item = gmenu_tree_iter_get_alias (iter);
				create_menuitem_from_alias (menu, item);
				break;

			case GMENU_TREE_ITEM_HEADER:
				item = gmenu_tree_iter_get_header (iter);
				create_header (menu, item);
				break;

			default:
				break;
		}
		if (item)
			gmenu_tree_item_unref (item);
	}
	gmenu_tree_iter_unref (iter);
}

void cd_append_tree_in_menu (GMenuTree *tree, GtkWidget *pMenu)
{
	GMenuTreeDirectory *dir = gmenu_tree_get_root_directory (tree);
	g_return_if_fail (dir);
	
	cd_populate_menu_from_directory (pMenu, dir);
	gmenu_tree_item_unref (dir);
	
	g_signal_connect (tree, "changed", G_CALLBACK (_on_tree_changed), NULL);
}


  /////////////////
 /// LOAD TREE ///
/////////////////

// $XDG_CONFIG_DIRS => /etc/xdg/xdg-cairo-dock:/etc/xdg
// http://developer.gnome.org/menu-spec/
static gchar ** _get_xdg_menu_dirs (void)
{
	const gchar *dirs = g_getenv ("XDG_CONFIG_DIRS");
	if (! dirs || *dirs == '\0')
		dirs = "/etc/xdg";

	return g_strsplit (dirs, ":", 0);
}

// check if the file exists and if yes, *cMenuName is created
static gchar * _check_file_exists (const gchar *cDir, const gchar *cPrefix, const gchar *cMenuFile, gchar **cMenuName)
{
	gchar *cMenuFilePathWithPrefix = g_strdup_printf ("%s/%s%s", cDir, cPrefix, cMenuFile);
	gchar *cFoundMenuFile = NULL;
	if (g_file_test (cMenuFilePathWithPrefix, G_FILE_TEST_EXISTS))
		cFoundMenuFile = g_strdup_printf ("%s%s", cPrefix, cMenuFile);
	
	cd_debug ("Check: %s: %d", cMenuFilePathWithPrefix, cFoundMenuFile!=NULL);
	g_free (cMenuFilePathWithPrefix);
	return cFoundMenuFile;
}

static const gchar *cPrefixNames[] = {"", "gnome-", "kde-", "kde4-", "xfce-", "lxde-", NULL};

static gchar * cd_find_menu_file (const gchar *cMenuFile)
{
	gchar *cMenuFileName = NULL, *cXdgMenuPath = NULL;
	const gchar *cMenuPrefix = g_getenv ("XDG_MENU_PREFIX"); // e.g. on xfce, it contains "xfce-", nothing on gnome
	gchar **cXdgPath = _get_xdg_menu_dirs ();
	
	int i;
	for (i = 0; cXdgPath[i] != NULL; i++)
	{
		g_free (cXdgMenuPath);
		cXdgMenuPath = g_strdup_printf ("%s/menus", cXdgPath[i]);
		if (! g_file_test (cXdgMenuPath, G_FILE_TEST_IS_DIR)) // cXdgPath can contain an invalid dir
			continue;

		// this test should be the good one: with or without the prefix
		if ((cMenuFileName = _check_file_exists (cXdgMenuPath, cMenuPrefix ? cMenuPrefix : "", cMenuFile, &cMenuFileName)) != NULL)
			break;

		// let's check with common prefixes
		for (int iPrefix = 0; cPrefixNames[iPrefix] != NULL; iPrefix++)
		{
			if ((cMenuFileName = _check_file_exists (cXdgMenuPath, cPrefixNames[iPrefix], cMenuFile, &cMenuFileName)) != NULL)
				break;
		}

		if (cMenuFileName == NULL) // let's check any *<menu-file>
		{
			const gchar *cFileName;
			GDir *dir = g_dir_open (cXdgMenuPath, 0, NULL);
			if (dir)
			{
				while ((cFileName = g_dir_read_name (dir)))
				{
					if (g_str_has_suffix (cFileName, cMenuFile))
					{
						cMenuFileName = g_strdup (cFileName);
						break;
					}
				}
				g_dir_close (dir);
				if (cMenuFileName != NULL)
					break;
			}
		}
	}
	
	cd_debug ("Menu: Found %s in %s (%s)", cMenuFileName, cXdgPath[i], cXdgMenuPath);
	
	if (cMenuFileName == NULL)  // desperation move: let gmenu try to do better.
		cMenuFileName = g_strdup (cMenuFile);
	
	g_strfreev (cXdgPath);
	g_free (cXdgMenuPath);
	return cMenuFileName;
}

GMenuTree *cd_load_tree_from_file (const gchar *cMenuFile)
{
	gchar *cMenuFileName = cd_find_menu_file (cMenuFile);
	GMenuTree *tree = gmenu_tree_new (cMenuFileName, GMENU_TREE_FLAGS_INCLUDE_NODISPLAY | GMENU_TREE_FLAGS_INCLUDE_EXCLUDED);  /// GMENU_TREE_FLAGS_INCLUDE_NODISPLAY
	if (! gmenu_tree_load_sync (tree, NULL))  // this does all the heavy work of parsing the .menu and each desktop files.
	{
		g_object_unref (tree);
		tree = NULL;
	}
	g_free (cMenuFileName);
	return tree;
}
