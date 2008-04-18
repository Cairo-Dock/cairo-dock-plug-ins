#ifndef __APPLET_NETSPEED__
#define  __APPLET_NETSPEED__

#include <cairo-dock.h>
gboolean inDebug;
gboolean cd_netspeed_timer(gpointer data);
gboolean cd_netspeed_getRate(void);
void cd_netspeed_get_data (void);
void cd_netspeed_launch_analyse(void);

void cd_netspeed_read_data (void);
void cd_netspeed_formatRate(unsigned long long rate, gchar* debit);
void cd_netspeed_update_from_data (void);

#endif
