
#ifndef __CD_DUSTBIN_DRAW__
#define  __CD_DUSTBIN_DRAW__


#include <cairo-dock.h>
#include "applet-struct.h"


CdDustbin *cd_dustbin_find_dustbin_from_uri (const gchar *cDustbinPath);

void cd_dustbin_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, CdDustbin *pDustbin);


gboolean cd_dustbin_check_trashes (Icon *icon);


void cd_dustbin_draw_quick_info (gboolean bRedraw);

void cd_dustbin_signal_full_dustbin (void);

#endif
