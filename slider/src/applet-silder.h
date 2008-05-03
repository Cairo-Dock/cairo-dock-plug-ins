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
GList* cd_slider_get_previous_img(GList *pList, GList *pImg);
cairo_surface_t* cd_slider_get_previous_img_surface(GList *pList, GList *pImg);
void _printList (GList *pList);
GList* _slider_random_image(void);
void _slider_free_list(GList *pList);

#endif
