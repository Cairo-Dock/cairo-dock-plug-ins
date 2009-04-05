
#ifndef __APPLET_DRAW__
#define  __APPLET_DRAW__


#include <cairo-dock.h>


void cd_xeyes_render_to_texture (CairoDockModuleInstance *myApplet, int iWidth, int iHeight);


void cd_xeyes_render_to_surface (CairoDockModuleInstance *myApplet, int iWidth, int iHeight);


#endif
