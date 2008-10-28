
#ifndef __APPLET_MENU_CALLBACKS__
#define  __APPLET_MENU_CALLBACKS__

#include <cairo-dock.h>
#define GMENU_I_KNOW_THIS_IS_UNSTABLE
#include <gmenu-tree.h>


void handle_gmenu_tree_changed (GMenuTree *tree,
			   GtkWidget *menu);

void remove_gmenu_tree_monitor (GtkWidget *menu,
			  GMenuTree  *tree);

gboolean menu_dummy_button_press_event (GtkWidget      *menuitem,
			       GdkEventButton *event);
void remove_submenu_to_display_idle (gpointer data);

gboolean submenu_to_display_in_idle (gpointer data);

void submenu_to_display (GtkWidget *menu);

void panel_desktop_menu_item_append_menu (GtkWidget *menu,
				     gpointer   data);
void main_menu_append (GtkWidget *main_menu,
		  gpointer   data);

void icon_to_load_free (IconToLoad *icon);

void image_menu_shown (GtkWidget *image, gpointer data);

void activate_app_def (GtkWidget      *menuitem,
		  GMenuTreeEntry *entry);

void  drag_begin_menu_cb (GtkWidget *widget, GdkDragContext     *context);

void  drag_end_menu_cb (GtkWidget *widget, GdkDragContext     *context);

void  drag_data_get_menu_cb (GtkWidget        *widget,
		       GdkDragContext   *context,
		       GtkSelectionData *selection_data,
		       guint             info,
		       guint             time,
		       GMenuTreeEntry   *entry);





#endif
