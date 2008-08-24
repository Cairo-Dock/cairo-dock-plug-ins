
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

#define COMPIZ_DEFAULT_NAME "_Compiz_"

typedef enum {
  COMPIZ_DEFAULT = 0,
  COMPIZ_BROKEN,
  COMPIZ_OTHER,
  COMPIZ_SETTING,
  COMPIZ_EMERALD,
  COMPIZ_RELOAD,
  COMPIZ_EXPOSITION,
  COMPIZ_WLAYER,
  COMPIZ_NB_ITEMS,
} compizIcons;

typedef enum {
	COMPIZ_NO_ACTION = 0,
	COMPIZ_SWITCH_WM,
	COMPIZ_LAYER,
	COMPIZ_EXPO,
	COMPIZ_SHOW_DESKTOP,
	COMPIZ_NB_ACTIONS
} compizAction;

typedef enum {
	DECORATOR_EMERALD = 0,
	DECORATOR_GTK,
	DECORATOR_KDE,
	DECORATOR_HELIODOR,
	DECORATOR_USER,
	COMPIZ_NB_DECORATORS
} compizDecorator;


//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean lBinding;
	gboolean iRendering;
	gboolean uLocalScreen;
	gboolean bSystemDecorator;
	gboolean bAutoReloadCompiz;
	gboolean bAutoReloadDecorator;
	///gboolean protectDecorator;
	gboolean forceConfig;
	///compizWM iWM;
	gchar *cRenderer;
	gchar *cUserWMCommand;
	gchar *cWindowDecorator;
	gchar *cUserImage[COMPIZ_NB_ITEMS];
	compizAction iActionOnMiddleClick;
	const gchar *cDecorators[COMPIZ_NB_DECORATORS];
	gboolean bStealTaskBarIcon;
	gboolean bScriptSubDock;
} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	compizIcons iCompizIcon;
	gboolean bDecoratorIsRunning;
	gboolean bCompizIsRunning;
	gboolean bAcquisitionOK;
	CairoDockMeasure *pMeasureTimer;
	gboolean bCompizRestarted;
	gboolean bDecoratorRestarted;
	int iCompizMajor, iCompizMinor, iCompizMicro;
} ;


#endif
