#ifndef __APPLET_WIFI__
#define  __APPLET_WIFI__

#include <cairo-dock.h>

gboolean cd_wifi(gchar *origine);
gboolean cd_wifi_getStrength(void);
float pourcent(float x, float y);
//void cd_wifi_init(gchar *origine);
void cd_wifi_wait(gchar *origine);

#endif
