
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

#define CD_STACK_DEFAULT_NAME "Stack"

typedef enum {
	CD_STACK_SORT_BY_NAME=0,
	CD_STACK_SORT_BY_DATE,
	CD_STACK_SORT_BY_TYPE,
	CD_STACK_SORT_MANUALLY,
	CD_STACK_NB_SORT
	} CDStackSortType;


//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gchar **cMimeTypes;
	gchar **cMonitoredDirectory;
	gchar *cRenderer;
	//gboolean bHiddenFiles;
	//gboolean bLocalDir;
	gboolean bFilter;
	//gboolean bUseSeparator;
	CDStackSortType iSortType;
	gchar *cTextIcon;
	gchar *cUrlIcon;
	gboolean bSelectionClipBoard;
	gchar *cStackDir;
} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	/*gint iIconOrder;
	gint iSidTimer;
	gint iNbAnimation;*/
} ;


#endif
