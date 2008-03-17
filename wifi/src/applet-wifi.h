#ifndef __APPLET_WIFI__
#define  __APPLET_WIFI__

#include <cairo-dock.h>

gboolean cd_wifi(Icon *icon);
gboolean cd_wifi_getStrength(Icon *icon);
int pourcent(int x);
void cd_wifi_init(void);
void cd_wifi_wait(void);

#endif
