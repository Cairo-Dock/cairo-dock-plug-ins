
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


typedef struct
{
	GObject parent;
	DBusGConnection *connection;
	DBusGProxy *proxy;
	gchar *cModuleName;
	CairoDockModuleInstance *pModuleInstance;
} dbusApplet;

typedef struct
{
	GObjectClass parent_class;
} dbusAppletClass;


typedef enum {
	CLIC=0,
	MIDDLE_CLIC,
	SCROLL,
	BUILD_MENU,
	MENU_SELECT,
	DROP_DATA,
	RELOAD_MODULE,
	INIT_MODULE,
	STOP_MODULE,
	NB_SIGNALS
} CDSignalEnum;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	dbusCallback *server;
	guint iSidOnClickIcon;
	guint iSidOnMiddleClickIcon;
	guint iSidOnScrollIcon;
	guint iSidOnBuildMenu;
	guint iSidOnDropData;
	guint iSidOnReloadModule;
	guint iSidOnInitModule;
	guint iSidOnStopModule;
	guint iSidOnMenuSelect;
	GtkWidget *pModuleSubMenu;
	gpointer pCurrentMenuDbusApplet;
	GList *pAppletList;
	} ;


#endif
