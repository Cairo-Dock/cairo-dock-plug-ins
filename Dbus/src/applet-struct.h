
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bEnablePopUp;
	gboolean bEnableReboot;
	gboolean bEnableDesklets;
	gboolean bEnableReloadModule;
	gboolean bEnableQuit;
	gboolean bEnableShowDock;
	gboolean bEnableLoadLauncher;
	gboolean bEnableCreateLauncher;
	gboolean bEnableSetQuickInfo;
	gboolean bEnableSetLabel;
	gboolean bEnableSetIcon;
	gboolean bEnableNewModule;
	gboolean bEnableAnimateIcon;
	} ;


typedef struct
{
	GObject parent;
	DBusGConnection *connection;
	DBusGProxy *proxy;
} dbusCallback;

typedef struct
{
	GObjectClass parent_class;
} dbusCallbackClass;


//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	dbusCallback *server;
	guint iSidOnClickIcon;
	guint iSidOnMiddleClickIcon;
	guint iSidOnScrollIcon;
	guint iSidOnBuildMenu;
	guint iSidOnDropData;
	guint iSidOnReloadModule;
	guint iSidOnMenuSelect;
	GtkWidget *pModuleSubMenu;
	gchar *cCurrentMenuModule;
	} ;


#endif
