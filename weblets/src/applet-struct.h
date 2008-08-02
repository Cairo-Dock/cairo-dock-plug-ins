
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__


#define CD_APPLET_MULTI_INSTANCE 1

#include <gtk/gtk.h>
#include <cairo-dock.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_WEBKIT
#include <webkit/webkit.h>
#endif

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	   gchar *cURI_to_load;
	   gboolean bShowScrollbars;
	   gint iPosScrollX;
	   gint iPosScrollY;
	   guint iReloadTimeout;
	   gchar **cListURI;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
      CairoDialog *dialog;
	    GtkWidget *pGtkMozEmbed;
#if HAVE_WEBKIT
	    WebKitWebView *pWebKitView;
#endif
	    CairoDockMeasure *pRefreshTimer;
	} ;


#endif
