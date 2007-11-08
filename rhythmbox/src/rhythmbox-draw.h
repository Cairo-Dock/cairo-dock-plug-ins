#ifndef __rhythmbox_DRAW__
#define  __rhythmbox_DRAW__

#include <cairo-dock.h>

#include "rhythmbox-struct.h"

void rhythmbox_iconWitness(int animationLenght);
void rhythmbox_setIconSurface(cairo_surface_t *Surface);
void rhythmbox_setIconName(const gchar *IconName);

#endif
