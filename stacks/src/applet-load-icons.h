
#ifndef __APPLET_LOAD_ICONS__
#define  __APPLET_LOAD_ICONS__


#include <cairo-dock.h>

void cd_stacks_build_icons (void);
void cd_stacks_destroy_icons (void);
void cd_stacks_update (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon);
void cd_stacks_reload (void);
void cd_stacks_debug_icon(Icon *pIcon);

#endif
