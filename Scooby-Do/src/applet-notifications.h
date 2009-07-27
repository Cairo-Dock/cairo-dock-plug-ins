
#ifndef __APPLET_NOTIFICATIONS__
#define  __APPLET_NOTIFICATIONS__


#include <cairo-dock.h>
#include "applet-struct.h"


gboolean cd_do_render (gpointer pUserData, CairoContainer *pContainer, cairo_t *pCairoContext);


gboolean cd_do_update_container (gpointer pUserData, CairoContainer *pContainer, gboolean *bContinueAnimation);


//gboolean cd_do_enter_container (gpointer pUserData, CairoContainer *pContainer, gboolean *bStartAnimation);


gboolean cd_do_check_icon_stopped (gpointer pUserData, Icon *pIcon);


gboolean cd_do_check_active_dock (gpointer pUserData, Window *XActiveWindow);


gboolean cd_do_key_pressed (gpointer pUserData, CairoContainer *pContainer, guint iKeyVal, guint iModifierType, const gchar *string);


void cd_do_on_shortkey_nav (const char *keystring, gpointer data);
void cd_do_on_shortkey_search (const char *keystring, gpointer data);



#endif
