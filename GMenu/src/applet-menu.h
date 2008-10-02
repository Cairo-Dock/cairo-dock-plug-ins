
#ifndef __APPLET_MENU__
#define  __APPLET_MENU__

#include <cairo-dock.h>
#define GMENU_I_KNOW_THIS_IS_UNSTABLE
#include <gmenu-tree.h>

#define PANEL_ICON_FOLDER "folder"
#define PANEL_DEFAULT_MENU_ICON_SIZE 24


GtkWidget * add_menu_separator (GtkWidget *menu);

GtkWidget * create_fake_menu (GMenuTreeDirectory *directory);

GdkPixbuf * panel_make_menu_icon (GtkIconTheme *icon_theme,
		      const char   *icon,
		      const char   *fallback,
		      int           size,
		      gboolean     *long_operation);

void setup_menuitem (GtkWidget   *menuitem,
		GtkIconSize  icon_size,
		GtkWidget   *image,
		const char  *title);

GtkWidget * populate_menu_from_directory (GtkWidget          *menu,
			      GMenuTreeDirectory *directory);

void image_menu_destroy (GtkWidget *image, gpointer data);


GtkWidget * create_empty_menu (void);

GtkWidget * create_applications_menu (const char *menu_file,
			  const char *menu_path);

GtkWidget * create_main_menu (CairoDockModuleInstance *myApplet);


#endif
