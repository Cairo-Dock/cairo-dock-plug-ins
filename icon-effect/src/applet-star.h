
#ifndef __APPLET_STAR__
#define  __APPLET_STAR__


#include <cairo-dock.h>


#define cd_icon_effect_load_star_texture(...) CD_APPLET_LOAD_LOCAL_TEXTURE ("star.png")


gboolean cd_icon_effect_update_star_system (CairoParticleSystem *pParticleSystem, CairoDockRewindParticleFunc pRewindParticle);


CairoParticleSystem *cd_icon_effect_init_stars (Icon *pIcon, CairoDock *pDock, double dt);


void cd_icon_effect_rewind_star_particle (CairoParticle *p, double dt);


#endif
