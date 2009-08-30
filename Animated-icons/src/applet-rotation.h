
#ifndef __APPLET_ICON_ROTATION__
#define  __APPLET_ICON_ROTATION__

#include <cairo-dock.h>
#include <applet-struct.h>


void cd_animations_init_rotation (CDAnimationData *pData, double dt, gboolean bUseOpenGL);

#define cd_animation_load_chrome_texture(...) CD_APPLET_LOAD_LOCAL_TEXTURE ("texture-chrome.png")

void cd_animation_render_capsule (Icon *pIcon, CairoDock *pDock, gboolean bInvisibleBackground);

void cd_animation_render_cube (Icon *pIcon, CairoDock *pDock, gboolean bInvisibleBackground);

void cd_animation_render_square (Icon *pIcon, CairoDock *pDock, gboolean bInvisibleBackground);


void cd_animations_draw_rotating_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData);

void cd_animations_draw_rotating_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext);


gboolean cd_animations_update_rotating (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, gboolean bUseOpenGL, gboolean bWillContinue);


#endif
