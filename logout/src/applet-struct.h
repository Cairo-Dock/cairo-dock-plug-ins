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
	CD_POP_UP_MENU,
	CD_NB_ACTIONS
	} CDActionsEnum;

typedef enum {
	CD_DISPLAY_EMBLEM,
	CD_DISPLAY_IMAGE,
	CD_NB_REBOOT_NEEDED_DISPLAYS
	} CDDisplayRebootNeeded;

struct _AppletConfig {
	gchar *cUserAction;  // custom logout command
	gchar *cUserActionShutdown;  // custom shutdown command
	gchar *cUserActionSwitchUser;  // custom switch-user command
	CDActionsEnum iActionOnMiddleClick;
	gint iShutdownTime;  // time_t
	gchar *cEmblemPath;
	gchar *cDefaultLabel;
	gchar *cDefaultIcon;
	gchar *cShortkey;
	gchar *cShortkey2;
	gboolean bConfirmAction;
	CDDisplayRebootNeeded iRebootNeededImage;
	} ;

typedef enum {
	CD_UNKNOWN,
	CD_CONSOLE_KIT,
	CD_LOGIND,
	CD_NB_LOGIN_MANAGER
	} CDLoginManager;
	
typedef struct {
	gboolean bCanHibernate;
	gboolean bCanHybridSleep;
	gboolean bCanSuspend;
	gboolean bCanStop;
	gboolean bCanRestart;
	gboolean bHasGuestAccount;
	CDLoginManager iLoginManager;
	} CDSharedMemory;

struct _AppletData {
	guint iSidTimer;
	// manual capabilities.
	CairoDockTask *pTask;
	gboolean bCapabilitiesChecked;
	gboolean bCanHibernate;
	gboolean bCanHybridSleep;
	gboolean bCanSuspend;
	gboolean bCanStop;
	gboolean bCanRestart;
	gboolean bHasGuestAccount;
	CDLoginManager iLoginManager;
	GldiShortkey *pKeyBinding;
	GldiShortkey *pKeyBinding2;
	GList *pUserList;
	// shut-down confirmation
	gint iCountDown;
	guint iSidShutDown;
	CairoDialog *pConfirmationDialog;
	gint iDesiredIconSize;
	} ;

#endif
