
#ifndef __APPLET_NVIDIA__
#define  __APPLET_NVIDIA__


#include <cairo-dock.h>

void cd_nvidia_acquisition (void);
static gboolean _nvidia_get_values_from_file (gchar *cContent);
void cd_nvidia_read_data (void);
void cd_nvidia_update_from_data (void);

#endif
