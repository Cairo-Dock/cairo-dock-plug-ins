/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


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
