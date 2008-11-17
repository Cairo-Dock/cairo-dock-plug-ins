
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>
#include "applet-struct.h"


gboolean cd_show_mouse_render (gpointer pUserData, CairoContainer *pContainer);


gboolean cd_show_mouse_update_container (gpointer pUserData, CairoContainer *pContainer, gboolean *bContinueAnimation);


gboolean cd_show_mouse_enter_container (gpointer pUserData, CairoContainer *pContainer, gboolean *bStartAnimation);


gdouble *cd_show_mouse_init_sources (void);
CairoParticleSystem *cd_show_mouse_init_system (CairoContainer *pContainer, double dt, double *pSourceCoords);

void cd_show_mouse_update_sources (CDShowMouseData *pData);
void cd_show_mouse_update_particle_system (CairoParticleSystem *pParticleSystem, CDShowMouseData *pData);


#endif
