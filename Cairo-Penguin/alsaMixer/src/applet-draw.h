
#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__


#include <cairo-dock.h>


int mixer_element_update_with_event (snd_mixer_elem_t *elem, unsigned int mask);

void mixer_apply_zoom_effect (cairo_surface_t *pSurface);

void mixer_apply_transparency_effect (cairo_surface_t *pSurface);

void mixer_draw_bar (cairo_surface_t *pSurface);


#endif
