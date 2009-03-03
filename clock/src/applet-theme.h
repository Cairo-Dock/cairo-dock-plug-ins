
#ifndef __CD_CLOCK_THEME__
#define  __CD_CLOCK_THEME__


#include <cairo-dock.h>
#include "applet-struct.h"


void cd_clock_load_theme (CairoDockModuleInstance *myApplet);

void cd_clock_load_back_and_fore_ground (CairoDockModuleInstance *myApplet);

void cd_clock_load_textures (CairoDockModuleInstance *myApplet);

void cd_clock_clear_theme (CairoDockModuleInstance *myApplet, gboolean bClearAll);


#endif
