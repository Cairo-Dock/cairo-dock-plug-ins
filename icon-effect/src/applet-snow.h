
#ifndef __APPLET_SNOW__
#define  __APPLET_SNOW__


#include <cairo-dock.h>


#define cd_icon_effect_load_snow_texture(...) CD_APPLET_LOAD_LOCAL_TEXTURE ("snow.png")


#define cd_icon_effect_update_snow_system cairo_dock_update_default_particle_system


CairoParticleSystem *cd_icon_effect_init_snow (Icon *pIcon, CairoDock *pDock, double dt);


void cd_icon_effect_rewind_snow_particle (CairoParticle *p, double dt);


#endif
