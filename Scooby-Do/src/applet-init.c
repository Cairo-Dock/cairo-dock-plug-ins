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

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-session.h"
#include "applet-search.h"
#include "applet-listing.h"
#include "applet-backend-files.h"
#include "applet-backend-web.h"
#include "applet-backend-command.h"
#include "applet-backend-firefox.h"
#include "applet-backend-recent.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN ("Scooby-Do",
	2, 1, 4,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	("This plug-in allows you to make different actions directly from the keyboard.\n"
	"It is triggered by a keyboard shortcut (by default: CTRL + Enter):\n"
	"It lets you find and launch applications, files, recent files, firefox bookmarks, commands, and even calculations.\n"
	"Type what you want to search, the results will be displayed in real time.\n"
	"The first results of each category are displayed in the main listing.\n"
	"Use the up/down arrows to navigate inside the list,\n"
	" and use the left/right arrows to enter into a category, or to display more actions (when a little arrow is drawn next to text).\n"
	"Once inside a category, you can filter the results by typing some letters.\n"
	"Press Enter to validate, maintain SHIFT or ALT to keep the list of results opened.\n"
	"Escape or the same shortkey will cancel."),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE_END

#define _register_backends(...) do {\
	if (myConfig.bUseFiles)\
		cd_do_register_files_backend ();\
	if (myConfig.bUseWeb)\
		cd_do_register_web_backend ();\
	if (myConfig.bUseCommand)\
		cd_do_register_command_backend ();\
	if (myConfig.bUseFirefox)\
		cd_do_register_firefox_backend ();\
	if (myConfig.bUseRecent)\
		cd_do_register_recent_backend (); } while (0)
//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	cairo_dock_register_notification_on_object (&myContainersMgr,
		NOTIFICATION_KEY_PRESSED,
		(CairoDockNotificationFunc) cd_do_key_pressed,
		CAIRO_DOCK_RUN_AFTER, NULL);
	
	myData.cKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortkeySearch,
		D_("Enable/disable the Finder"),
		"Configuration", "shortkey search",
		(CDBindkeyHandler) cd_do_on_shortkey_search);
	
	_register_backends ();
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	cairo_dock_remove_notification_func_on_object (&myContainersMgr,
		NOTIFICATION_KEY_PRESSED,
		(CairoDockNotificationFunc) cd_do_key_pressed, NULL);
	
	cd_keybinder_unbind (myData.cKeyBinding);
	
	cd_do_exit_session ();
	cd_do_stop_all_backends ();
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		cd_do_stop_all_backends ();
		
		cd_do_free_all_backends ();
		
		cd_do_destroy_listing (myData.pListing);
		myData.pListing = NULL;
		
		cd_keybinder_rebind (myData.cKeyBinding, myConfig.cShortkeySearch);
		
		if (myData.sCurrentText != NULL)  // peu probable.
		{
			/// recharger surfaces / textures
			
			cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
		}
		
		_register_backends ();
	}
CD_APPLET_RELOAD_END
