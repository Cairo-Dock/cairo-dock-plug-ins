
#ifndef __APPLET_CONFIG__
#define  __APPLET_CONFIG__

#include <cairo-dock.h>


CD_APPLET_CONFIG_H


void reset_config (void);

void reset_data (void);


void mixer_write_elements_list (gchar *cConfFilePath, GKeyFile *pKeyFile);

#endif
