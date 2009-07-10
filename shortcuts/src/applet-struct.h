
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
} CDDiskUsageDisplayType;

typedef enum {
	CD_DESKLET_SLIDE=0,
	CD_DESKLET_TREE,
	CD_DESKLET_NB_RENDERER
} CDDeskletRendererType;

struct _AppletConfig {
	gboolean bListDrives;
	gboolean bListNetwork;
	gboolean bListBookmarks;
	gboolean bUseSeparator;
	CDDiskUsageDisplayType iDisplayType;
	gint iCheckInterval;
	gboolean bDrawBar;
	gchar *cRenderer;
	CDDeskletRendererType iDeskletRendererType;
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
	// shared memory for the loading task
	GList *pIconList;
	// end of shared memory
	CairoDockTask *pTask;
	gchar *cDisksURI;
	gchar *cNetworkURI;
	gchar *cBookmarksURI;
	
	CairoDockTask *pDiskTask;
	// shared memory for the disk usage task
	GList *pDiskUsageList;
	// end of shared memory
	} ;


#endif
