
#ifndef __APPLET_STRUCT__
#define  __APPLET_STRUCT__

#include <glib.h>
#include <cairo-dock.h>

#define SHORTCUTS_DEFAULT_NAME "_shortcuts_"

typedef struct {
	gboolean bListDrives;
	gboolean bListNetwork;
	gboolean bListBookmarks;
	gboolean bUseSeparator;
	gchar *cRenderer;
	} AppletConfig;


typedef struct {
	GList *pIconList;
	CairoDockMeasure *pMeasureTimer;
	gchar *cDisksURI;
	gchar *cNetworkURI;
	gchar *cBookmarksURI;
	} AppletData;


#endif
