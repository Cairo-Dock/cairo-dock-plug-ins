
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>
#ifdef HAVE_GIO
#include <gio/gio.h>
#else
#define GIcon gpointer
#endif

typedef struct {
	GtkWidget    *pixmap;
	const char   *stock_id;
	GIcon        *gicon;
	char         *image;
	char         *fallback_image;
	GtkIconTheme *icon_theme;
	GtkIconSize   icon_size;
} IconToLoad;

typedef struct {
	GtkWidget   *image;
	const char  *stock_id;
	GdkPixbuf   *pixbuf;
	GtkIconSize  icon_size;
} IconToAdd;


//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bHasIcons;
	gchar *cMenuShortkey;
	gchar *cQuickLaunchShortkey;
	gchar *cConfigureMenuCommand;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GtkWidget *pMenu;
	CairoDialog *pQuickLaunchDialog;
	gboolean bIconsLoaded;
	guint iSidFakeMenuIdle;
	guint iSidCreateMenuIdle;
	guint iSidTreeChangeIdle;
	} ;


#endif
