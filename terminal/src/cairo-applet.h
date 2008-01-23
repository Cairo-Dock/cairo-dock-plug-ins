
#ifndef __CAIRO_APPLET_H__
#define  __CAIRO_APPLET_H__

#include <glib.h>

#include "cairo-dock-struct.h"

/* #include <stdlib.h> */
/* #include <string.h> */
/* #include <math.h> */

/* #include "cairo-dock-struct.h" */
/* #include "cairo-dock-icons.h" */
/* #include "cairo-dock-dock-factory.h" */
/* #include "cairo-dock-load.h" */
/* #include "cairo-dock-draw.h" */
/* #include "cairo-dock-dialogs.h" */

gboolean applet_dialog_reference (CairoDockDialog *pDialog);

gboolean applet_dialog_unreference (CairoDockDialog *pDialog);

void applet_isolate_dialog (CairoDockDialog *pDialog);

void applet_free_dialog (CairoDockDialog *pDialog);

CairoDockDialog *applet_build_dialog (CairoDock *pDock, GtkWidget *pInteractiveWidget, gpointer data);


void applet_hide_dialog (CairoDockDialog *pDialog);

void applet_unhide_dialog (CairoDockDialog *pDialog);
















#endif
