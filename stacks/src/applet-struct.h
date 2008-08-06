
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include "cairo-dock.h"

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gchar **cMimeTypes;
  gchar **cMonitoredDirectory;
  gchar *cRenderer;
  gboolean bHiddenFiles;
  gboolean bLocalDir;
  gboolean bFilter;
  gboolean bUseSeparator;
};

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	gint iIconOrder;
	gint iSidTimer;
	gint iNbAnimation;
	gchar **cMonitoredDirectory;
	GKeyFile *pKeyFile;
};


#endif
