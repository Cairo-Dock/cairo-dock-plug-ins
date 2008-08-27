
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <time.h>

#include "mailwatch.h"
#include <cairo-dock.h>

#define MAIL_DEFAULT_NAME "_Mail_"

struct _AppletConfig {
        gchar *cNoMailUserImage;
        gchar *cHasMailUserImage;
        gchar *cNewMailUserSound;
        gchar *cMailApplication;
        gchar *cThemePath;
        time_t timeEndOfSound; 
	} ;

struct _AppletData {
        XfceMailwatch *mailwatch;
        guint iNbUnreadMails;
        gboolean bNewMailFound;
	} ;

#endif
