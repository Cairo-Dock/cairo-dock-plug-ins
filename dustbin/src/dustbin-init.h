
#ifndef __CD_DUSTBIN_INIT__
#define  __CD_DUSTBIN_INIT__


#include <cairo-dock.h>


Icon *cd_dustbin_init (cairo_t *pSourceContext, GError **erreur);


void cd_dustbin_stop (void);


gboolean cd_dustbin_config (void);


gboolean cd_dustbin_action (void);


#endif

