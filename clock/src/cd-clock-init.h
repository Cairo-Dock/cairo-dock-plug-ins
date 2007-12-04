
#ifndef __CD_CLOCK_INIT__
#define  __CD_CLOCK_INIT__


#include <cairo-dock.h>


CairoDockVisitCard *pre_init (void);


Icon *init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur);


void stop (void);



#endif

