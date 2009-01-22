
#ifndef __CD_CLOCK_CONFIG__
#define  __CD_CLOCK_CONFIG__


#include <cairo-dock.h>


CD_APPLET_CONFIG_H

void cd_clock_load_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile* pKeyFile);

void cd_clock_save_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile *pKeyFile);


#endif
