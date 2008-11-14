
#ifndef __APPLET_RAIN__
#define  __APPLET_RAIN__


#include <cairo-dock.h>


#define cd_icon_effect_load_rain_texture(...) CD_APPLET_LOAD_LOCAL_TEXTURE ("rain.png")


#define cd_icon_effect_update_rain_system cairo_dock_update_default_particle_system


CairoParticleSystem *cd_icon_effect_init_rain (Icon *pIcon, CairoDock *pDock, double dt);


void cd_icon_effect_rewind_rain_particle (CairoParticle *p, double dt);


#endif
