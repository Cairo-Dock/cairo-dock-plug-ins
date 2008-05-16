#ifndef __APPLET_SILDER__
#define  __APPLET_SLIDER__


#include <cairo-dock.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

void cd_slider_get_files_from_dir(void);
gboolean cd_slider_draw_images(void);
gboolean cd_slider_fade (void);
gboolean cd_slider_blank_fade (void);
gboolean cd_slider_fade_in_out (void);
gboolean cd_slider_side_kick (void);
gboolean cd_slider_diaporama (void);
gboolean cd_slider_grow_up (void);
gboolean cd_slider_shrink_down (void);

#endif
