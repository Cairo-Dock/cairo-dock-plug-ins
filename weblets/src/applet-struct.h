
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include "gtk/gtk.h"
#include "cairo-dock.h"

//\___________ structure containing the applet's configuration parameters.
typedef struct {
	   gchar *cURI_to_load;
	   gboolean bShowScrollbars;
	   gint iPosScrollX;
	   gint iPosScrollY;
	   guint iReloadTimeout;
	   gchar **cListURI;
	} AppletConfig;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
typedef struct {
      CairoDialog *dialog;
	    GtkWidget *pGtkMozEmbed;
	    CairoDockMeasure *pRefreshTimer;
	} AppletData;


#endif
