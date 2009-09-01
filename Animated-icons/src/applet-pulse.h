
#ifndef __APPLET_PULSE__
#define  __APPLET_PULSE__


#include <cairo-dock.h>


void cd_animations_init_pulse (CDAnimationData *pData, double dt);


gboolean cd_animations_update_pulse (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, gboolean bUseOpenGL);


void cd_animations_draw_pulse_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData);

void cd_animations_draw_pulse_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext);


#endif
