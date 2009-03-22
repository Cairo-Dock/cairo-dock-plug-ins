#ifndef __CD_3DCOVER_DRAW__
#define  __CD_3DCOVER_DRAW__


#include <cairo-dock.h>
#include "rhythmbox-struct.h"


void cd_opengl_load_external_conf_theme_values (CairoDockModuleInstance *myApplet);

GLuint cd_opengl_load_texture (CairoDockModuleInstance *myApplet, gchar *texture);

void cd_opengl_init_opengl_datas (void);

void cd_opengl_scene (CairoDockModuleInstance *myApplet, int iWidth, int iHeight);

void cd_opengl_render_to_texture (CairoDockModuleInstance *myApplet, int iWidth, int iHeight);

gboolean cd_opengl_test_update_icon_slow (CairoDockModuleInstance *myApplet, Icon *pIcon, CairoContainer *pContainer, gboolean *bContinueAnimation);

#endif //__CD_3DCOVER_DRAW__
