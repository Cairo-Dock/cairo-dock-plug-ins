
#ifndef __CD_DUSTBIN_INIT__
#define  __CD_DUSTBIN_INIT__


#include <cairo-dock.h>


gchar *pre_init (void);


Icon *init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur);


void stop (void);


#endif

