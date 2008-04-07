
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <glib.h>

//\___________ structure containing the applet's configuration parameters.
typedef struct {
	gboolean bEnablePopUp;
	gboolean bEnableReboot;
	gboolean bEnableDesklets;
	} AppletConfig;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
typedef struct {
	gint no_data;
	} AppletData;


#endif
