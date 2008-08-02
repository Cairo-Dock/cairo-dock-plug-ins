
#ifndef __APPLET_STRUCT__
#define  __APPLET_STRUCT__

#include <cairo-dock.h>

#define SHORTCUTS_DEFAULT_NAME "_shortcuts_"

struct _AppletConfig {
	gboolean bListDrives;
	gboolean bListNetwork;
	gboolean bListBookmarks;
	gboolean bUseSeparator;
	gchar *cRenderer;
	} ;


struct _AppletData {
	GList *pIconList;
	CairoDockMeasure *pMeasureTimer;
	gchar *cDisksURI;
	gchar *cNetworkURI;
	gchar *cBookmarksURI;
	} ;


#endif
