
#ifndef __APPLET_STRUCT__
#define  __APPLET_STRUCT__

#include <glib.h>
#include <cairo.h>

#define SHORTCUTS_DEFAULT_NAME "_shortcuts_"

typedef struct {
	gboolean bListDrives;
	gboolean bListNetwork;
	gboolean bListBookmarks;
	gboolean bUseSeparator;
	gchar *cRenderer;
	} AppletConfig;


typedef struct {
	GList *pDeskletIconList;
	gint iNbIconsInTree;
	gint iNbBranches;
	gdouble fTreeWidthFactor, fTreeHeightFactor;
	cairo_surface_t *pBrancheSurface[2];
	gchar *cDisksURI;
	gchar *cNetworkURI;
	gchar *cBookmarksURI;
	} AppletData;


#endif
