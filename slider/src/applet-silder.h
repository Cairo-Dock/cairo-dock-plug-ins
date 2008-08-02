#ifndef __APPLET_SILDER__
#define  __APPLET_SLIDER__

#include <cairo-dock.h>

#include "applet-struct.h"


void cd_slider_free_image (SliderImage *pImage);
void cd_slider_free_images_list (GList *pList);
void cd_slider_get_files_from_dir(CairoDockModuleInstance *myApplet);

void cd_slider_read_directory (CairoDockModuleInstance *myApplet);
gboolean cd_slider_launch_slides (CairoDockModuleInstance *myApplet);

void cd_slider_read_image (CairoDockModuleInstance *myApplet);
gboolean cd_slider_update_slide (CairoDockModuleInstance *myApplet);

gboolean cd_slider_draw_images(CairoDockModuleInstance *myApplet);

gboolean cd_slider_fade (CairoDockModuleInstance *myApplet);
gboolean cd_slider_blank_fade (CairoDockModuleInstance *myApplet);
gboolean cd_slider_fade_in_out (CairoDockModuleInstance *myApplet);
gboolean cd_slider_side_kick (CairoDockModuleInstance *myApplet);
gboolean cd_slider_diaporama (CairoDockModuleInstance *myApplet);
gboolean cd_slider_grow_up (CairoDockModuleInstance *myApplet);
gboolean cd_slider_shrink_down (CairoDockModuleInstance *myApplet);


#endif
