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


#ifndef __LOGOUT_STRUCT__
#define  __LOGOUT_STRUCT__

#include <cairo-dock.h>

#define CD_REBOOT_NEEDED_FILE "/var/run/reboot-required"

typedef enum {
	CD_LOGOUT=0,
	CD_SHUTDOWN,
	CD_LOCK_SCREEN,
	CD_NB_ACTIONS
	} CDActionsEnum;

struct _AppletConfig {
	gchar *cUserAction;
	gchar *cUserAction2;
	CDActionsEnum iActionOnClick;
	CDActionsEnum iActionOnMiddleClick;
	gint iShutdownTime;  // time_t
	gchar *cDefaultLabel;
	} ;

struct _AppletData {
	guint iSidTimer;
	gboolean bRebootNeeded;
	} ;

#endif
