
#ifndef __APPLET_MESH_FACTORY__
#define  __APPLET_MESH_FACTORY__


#include <cairo-dock.h>
#include <applet-struct.h>


GLuint cairo_dock_load_capsule_calllist (void);

GLuint cairo_dock_load_ring_calllist (void);

GLuint cairo_dock_load_square_calllist (void);

GLuint cairo_dock_load_cube_calllist (void);

GLuint cd_animations_load_mesh (CDAnimationsMeshType iMeshType);

#endif
