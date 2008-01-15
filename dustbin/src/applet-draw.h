
#ifndef __CD_DUSTBIN_DRAW__
#define  __CD_DUSTBIN_DRAW__


#include <cairo-dock.h>


void cd_dustbin_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon);


gboolean cd_dustbin_check_trashes (Icon *icon);


void cd_dustbin_draw_quick_info (gboolean bRedraw);


#endif
