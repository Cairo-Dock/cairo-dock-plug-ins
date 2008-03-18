
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>
#include "mailwatch.h"

CD_APPLET_ABOUT_H

CD_APPLET_ON_CLICK_H

CD_APPLET_ON_BUILD_MENU_H

void mailwatch_new_messages_changed_cb(XfceMailwatch *mailwatch, gpointer arg, gpointer user_data);

#endif
