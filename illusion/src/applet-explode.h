
#ifndef __APPLET_EXPLODE__
#define  __APPLET_EXPLODE__


#include <cairo-dock.h>


gboolean cd_illusion_init_explode (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData, double dt);


gboolean cd_illusion_update_explode (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData);


void cd_illusion_draw_explode_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData);


#endif
