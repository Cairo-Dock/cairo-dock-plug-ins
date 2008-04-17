
#ifndef __APPLET_INFOPIPE__
#define  __APPLET_INFOPIPE__


#include <cairo-dock.h>

void cd_xmms_acquisition (void);

void cd_xmms_read_data (void);

/*
gchar *cd_xmms_get_pipe(void);

void cd_xmms_read_pipe(gchar *cInfopipeFilePath);


void cd_xmms_launch_measure (void);*/

void cd_xmms_remove_pipes(void);

#endif
