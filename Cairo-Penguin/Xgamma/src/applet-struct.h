
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

/* Minimum extension version required */
#define MINMAJOR 2
#define MINMINOR 0

/* Maximum and Minimum gamma values */
#define GAMMA_MIN 0.1
#define GAMMA_MAX 5.0

#include <glib.h>
#include <gtk/gtk.h>

#include <X11/extensions/xf86vmode.h>

typedef struct {
	gint no_parameter_yet;
	} AppletConfig;

typedef struct {
	gboolean bVideoExtensionOK;
	CairoDockDialog *pDialog;
	GtkWidget *pWidget;
	GtkWidget *pGlobalScale;
	GtkWidget *pRedScale;
	GtkWidget *pGreenScale;
	GtkWidget *pBlueScale;
	guint iGloalScaleSignalID;
	guint iRedScaleSignalID;
	guint iGreenScaleSignalID;
	guint iBlueScaleSignalID;
	XF86VidModeGamma Xgamma;
	XF86VidModeGamma XoldGamma;
	} AppletData;


#endif
