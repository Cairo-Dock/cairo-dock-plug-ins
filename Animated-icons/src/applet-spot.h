
#ifndef __APPLET_SPOT__
#define  __APPLET_SPOT__


#include <cairo-dock.h>

#define cd_animation_load_halo_texture(...) CD_APPLET_LOAD_LOCAL_TEXTURE ("halo.png")
#define cd_animation_load_spot_texture(...) CD_APPLET_LOAD_TEXTURE_WITH_DEFAULT (myConfig.cSpotImage, "spot.png")
#define cd_animation_load_spot_front_texture(...) CD_APPLET_LOAD_TEXTURE_WITH_DEFAULT (myConfig.cSpotFrontImage, "spot-front2.png")

void cd_animations_init_spot (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt);

void cd_animation_render_spot (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor);

void cd_animation_render_halo (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor, int iHaloRotationAngle);

void cd_animation_render_spot_front (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor);

gboolean cd_animations_update_spot (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bWillContinue);


#endif
