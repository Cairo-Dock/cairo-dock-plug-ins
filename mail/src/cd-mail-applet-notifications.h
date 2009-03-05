
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>

CD_APPLET_ABOUT_H

CD_APPLET_ON_CLICK_H

CD_APPLET_ON_BUILD_MENU_H

CD_APPLET_ON_MIDDLE_CLICK_H

gboolean cd_mail_update_icon(CairoDockModuleInstance *myApplet, Icon *pIcon, CairoContainer *pContainer, gboolean *bContinueAnimation);
gboolean cd_mail_update_status( CairoDockModuleInstance *myApplet );
void cd_mail_force_update(CairoDockModuleInstance *myApplet);

#endif
