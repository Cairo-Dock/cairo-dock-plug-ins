
#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__


#include <cairo-dock.h>


void cd_shortcuts_load_tree (GList *pIconsList, cairo_t *pCairoContext);

void cd_shortcuts_draw_in_desklet (cairo_t *pCairoContext, gpointer data);


#endif

