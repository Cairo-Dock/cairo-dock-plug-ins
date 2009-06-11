#ifndef __RHYTHMBOX_DRAW__
#define  __RHYTHMBOX_DRAW__

#include <rhythmbox-struct.h>

void rhythmbox_add_buttons_to_desklet (void);

void rhythmbox_iconWitness(int animationLenght);

void update_icon (gboolean bCheckTwice);

void music_dialog(void);

void rhythmbox_set_surface (MyAppletPlayerStatus iStatus);

gboolean cd_check_if_size_is_constant (gchar *cFileName);

#endif
