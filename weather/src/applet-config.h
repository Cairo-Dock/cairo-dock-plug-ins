
#ifndef __APPLET_CONFIG__
#define  __APPLET_CONFIG__

#include <cairo-dock.h>


CD_APPLET_CONFIG_H

void cd_weather_free_location_list (void);

void cd_weather_reset_all_datas (CairoDockModuleInstance *myApplet);

void cd_weather_load_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile* pKeyFile);


#endif
