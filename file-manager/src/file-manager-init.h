
#ifndef __FILE_MANAGER_INIT__
#define  __FILE_MANAGER_INIT__


#include <cairo-dock.h>


Icon *file_manager_init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur);


void file_manager_stop (void);


#endif

