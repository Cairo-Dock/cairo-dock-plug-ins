
#ifndef __APPLET_EVAPORATE__
#define  __APPLET_EVAPORATE__


#include <cairo-dock.h>
#include "evaporate-tex.h"


#define cd_illusion_load_evaporate_texture(...) cairo_dock_load_texture_from_raw_data (evaporateTex, 32, 32)


gboolean cd_illusion_init_evaporate (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData, double dt);


gboolean cd_illusion_update_evaporate (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData);


void cd_illusion_draw_evaporate_icon (Icon *pIcon, CairoDock *pDock, CDIllusionData *pData);


#endif
