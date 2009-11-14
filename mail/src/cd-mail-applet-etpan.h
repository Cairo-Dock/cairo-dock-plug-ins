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


#ifndef __APPLET_ETPAN__
#define  __APPLET_ETPAN__

#include <cairo-dock.h>
#include "cd-mail-applet-struct.h"


void cd_mail_get_folder_data(CDMailAccount *pMailAccount);
gboolean cd_mail_update_account_status( CDMailAccount *pUpdatedMailAccount );
void cd_mail_mark_all_mails_as_read(CDMailAccount *pMailAccount);

void cd_mail_load_icons( CairoDockModuleInstance *myApplet );
void cd_mail_draw_main_icon (CairoDockModuleInstance *myApplet, gboolean bSignalNewMessages);

void cd_mail_render_3D_to_texture (CairoDockModuleInstance *myApplet);


#endif
