
#ifndef __APPLET_BLACK_HOLE__
#define  __APPLET_BLACK_HOLE__


#include <cairo-dock.h>


gboolean cd_illusion_init_black_hole (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData);


void cd_illusion_update_black_hole (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData);


void cd_illusion_draw_black_hole_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData);


#endif
