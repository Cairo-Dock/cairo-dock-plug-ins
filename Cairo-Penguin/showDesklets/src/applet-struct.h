
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <glib.h>
#include <X11/Xlib.h>

typedef struct {
	gboolean bShowWidgetLayerDesklet;
	gchar *cShowImage;
	gchar *cHideImage;
	gchar *cShortcut;
	} AppletConfig;

typedef struct {
	gboolean bHide;
	Window xLastActiveWindow;
	} AppletData;

#endif
