
#ifndef __CD_DUSTBIN_DRAW__
#define  __CD_DUSTBIN_DRAW__


#include <cairo-dock.h>


int cd_dustbin_count_trashes (gchar *cDirectory);

void cd_dustbin_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon);


gboolean cd_dustbin_check_trashes (Icon *icon);


#endif
