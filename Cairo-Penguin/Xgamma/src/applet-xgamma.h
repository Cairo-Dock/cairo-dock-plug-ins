
#ifndef __APPLET_XGAMMA__
#define  __APPLET_XGAMMA__

#include <stdio.h>
#include <errno.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>
#include <ctype.h>
#include <stdlib.h>


double xgamma_get_gamma (XF86VidModeGamma *pGamma);

void xgamma_set_gamma (XF86VidModeGamma *pGamma);


void xgamma_create_scales_widget (void);

void xgamma_draw_in_desklet (cairo_t *pCairoContext, gpointer data);

void xgamma_apply_values (int iAnswer, GtkWidget *pWidget, gpointer data);

void xgamma_build_and_show_widget (void);


#endif
