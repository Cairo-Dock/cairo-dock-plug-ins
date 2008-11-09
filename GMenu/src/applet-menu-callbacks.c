/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Adapted from the Gnome-panel for Cairo-Dock by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>
#include <gdk/gdkkeysyms.h>

#include "applet-struct.h"
#include "applet-menu.h"
#include "applet-util.h"
#include "applet-menu-callbacks.h"

static guint load_icons_id = 0;
static GList *icons_to_load = NULL;
static GList *icons_to_add = NULL;


void handle_gmenu_tree_changed (GMenuTree *tree,
			   GtkWidget *menu)
{
	g_print ("%s ()\n", __func__);
	
	if (myData.pMenu != NULL)
	{
		gtk_widget_destroy (myData.pMenu);
		myData.pMenu = NULL;
	}
	
	if (myData.pMenu == NULL)
	{
		myData.pMenu = create_main_menu (NULL);
	}
	return ;
	
	guint idle_id;

	while (GTK_MENU_SHELL (menu)->children)
                gtk_widget_destroy (GTK_MENU_SHELL (menu)->children->data);

	g_object_set_data_full (G_OBJECT (menu),
				"panel-menu-tree-directory",
				NULL, NULL);

	g_object_set_data (G_OBJECT (menu),
			   "panel-menu-needs-loading",
			   GUINT_TO_POINTER (TRUE));

	idle_id = g_idle_add_full (G_PRIORITY_LOW,
				   submenu_to_display_in_idle,
				   menu,
				   NULL);
	if (myData.iSidTreeChangeIdle != 0)
		g_source_remove (myData.iSidTreeChangeIdle);
	myData.iSidTreeChangeIdle = idle_id;
	g_object_set_data_full (G_OBJECT (menu),
				"panel-menu-idle-id",
				GUINT_TO_POINTER (idle_id),
				remove_submenu_to_display_idle);
}

void remove_gmenu_tree_monitor (GtkWidget *menu,
			  GMenuTree  *tree)
{
	g_print ("%s (%x)\n", __func__, tree);
	gmenu_tree_remove_monitor (tree,
				  (GMenuTreeChangedFunc) handle_gmenu_tree_changed,
				  menu);
}


gboolean menu_dummy_button_press_event (GtkWidget      *menuitem,
			       GdkEventButton *event)
{
	if (event->button == 3)
		return TRUE;

	return FALSE;
}


void remove_submenu_to_display_idle (gpointer data)
{
	guint idle_id = GPOINTER_TO_UINT (data);

	g_source_remove (idle_id);
}


gboolean submenu_to_display_in_idle (gpointer data)
{
	GtkWidget *menu = GTK_WIDGET (data);
	g_print ("%s (%x)\n", __func__, menu);

	g_object_set_data (G_OBJECT (menu), "panel-menu-idle-id", NULL);

	submenu_to_display (menu);

	return FALSE;
}

void submenu_to_display (GtkWidget *menu)
{
	g_print ("%s (%x)\n", __func__, menu);
	GMenuTree           *tree;
	GMenuTreeDirectory  *directory;
	const char          *menu_path;
	void               (*append_callback) (GtkWidget *, gpointer);
	gpointer             append_data;

	if (!g_object_get_data (G_OBJECT (menu), "panel-menu-needs-loading"))
	{
		g_print ("en fait non\n");
		return;
	}

	g_object_set_data (G_OBJECT (menu), "panel-menu-needs-loading", NULL);

	directory = g_object_get_data (G_OBJECT (menu),
				       "panel-menu-tree-directory");
	if (!directory) {
		menu_path = g_object_get_data (G_OBJECT (menu),
					       "panel-menu-tree-path");
		g_print ("n'est pas un directory, menu_path : %s\n", menu_path);
		if (!menu_path)
		{
			cd_warning ("menu_path is empty");
			return;
		}
		
		tree = g_object_get_data (G_OBJECT (menu), "panel-menu-tree");
		if (!tree)
		{
			cd_warning ("no tree found in datas");
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


void panel_desktop_menu_item_append_menu (GtkWidget *menu,
				     gpointer   data)
{
	//g_print ("%s ()\n", __func__);	
	CairoDockModuleInstance *myApplet;
	gboolean              add_separator;
	GList                *children;
	GList                *last;

	myApplet = (CairoDockModuleInstance *) data;

	add_separator = FALSE;
	children = gtk_container_get_children (GTK_CONTAINER (menu));
	last = g_list_last (children);

	///if (last != NULL)
	///	add_separator = !GTK_IS_SEPARATOR (GTK_WIDGET (last->data));

	g_list_free (children);

	if (add_separator)
		add_menu_separator (menu);

	//panel_menu_items_append_from_desktop (menu, "yelp.desktop", NULL);
	//panel_menu_items_append_from_desktop (menu, "gnome-about.desktop", NULL);

	//if (parent->priv->append_lock_logout)
	//	panel_menu_items_append_lock_logout (menu);
}
void main_menu_append (GtkWidget *main_menu,
		  gpointer   data)
{
	//g_print ("%s ()\n", __func__);	
	CairoDockModuleInstance *myApplet;
	GtkWidget   *item;
	gboolean     add_separator;
	GList       *children;
	GList       *last;

	myApplet = (CairoDockModuleInstance *) data;

	add_separator = FALSE;
	children = gtk_container_get_children (GTK_CONTAINER (main_menu));
	last = g_list_last (children);
	if (last != NULL) {
		///add_separator = !GTK_IS_SEPARATOR (GTK_WIDGET (last->data));
	}
	g_list_free (children);

	if (add_separator)
		add_menu_separator (main_menu);
	
	
	GtkWidget *desktop_menu;

	desktop_menu = create_applications_menu ("settings.menu", NULL, main_menu);
	g_object_set_data_full (G_OBJECT (desktop_menu),
			"panel-menu-tree-directory",
			NULL, NULL);
	
	g_object_set_data (G_OBJECT (desktop_menu),
			   "panel-menu-append-callback",
			   panel_desktop_menu_item_append_menu);
	g_object_set_data (G_OBJECT (desktop_menu),
			   "panel-menu-append-callback-data",
			   myApplet);
	
	/*item = panel_place_menu_item_new (TRUE);
	panel_place_menu_item_set_panel (item, panel);
	gtk_menu_shell_append (GTK_MENU_SHELL (main_menu), item);
	gtk_widget_show (item);

	item = panel_desktop_menu_item_new (TRUE, FALSE);
	panel_desktop_menu_item_set_panel (item, panel);
	gtk_menu_shell_append (GTK_MENU_SHELL (main_menu), item);
	gtk_widget_show (item);

	panel_menu_items_append_lock_logout (main_menu);*/
}

/*gboolean show_item_menu (GtkWidget      *item,
		GdkEventButton *bevent)
{
	CairoDockModuleInstance *myApplet;
	GtkWidget   *menu;

	if (panel_lockdown_get_locked_down ())
		return FALSE;

	panel_widget = menu_get_panel (item);

	menu = g_object_get_data (G_OBJECT (item), "panel-item-context-menu");

	if (!menu)
		menu = create_item_context_menu (item, panel_widget);

	if (!menu)
		return FALSE;

	gtk_menu_set_screen (GTK_MENU (menu),
			     gtk_window_get_screen (GTK_WINDOW (panel_widget->toplevel)));

	gtk_menu_popup (GTK_MENU (menu),
			NULL, NULL, NULL, NULL,
			bevent->button,
			bevent->time);

	return TRUE;
}
gboolean panel_menu_key_press_handler (GtkWidget   *widget,
			      GdkEventKey *event)
{
	gboolean retval = FALSE;

	if ((event->keyval == GDK_Menu) ||
	    (event->keyval == GDK_F10 &&
	    (event->state & gtk_accelerator_get_default_mod_mask ()) == GDK_SHIFT_MASK)) {
		GtkMenuShell *menu_shell = GTK_MENU_SHELL (widget);

		if (menu_shell->active_menu_item &&
		    GTK_MENU_ITEM (menu_shell->active_menu_item)->submenu == NULL) {
			GdkEventButton bevent;

			bevent.button = 3;
			bevent.time = GDK_CURRENT_TIME;
			retval = show_item_menu (menu_shell->active_menu_item,
// 						 &bevent);
		}
		
	}
	return retval;
}*/

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

	is_mapped = GTK_WIDGET_MAPPED (widget);
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

		if (icon_to_add->stock_id)
			gtk_image_set_from_stock (
				GTK_IMAGE (icon_to_add->image),
				icon_to_add->stock_id,
				icon_to_add->icon_size);
		else {
			g_assert (icon_to_add->pixbuf);

			gtk_image_set_from_pixbuf (
				GTK_IMAGE (icon_to_add->image),
				icon_to_add->pixbuf);

			g_signal_connect (icon_to_add->image, "style-set",
					  G_CALLBACK (menu_item_style_set),
					  GINT_TO_POINTER (icon_to_add->icon_size));

			g_object_unref (icon_to_add->pixbuf);
		}

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

	if (icon->gicon)
		g_object_unref (icon->gicon);
	icon->gicon = NULL;

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
	if ( ! GTK_WIDGET_VISIBLE (icon->pixmap)) {
		icon_to_load_free (icon);
		/* we didn't do anything long/hard, so just do this again,
		 * this is fun, don't go back to main loop */
		goto load_icons_handler_again;
	}

	if (icon->stock_id) {
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
		int        icon_height = PANEL_DEFAULT_MENU_ICON_SIZE;

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
				/* this may have been a long operation so jump
				 * back to the main loop for a while */
				return TRUE;
			else
				/* we didn't do anything long/hard, so just do
				 * this again, this is fun, don't go back to
				 * main loop */
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
	else {
		IconToAdd *icon_to_add;
		GdkPixbuf *pb;
		int        icon_height = PANEL_DEFAULT_MENU_ICON_SIZE;

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
		icon_to_add->stock_id  = NULL;
		icon_to_add->pixbuf    = pb;
		icon_to_add->icon_size = icon->icon_size;

		icons_to_add = g_list_prepend (icons_to_add, icon_to_add);
	}

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
	if (icon->gicon)
		retval->gicon  = g_object_ref (icon->gicon);
	else
		retval->gicon  = NULL;
	retval->image          = g_strdup (icon->image);
	retval->fallback_image = g_strdup (icon->fallback_image);
	retval->stock_id       = icon->stock_id;
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
		load_icons_id = g_idle_add (load_icons_handler, NULL);
}

void activate_app_def (GtkWidget      *menuitem,
		  GMenuTreeEntry *entry)
{
	const char       *path;

	path = gmenu_tree_entry_get_desktop_file_path (entry);
	panel_menu_item_activate_desktop_file (menuitem, path);
}



void  drag_begin_menu_cb (GtkWidget *widget, GdkDragContext     *context)
{
	/* FIXME: workaround for a possible gtk+ bug
	 *    See bugs #92085(gtk+) and #91184(panel) for details.
	 *    Maybe it's not needed with GtkTooltip?
	 */
	g_object_set (widget, "has-tooltip", FALSE, NULL);
}

/* This is a _horrible_ hack to have this here. This needs to be added to the
 * GTK+ menuing code in some manner.
 */
void  drag_end_menu_cb (GtkWidget *widget, GdkDragContext     *context)
{
  GtkWidget *xgrab_shell;
  GtkWidget *parent;

  /* Find the last viewable ancestor, and make an X grab on it
   */
  parent = widget->parent;
  xgrab_shell = NULL;

  /* FIXME: workaround for a possible gtk+ bug
   *    See bugs #92085(gtk+) and #91184(panel) for details.
   */
  g_object_set (widget, "has-tooltip", TRUE, NULL);

  while (parent)
    {
      gboolean viewable = TRUE;
      GtkWidget *tmp = parent;
      
      while (tmp)
	{
	  if (!GTK_WIDGET_MAPPED (tmp))
	    {
	      viewable = FALSE;
	      break;
	    }
	  tmp = tmp->parent;
	}
      
      if (viewable)
	xgrab_shell = parent;
      
      parent = GTK_MENU_SHELL (parent)->parent_menu_shell;
    }
  
  if (xgrab_shell && !GTK_MENU(xgrab_shell)->torn_off)
    {
      GdkCursor *cursor = gdk_cursor_new (GDK_ARROW);

      if ((gdk_pointer_grab (xgrab_shell->window, TRUE,
			     GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			     GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
			     GDK_POINTER_MOTION_MASK,
			     NULL, cursor, GDK_CURRENT_TIME) == 0))
	{
	  if (gdk_keyboard_grab (xgrab_shell->window, TRUE,
				 GDK_CURRENT_TIME) == 0)
	    GTK_MENU_SHELL (xgrab_shell)->have_xgrab = TRUE;
	  else
	    {
	      gdk_pointer_ungrab (GDK_CURRENT_TIME);
	    }
	}

      gdk_cursor_unref (cursor);
    }
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
				selection_data->target, 8, (guchar *)uri_list,
				strlen (uri_list));
	g_free (uri_list);
}

/*gboolean menuitem_button_press_event (GtkWidget      *menuitem,
			     GdkEventButton *event)
{
	if (event->button == 3)
		return show_item_menu (menuitem, event);
	
	return FALSE;
}*/
