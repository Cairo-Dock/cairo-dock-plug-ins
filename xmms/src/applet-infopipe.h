
#ifndef __APPLET_INFOPIPE__
#define  __APPLET_INFOPIPE__


#include <cairo-dock.h>

gchar *cd_xmms_get_pipe(void);

void cd_xmms_read_pipe(gchar *cInfopipeFilePath);


void cd_xmms_launch_measure (void);

void cd_remove_pipes(void);

#endif
