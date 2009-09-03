
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bEnableReboot;
	gboolean bEnableDesklets;
	gboolean bEnableReloadModule;
	gboolean bEnableActivateModule;
	gboolean bEnableQuit;
	gboolean bEnableShowDock;
	gboolean bEnableTweakingLauncher;
	gboolean bEnableCreateLauncher;
	gboolean bEnableSetQuickInfo;
	gboolean bEnableSetLabel;
	gboolean bEnableSetIcon;
	gboolean bEnablePopUp;
	gboolean bEnableAnimateIcon;
	gboolean bEnableNewModule;
	} ;


typedef struct
{
	GObject parent;
	DBusGConnection *connection;
} dbusMainObject;

typedef struct
{
	GObjectClass parent_class;
} dbusMainObjectClass;


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
	dbusMainObject *pMainObject;
	GList *pAppletList;
	GtkWidget *pModuleSubMenu;
	gpointer pCurrentMenuDbusApplet;
	} ;


#endif
