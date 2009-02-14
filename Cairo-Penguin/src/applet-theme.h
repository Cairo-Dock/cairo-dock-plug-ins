
#ifndef __APPLET_LOAD_ICONS__
#define  __APPLET_LOAD_ICONS__

#include <cairo-dock.h>


void penguin_load_theme (CairoDockModuleInstance *myApplet, gchar *cThemePath);


void penguin_load_animation_buffer (PenguinAnimation *pAnimation, cairo_t *pSourceContext, double fAlpha, gboolean bLoadTexture);


#endif
