
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <X11/Xlib.h>
#include <cairo-dock.h>

struct _AppletConfig {
	gboolean bShowWidgetLayerDesklet;
	gchar *cShowImage;
	gchar *cHideImage;
	gchar *cShortcut;
	} ;

struct _AppletData {
	gboolean bHide;
	Window xLastActiveWindow;
	} ;

#endif
