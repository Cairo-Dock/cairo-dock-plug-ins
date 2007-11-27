
#ifndef __FILE_MANAGER_INIT__
#define  __FILE_MANAGER_INIT__

#include <cairo-dock.h>


gchar *pre_init (void);


Icon *init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur);


void stop (void);


#endif
