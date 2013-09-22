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

typedef enum _CDGMenuShowQuit {
	CD_GMENU_SHOW_QUIT_NONE=0,
	CD_GMENU_SHOW_QUIT_LOGOUT,
	CD_GMENU_SHOW_QUIT_SHUTDOWN,
	CD_GMENU_SHOW_QUIT_BOTH,
	CD_GMENU_NB_SHOW_QUIT
	} CDGMenuShowQuit; 

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gchar *cMenuShortkey;
	gchar *cQuickLaunchShortkey;
	gchar *cConfigureMenuCommand;
	gboolean bShowRecent;
	gboolean bLoadSettingsMenu;
	gboolean bDisplayDesc;
	gint iNbRecentItems;
	CDGMenuShowQuit iShowQuit;
	} ;

typedef struct _CDSharedMemory {
	GList *pTrees;
	} CDSharedMemory;


//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	// menu
	GtkWidget *pMenu;
	GList *pTrees;  // list of trees that have been loaded from .menu files
	CairoDockTask *pTask;
	CDGMenuShowQuit iShowQuit;
	gboolean bLoadSettingsMenu;
	gint iPanelDefaultMenuIconSize;
	gboolean bShowMenuPending;
	// new apps
	gboolean bFirstLaunch;
	GHashTable *pKnownApplications;  // table of all applications that could go in the menu, including ignored ones
	GList *pNewApps;  // a list of GAppInfo that were not present before.
	// entry
	GtkWidget *pEntry;
	GSList *pApps; // a (singly linked) list of GAppInfo: better to check all items
	// recent files sub-menu
	GtkWidget *pRecentMenuItem;
	gint iNbRecentItems;
	// quick-launcher
	CairoDialog *pQuickLaunchDialog;
	GHashTable *dir_hash;
	GList *possible_executables;
	GList *completion_items;
	GCompletion *completion;
	gboolean completion_started;
	GldiShortkey *cKeyBinding;
	GldiShortkey *cKeyBindingQuickLaunch;
	} ;


#endif
