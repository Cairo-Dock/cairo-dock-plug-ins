
#ifndef __APPLET_LOAD_ICONS__
#define  __APPLET_LOAD_ICONS__


#include <cairo-dock.h>


void cd_shortcuts_launch_measure (void);


void cd_shortcuts_load_tree (GList *pIconsList, cairo_t *pCairoContext);
void cd_shortcuts_draw_in_desklet (cairo_t *pCairoContext, gpointer data);

#endif

