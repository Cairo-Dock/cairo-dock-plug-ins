#ifndef __POWERMANAGER_DRAW__
#define  __POWERMANAGER_DRAW__

#include <cairo-dock.h>
#include "powermanager-struct.h"

void iconWitness(int animationLenght);
void update_icon(void);
void cd_powermanager_bubble(void);
void cd_powermanager_draw_icon_with_effect (gboolean bOnBattery);
gboolean cd_powermanager_alert(MyAppletCharge alert);

#endif
