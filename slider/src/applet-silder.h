#ifndef __APPLET_SILDER__
#define  __APPLET_SLIDER__

#include <cairo-dock.h>

#include "applet-struct.h"


void cd_slider_free_image (SliderImage *pImage);
void cd_slider_free_images_list (GList *pList);
void cd_slider_get_files_from_dir(void);

void cd_slider_read_directory (void);
void cd_slider_launch_slides (void);

void cd_slider_read_image (void);
void cd_slider_update_slide (void);

gboolean cd_slider_draw_images(void);

gboolean cd_slider_fade (void);
gboolean cd_slider_blank_fade (void);
gboolean cd_slider_fade_in_out (void);
gboolean cd_slider_side_kick (void);
gboolean cd_slider_diaporama (void);
gboolean cd_slider_grow_up (void);
gboolean cd_slider_shrink_down (void);


#endif
