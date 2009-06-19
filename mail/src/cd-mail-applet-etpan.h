
#ifndef __APPLET_ETPAN__
#define  __APPLET_ETPAN__

#include <cairo-dock.h>
#include "cd-mail-applet-struct.h"


void cd_mail_get_folder_data(CDMailAccount *pMailAccount);
gboolean cd_mail_update_account_status( CDMailAccount *pUpdatedMailAccount );

void cd_mail_load_icons( CairoDockModuleInstance *myApplet );
void cd_mail_draw_main_icon (CairoDockModuleInstance *myApplet, gboolean bSignalNewMessages);

void cd_mail_render_3D_to_texture (CairoDockModuleInstance *myApplet);


#endif
