
#ifndef __APPLET_STORM__
#define  __APPLET_STORM__


#include <cairo-dock.h>
#include "applet-fire.h"


#define cd_icon_effect_load_storm_texture cd_icon_effect_load_fire_texture


gboolean cd_icon_effect_update_storm_system (CairoParticleSystem *pParticleSystem, CairoDockRewindParticleFunc pRewindParticle);


CairoParticleSystem *cd_icon_effect_init_storm (Icon *pIcon, CairoDock *pDock, double dt);


void cd_icon_effect_rewind_storm_particle (CairoParticle *p, double dt);


#endif
