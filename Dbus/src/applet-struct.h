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
	gboolean bEnableReloadModule;
	gboolean bEnableActivateModule;
	gboolean bEnableQuit;
	gboolean bEnableShowDock;
	gboolean bEnableTweakingLauncher;
	gboolean bEnableCreateLauncher;
	gboolean bEnableSetQuickInfo;
	gboolean bEnableSetLabel;
	gboolean bEnableSetIcon;
	gboolean bEnablePopUp;
	gboolean bEnableAnimateIcon;
	gboolean bEnableNewModule;
	} ;


typedef struct _dbusMainObject dbusMainObject;
typedef struct _dbusApplet dbusApplet;
typedef struct _dbusSubApplet dbusSubApplet;

struct _dbusMainObject {
	GObject parent;
	DBusGConnection *connection;
};
typedef struct {
	GObjectClass parent_class;
} dbusMainObjectClass;


struct _dbusApplet {
	GObject parent;
	DBusGConnection *connection;
	DBusGProxy *proxy;
	CairoDockModuleInstance *pModuleInstance;
	gchar *cModuleName;
	gint id;
	gchar *cBusPath;
	dbusSubApplet *pSubApplet;
	CairoDialog *pDialog;
	GList *pShortkeyList;
};
typedef struct {
	GObjectClass parent_class;
} dbusAppletClass;


struct _dbusSubApplet {
	GObject parent;
	dbusApplet *pApplet;
};
typedef struct {
	GObjectClass parent_class;
} dbusSubAppletClass;


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

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	dbusMainObject *pMainObject;
	const gchar *cProgName;
	gchar *cBasePath;
	///GtkWidget *pModuleSubMenu;
	GtkWidget *pModuleMainMenu;
	dbusApplet *pCurrentMenuDbusApplet;
	Window xActiveWindow;
	CairoDockTask *pGetListTask;
	GList *pUpdateTasksList;
	} ;


#endif
