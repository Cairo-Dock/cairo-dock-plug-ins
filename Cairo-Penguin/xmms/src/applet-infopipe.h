
#ifndef __APPLET_INFOPIPE__
#define  __APPLET_INFOPIPE__


#include <cairo-dock.h>

gboolean cd_xmms_get_pipe();
gboolean cd_xmms_read_pipe(gchar *cInfopipeFilePath);
void cd_xmms_update_title();
int cd_remove_pipes();
void cd_xmms_animat_icon(int animationLength);
void cd_xmms_new_song_playing(void);

#endif
