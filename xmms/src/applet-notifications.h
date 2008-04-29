
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>

void cd_xmms_prev();
void cd_xmms_pp();
void cd_xmms_s();
void cd_xmms_next();
void cd_xmms_shuffle();
void cd_xmms_repeat();
void cd_xmms_jumpbox();
void cd_xmms_enqueue(const gchar *cFile);

CD_APPLET_ABOUT_H

CD_APPLET_ON_CLICK_H

CD_APPLET_ON_BUILD_MENU_H

CD_APPLET_ON_MIDDLE_CLICK_H

CD_APPLET_ON_DROP_DATA_H

CD_APPLET_ON_SCROLL_H


#endif
