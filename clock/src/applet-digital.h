
#ifndef __CD_CLOCK_DIGITAL__
#define  __CD_CLOCK_DIGITAL__


#include <cairo-dock.h>
#include "applet-struct.h"

void cd_clock_configure_digital (CairoDockModuleInstance *myApplet);
void cd_clock_digital_load_frames (CairoDockModuleInstance *myApplet);
void cd_clock_draw_frames (CairoDockModuleInstance *myApplet);
void cd_clock_put_text_on_frames (CairoDockModuleInstance *myApplet, int width, int height, double fMaxScale, struct tm *pTime);
void cd_clock_draw_ampm (CairoDockModuleInstance *myApplet, gchar *cMark);
void cd_clock_draw_text_from_surface (CairoDockModuleInstance *myApplet, int iNumber);
void cd_clock_fill_text_surface (CairoDockModuleInstance *myApplet, gchar *cStr, int iNumber);
void cd_clock_draw_date_on_frame (CairoDockModuleInstance *myApplet);


#endif
