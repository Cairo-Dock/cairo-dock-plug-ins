#ifndef __APPLET_SILDER__
#define  __APPLET_SLIDER__

#include <cairo-dock.h>

#include "applet-struct.h"


#define cd_slider_schedule_next_slide(myApplet) do {\
	if (myData.iTimerID == 0) {\
		myData.iTimerID = g_timeout_add_seconds (myConfig.iSlideTime, (GSourceFunc) cd_slider_next_slide, (gpointer) myApplet); } } while (0)

#define cd_slider_next_slide_is_scheduled(myApplet) (myData.iTimerID != 0)


void cd_slider_free_image (SliderImage *pImage);
void cd_slider_free_images_list (GList *pList);
void cd_slider_get_files_from_dir(CairoDockModuleInstance *myApplet);


void cd_slider_read_image (CairoDockModuleInstance *myApplet);
gboolean cd_slider_update_transition (CairoDockModuleInstance *myApplet);
gboolean cd_slider_next_slide (CairoDockModuleInstance *myApplet);


#endif
