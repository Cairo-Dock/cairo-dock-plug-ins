
#ifndef __APPLET_ICON_RENDERER__
#define  __APPLET_ICON_RENDERER__


#include <cairo-dock.h>

#define cd_animation_load_chrome_texture(...) CD_APPLET_LOAD_LOCAL_TEXTURE ("texture-chrome.png")
#define cd_animation_load_spot_texture(...) CD_APPLET_LOAD_LOCAL_TEXTURE ("spot.png")
#define cd_animation_load_halo_texture(...) CD_APPLET_LOAD_LOCAL_TEXTURE ("halo.png")
#define cd_animation_load_spot_front_texture(...) CD_APPLET_LOAD_LOCAL_TEXTURE ("spot-front2.png")

void cd_animation_render_capsule (Icon *pIcon, CairoDock *pDock, gboolean bInvisibleBackground);

void cd_animation_render_cube (Icon *pIcon, CairoDock *pDock, gboolean bInvisibleBackground);

void cd_animation_render_square (Icon *pIcon, CairoDock *pDock, gboolean bInvisibleBackground);


void cd_animation_render_spot (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor);
void cd_animation_render_halo (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor, int iHaloRotationAngle);
void cd_animation_render_spot_front (Icon *pIcon, CairoDock *pDock, gdouble fRadiusFactor);


#endif
