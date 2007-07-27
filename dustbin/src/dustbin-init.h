
#ifndef __DUSTBIN_INIT__
#define  __DUSTBIN_INIT__


#include <cairo-dock.h>


Icon *dustbin_init (cairo_t *pSourceContext, GError **erreur);


void dustbin_stop (void);


gboolean dustbin_config (void);


gboolean dustbin_action (void);


#endif

