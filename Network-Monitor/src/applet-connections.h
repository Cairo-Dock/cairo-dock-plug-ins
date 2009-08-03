#ifndef __APPLET_CONNECTIONS__
#define  __APPLET_CONNECTIONS__

#include <cairo-dock.h>

void cd_wifi_get_data (gpointer data);
gboolean cd_wifi_update_from_data (gpointer data);

gboolean cd_NetworkMonitor_get_active_connection_info (void);
void cd_NetworkMonitor_connect_signals ();
void cd_NetworkMonitor_disconnect_signals();

#endif
