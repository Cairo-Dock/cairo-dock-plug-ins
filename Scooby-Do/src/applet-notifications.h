
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>
#include "applet-struct.h"


gboolean cd_do_render (gpointer pUserData, CairoContainer *pContainer, cairo_t *pCairoContext);


gboolean cd_do_update_container (gpointer pUserData, CairoContainer *pContainer, gboolean *bContinueAnimation);


gboolean cd_do_enter_container (gpointer pUserData, CairoContainer *pContainer, gboolean *bStartAnimation);


gboolean cd_do_check_icon_stopped (gpointer pUserData, Icon *pIcon);


gboolean cd_do_key_pressed (gpointer pUserData, CairoContainer *pContainer, guint iKeyVal, guint iModifierType);


void cd_do_on_shortkey (const char *keystring, gpointer data);



// a deplacer dans leur fichier propre.
void cd_do_open_session (void);

void cd_do_close_session (void);

void cd_do_exit_session (void);


#endif
