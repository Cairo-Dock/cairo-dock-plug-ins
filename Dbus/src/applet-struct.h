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


#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

#define CD_DBUS_APPLETS_FOLDER "third-party"

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bEnableReboot;
	gboolean bEnableDesklets;
	gboolean bEnableQuit;
	gboolean bEnableShowDock;
	gboolean bEnableSetQuickInfo;
	gboolean bEnableSetLabel;
	gboolean bEnableSetIcon;
	gboolean bEnablePopUp;
	gboolean bEnableAnimateIcon;
	gboolean bEnableAddRemove;
	gboolean bEnableGetProps;
	gboolean bEnableSetMenu;
	gboolean bEnableSetProgress;
	gboolean bLaunchLauncherAPIDaemon;
	} ;

typedef struct _DBusAppletData {
	GDBusConnection *connection;
	GldiModuleInstance *pModuleInstance;
	gchar *cModuleName;
	gint id;
	gchar *cBusPath;
	gchar *cBusPathSub;
	CairoDialog *pDialog;
	GList *pShortkeyList;
	guint uRegApplet;
	guint uRegSubApplet;
} DBusAppletData;

typedef enum {
	CLIC=0,
	MIDDLE_CLIC,
	SCROLL,
	BUILD_MENU,
	MENU_SELECT,
	DROP_DATA,
	CHANGE_FOCUS,
	RELOAD_MODULE,
	INIT_MODULE,
	STOP_MODULE,
	ANSWER,
	ANSWER_DIALOG,
	SHORTKEY,
	NB_SIGNALS
} CDSignalEnum;


#ifdef DBUSMENU_GTK_FOUND
#include <libdbusmenu-gtk/menuitem.h>
#include <libdbusmenu-gtk/client.h>

typedef struct _CDIconData {
	gchar *cMenuPath;
	gchar *cBusName;
	DbusmenuGtkClient *client;
	GList *menu_items_list;
} CDIconData;
#endif

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	guint uRegMainObject;
	const gchar *cProgName;
	gchar *cBasePath;
	///GtkWidget *pModuleSubMenu;
	GtkWidget *pModuleMainMenu;
	DBusAppletData *pCurrentMenuDbusApplet;
	gint iMenuPosition;
	GldiWindowActor *pActiveWindow;;
	GldiTask *pGetListTask;
	GList *pUpdateTasksList;
	gboolean bDisabled; // if DBus is unavailable
	} ;


#endif
