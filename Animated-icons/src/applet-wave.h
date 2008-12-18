
#ifndef __APPLET_WAVE__
#define  __APPLET_WAVE__


#include <cairo-dock.h>


void cd_animations_init_wave (CDAnimationData *pData);

gboolean cd_animations_update_wave (CDAnimationData *pData);

void cd_animations_draw_wave_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData);

#endif
