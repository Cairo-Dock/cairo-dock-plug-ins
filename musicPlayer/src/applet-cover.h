#ifndef __APPLET_COVER__
#define  __APPLET_COVER__

#include <cairo-dock.h>
#include "applet-struct.h"

gboolean _cd_proceed_download_cover (gpointer p);

gboolean cd_download_musicPlayer_cover (gpointer data);

gchar *cd_check_musicPlayer_cover_exists (gchar *cURI, MySupportedPlayers iSMP);

#endif
