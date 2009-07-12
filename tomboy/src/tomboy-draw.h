#ifndef __TOMBOY_DRAW__
#define  __TOMBOY_DRAW__

#include <cairo-dock.h>


void load_all_surfaces(void);
void update_icon(void);


void cd_tomboy_show_results (GList *pIconsList);
void cd_tomboy_reset_icon_marks (gboolean bForceRedraw);


void cd_tomboy_draw_content_on_icon (cairo_t *pIconContext, Icon *pIcon);
void cd_tomboy_draw_content_on_all_icons (void);

void cd_tomboy_reload_desklet_renderer (void);


#endif
