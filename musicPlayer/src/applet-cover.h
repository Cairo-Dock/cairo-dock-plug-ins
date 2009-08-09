#ifndef __APPLET_COVER__
#define  __APPLET_COVER__

#include <cairo-dock.h>
#include "applet-struct.h"


void cd_musicplayer_get_cover_path (const gchar *cGivenCoverPath, gboolean bHandleCover);

void cd_musicplayer_dl_cover (void);


#endif
