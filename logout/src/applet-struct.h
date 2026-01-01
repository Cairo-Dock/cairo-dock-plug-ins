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
	gchar *cUserActionLock;  // custom screen lock command
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


// struct representing one user, as got from org.freedesktop.Accounts
typedef struct _CDLogoutUser {
	guint64 uid; // user ID
	gchar *cUserName; // user name
	gchar *cRealName; // display name
	gchar *cObjectPath; // DBus object path -- mainly to track the UserDeleted signal
	GCancellable *pCancel; // needed to cancel getting the user name if user if removed quickly (should not happen though)
	} CDLogoutUser;

// struct representing one active session as got from logind
typedef struct _CDLogoutSession {
	gchar *cID; // session ID (to be used when activating)
	guint64 uid; // user ID (to be matched with CDLogoutUser)
	CDLogoutUser *pUser; // user struct (can be NULL if user is not known)
	GCancellable *pCancel; // needed to cancel getting properties for a newly added session
	} CDLogoutSession;

struct _AppletData {
	guint iSidTimer;
	// manual capabilities.
	GldiShortkey *pKeyBinding;
	GldiShortkey *pKeyBinding2;
	GList *pUserList; // CDLogoutUser
	GList *pSessionList; // CDLogoutSession
	GDBusProxy *pAccountsProxy; // -> org.freedesktop.Accounts
	guint uRegUserChanged; // subscription ID to the org.freedesktop.Accounts.User.Changed signal (for any user)
	GDBusProxy *pLogin1Proxy; // -> org.freedesktop.login1
	GDBusProxy *pGdmProxy; // -> org.gnome.DisplayManager
	GCancellable *pCancellable; // to cancel DBus calls related to the above
	// shut-down confirmation
	CairoDialog *pConfirmationDialog;
	gint iDesiredIconSize;
	} ;

#endif
