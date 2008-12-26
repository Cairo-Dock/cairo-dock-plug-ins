
#ifndef __APPLET_BLINK__
#define  __APPLET_BLINK__


#include <cairo-dock.h>


void cd_animations_init_blink (CDAnimationData *pData, double dt);


gboolean cd_animations_update_blink (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL);


void cd_animations_draw_blink_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, int sens);


#endif
