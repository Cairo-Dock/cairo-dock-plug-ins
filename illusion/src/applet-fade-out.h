
#ifndef __APPLET_FADE_OUT__
#define  __APPLET_FADE_OUT__


#include <cairo-dock.h>


gboolean cd_illusion_init_fade_out (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData);


void cd_illusion_update_fade_out (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData);


void cd_illusion_draw_fade_out_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData);


#endif
