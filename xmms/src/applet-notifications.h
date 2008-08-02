
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>

void cd_xmms_prev(CairoDockModuleInstance *myApplet);
void cd_xmms_pp(CairoDockModuleInstance *myApplet);
void cd_xmms_s(CairoDockModuleInstance *myApplet);
void cd_xmms_next(CairoDockModuleInstance *myApplet);
void cd_xmms_shuffle(CairoDockModuleInstance *myApplet);
void cd_xmms_repeat(CairoDockModuleInstance *myApplet);
void cd_xmms_jumpbox(CairoDockModuleInstance *myApplet);
void cd_xmms_enqueue(CairoDockModuleInstance *myApplet, const gchar *cFile);

CD_APPLET_ABOUT_H

CD_APPLET_ON_CLICK_H

CD_APPLET_ON_BUILD_MENU_H

CD_APPLET_ON_MIDDLE_CLICK_H

CD_APPLET_ON_DROP_DATA_H

CD_APPLET_ON_SCROLL_H


#endif
