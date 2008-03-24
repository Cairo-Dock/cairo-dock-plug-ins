#ifndef __APPLET_NETSPEED__
#define  __APPLET_NETSPEED__

#include <cairo-dock.h>

gboolean cd_netspeed(gchar *origine);
gboolean cd_netspeed_getRate(void);
void cd_netspeed_init(gchar *origine);
void cd_netspeed_wait(gchar *origine);
void cd_netspeed_formatRate(unsigned long long rate, gchar* debit);
#endif
