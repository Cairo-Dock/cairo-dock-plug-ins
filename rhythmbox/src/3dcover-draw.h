#ifndef __CD_3DCOVER_DRAW__
#define  __CD_3DCOVER_DRAW__

#include <cairo-dock.h>
#include "rhythmbox-struct.h"


gboolean cd_opengl_load_3D_theme (CairoDockModuleInstance *myApplet, gchar *cThemePath);

void cd_opengl_reset_opengl_datas (CairoDockModuleInstance *myApplet);

void cd_opengl_scene (CairoDockModuleInstance *myApplet, int iWidth, int iHeight);

void cd_opengl_render_to_texture (CairoDockModuleInstance *myApplet);

int cd_opengl_check_buttons_state (CairoDockModuleInstance *myApplet);


#endif //__CD_3DCOVER_DRAW__
