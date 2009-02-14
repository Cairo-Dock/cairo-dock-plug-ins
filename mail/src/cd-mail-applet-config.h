
#ifndef __APPLET_CONFIG__
#define  __APPLET_CONFIG__

#include <cairo-dock.h>
#include "cd-mail-applet-struct.h"

CD_APPLET_CONFIG_H

typedef void (*cd_mail_fill_account)(CDMailAccount *mailaccount, GKeyFile *pKeyFile, gchar *mailbox_name);
typedef void (*cd_mail_create_account)( GKeyFile *pKeyFile, gchar *pMailAccountName );

void cd_mail_load_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile* pKeyFile);

void cd_mail_save_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile *pKeyFile);

#endif
