
#ifndef __APPLET_STRUCT__
#define  __APPLET_STRUCT__

#include <cairo-dock.h>

#define SHORTCUTS_DEFAULT_NAME "_shortcuts_"


typedef enum {
	CD_SHOW_NOTHING=0,
	CD_SHOW_FREE_SPACE,
	CD_SHOW_USED_SPACE,
	CD_SHOW_FREE_SPACE_PERCENT,
	CD_SHOW_USED_SPACE_PERCENT,
	CD_NB_SHOW
} CDSwictherDisplayType;

struct _AppletConfig {
	gboolean bListDrives;
	gboolean bListNetwork;
	gboolean bListBookmarks;
	gboolean bUseSeparator;
	CDSwictherDisplayType iDisplayType;
	gint iCheckInterval;
	gboolean bDrawBar;
	gchar *cRenderer;
	} ;

typedef struct _CDDiskUsage {
	long long iPrevAvail;
	long long iAvail;
	long long iFree;
	long long iTotal;
	long long iUsed;
	int iType;
	} CDDiskUsage;

struct _AppletData {
	GList *pIconList;
	CairoDockTask *pTask;
	gchar *cDisksURI;
	gchar *cNetworkURI;
	gchar *cBookmarksURI;
	
	CairoDockTask *pDiskTask;
	GList *pDiskUsageList;
	gboolean bDisksHaveChanged;
	} ;


#endif
