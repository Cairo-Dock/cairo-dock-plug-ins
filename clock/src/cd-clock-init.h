
#ifndef __CD_CLOCK_INIT__
#define  __CD_CLOCK_INIT__


#include <cairo-dock.h>


gchar *cd_clock_pre_init (void);


Icon *cd_clock_init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur);


void cd_clock_stop (void);


gboolean cd_clock_action (void);


#endif

