#ifndef __APPLET_WIFI__
#define  __APPLET_WIFI__

#include <cairo-dock.h>


void cd_wifi_acquisition (void);
void cd_wifi_read_data (void);
gboolean cd_wifi_update_from_data (void);


#endif
