
#ifndef __APPLET_BOUNCE__
#define  __APPLET_BOUNCE__


#include <cairo-dock.h>


void cd_animations_init_bounce (CairoDock *pDock, CDAnimationData *pData, double dt);


gboolean cd_animations_update_bounce (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL, gboolean bWillContinue);


void cd_animations_draw_bounce_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, int sens);

void cd_animations_draw_bounce_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext, int sens);


#endif
