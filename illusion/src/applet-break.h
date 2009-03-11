
#ifndef __APPLET_BREAK__
#define  __APPLET_BREAK__


#include <cairo-dock.h>


gboolean cd_illusion_init_break (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData, double dt);


gboolean cd_illusion_update_break (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData);


void cd_illusion_draw_break_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData);


#endif
