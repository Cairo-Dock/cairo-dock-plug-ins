
#ifndef __APPLET_SPOT__
#define  __APPLET_SPOT__


#include <cairo-dock.h>


void cd_animation_render_spot (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor);

void cd_animation_render_halo (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor, int iHaloRotationAngle);

void cd_animation_render_spot_front (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor);


#endif
