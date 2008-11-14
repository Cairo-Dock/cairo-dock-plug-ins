
#ifndef __APPLET_RAYS__
#define  __APPLET_RAYS__


#include <cairo-dock.h>


#define cd_animations_load_rays_texture(...) CD_APPLET_LOAD_LOCAL_TEXTURE ("ray.png")


gboolean cd_animations_update_rays_system (CairoParticleSystem *pParticleSystem, gboolean bContinue);


CairoParticleSystem *cd_animations_init_rays (Icon *pIcon, CairoDock *pDock, double dt);


void cd_animations_rewind_rays_particle (CairoParticle *p, double dt, double fHeight);


#endif
