
#ifndef __LOGOUT_INIT__
#define  __LOGOUT_INIT__


#include <cairo-dock.h>


gchar *cd_logout_pre_init (void);


Icon *cd_logout_init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur);


void cd_logout_stop (void);


#endif

