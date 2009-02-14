
#ifndef __CD_CLOCK_DRAW__
#define  __CD_CLOCK_DRAW__


#include <cairo-dock.h>
#include "applet-struct.h"


void cd_clock_free_alarm (CDClockAlarm *pAlarm);


gboolean cd_clock_update_with_time (CairoDockModuleInstance *myApplet);


void cd_clock_draw_text (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime);


void cd_clock_draw_analogic (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime);


void cd_clock_draw_analogic_opengl (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime);

void cd_clock_render_analogic_to_texture (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime);


gboolean cd_clock_update_icon_slow (CairoDockModuleInstance *myApplet, Icon *pIcon, CairoContainer *pContainer, gboolean *bContinueAnimation);


#endif

