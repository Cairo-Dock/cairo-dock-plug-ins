
#ifndef __RENDERING_INIT__
#define  __RENDERING_INIT__


#include <cairo-dock.h>


CairoDockVisitCard *pre_init (void);


Icon *init (CairoDock *pDock, CairoDockModule *pModule, GError **erreur);


void stop (void);


#endif
