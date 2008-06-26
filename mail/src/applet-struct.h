
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <glib.h>
#include <time.h>

#include "mailwatch.h"
#include "cairo-dock.h"

typedef struct {
        gchar *cNoMailUserImage;
        gchar *cHasMailUserImage;
        gchar *cNewMailUserSound;
        gchar *cMailApplication;
        gchar *cThemePath;
        time_t timeEndOfSound; 
	} AppletConfig;

typedef struct {
        XfceMailwatch *mailwatch;
        guint iNbUnreadMails;
        gboolean bNewMailFound;
	} AppletData;


#endif
