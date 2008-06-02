#ifndef __APPLET_COMPIZ__
#define  __APPLET_COMPIZ _

#include <cairo-dock.h>


void cd_compiz_start_system_wm (void);

void cd_compiz_start_compiz (void);

void cd_compiz_switch_manager(void);

void cd_compiz_start_favorite_decorator (void);
void cd_compiz_start_decorator (compizDecorator iDecorator);

void cd_compiz_kill_compmgr(void);


void cd_compiz_acquisition (void);
void cd_compiz_read_data (void);
gboolean cd_compiz_update_from_data (void);


#endif
