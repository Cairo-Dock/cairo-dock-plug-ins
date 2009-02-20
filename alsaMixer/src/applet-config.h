
#ifndef __APPLET_CONFIG__
#define  __APPLET_CONFIG__

#include <cairo-dock.h>


CD_APPLET_CONFIG_H


//void mixer_write_elements_list (gchar *cConfFilePath, GKeyFile *pKeyFile);

void cd_mixer_load_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile* pKeyFile);


#endif
