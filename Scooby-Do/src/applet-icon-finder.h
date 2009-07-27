
#ifndef __APPLET_ICON_FINDER__
#define  __APPLET_ICON_FINDER__

#include <cairo-dock.h>
#include "applet-struct.h"


Icon *cd_do_search_icon_by_command (const gchar *cCommandPrefix, Icon *pAfterIcon, CairoDock **pDock);


void cd_do_change_current_icon (Icon *pIcon, CairoDock *pDock);


void cd_do_search_current_icon (gboolean bLoopSearch);


gboolean cairo_dock_emit_motion_signal (CairoDock *pDock, int iMouseX, int iMouseY);


void cd_do_search_matching_icons (void);

void cd_do_select_previous_next_matching_icon (gboolean bNext);


#endif
