
#ifndef __APPLET_FIRE__
#define  __APPLET_FIRE__


#include <cairo-dock.h>
#include "fire-tex.h"


#define cd_icon_effect_load_fire_texture(...) cairo_dock_load_texture_from_raw_data (fireTex)


#define cd_icon_effect_update_fire_system cairo_dock_update_default_particle_system


CairoParticleSystem *cd_icon_effect_init_fire (Icon *pIcon, CairoDock *pDock, double dt);


void cd_icon_effect_rewind_fire_particle (CairoParticle *p, double dt);


#endif
