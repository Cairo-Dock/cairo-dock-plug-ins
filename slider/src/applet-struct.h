
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <glib/gi18n.h>
#include <glib/gstdio.h>

//\___________ structure containing the applet's configuration parameters.
typedef struct {
	double dSlideTime;
	gchar *cDirectory;
	gboolean bSubDirs;
} AppletConfig;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
typedef struct {
	GList *pList;
	GList *pElement;
	gboolean bPause;
	int iTimerID;
} AppletData;

#endif
