#ifndef __TOMBOY_DRAW__
#define  __TOMBOY_DRAW__

#include <cairo-dock.h>

void load_surface(cairo_surface_t *pSurface, gchar *default_image, gchar *user_image);
void load_all_surfaces(void);
void update_icon(void);

#endif
