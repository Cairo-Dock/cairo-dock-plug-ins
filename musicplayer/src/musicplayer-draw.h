#ifndef __MUSICPLAYER_DRAW__
#define  __MUSICPLAYER_DRAW__

#include "musicplayer-struct.h"
#include "string.h"

void musicplayer_iconWitness(int animationLenght);

void update_icon(gboolean make_witness);

void music_dialog(void);

void musicplayer_set_surface (MyAppletPlayerStatus iStatus);



#endif
