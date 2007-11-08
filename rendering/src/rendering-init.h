
#ifndef __RENDERING_INIT__
#define  __RENDERING_INIT__


#include <cairo-dock.h>


gchar *cd_rendering_pre_init (void);


Icon *cd_rendering_init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur);


void cd_rendering_stop (void);


#endif

