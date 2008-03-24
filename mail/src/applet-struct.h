
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <glib.h>

#include "mailwatch.h"
#include "cairo-dock.h"

typedef struct {
        gchar *cNoMailUserImage;
        gchar *cHasMailUserImage;
        gchar *cMailApplication;
	} AppletConfig;

typedef struct {
        XfceMailwatch *mailwatch;
        guint iNbUnreadMails;
	} AppletData;


#endif
