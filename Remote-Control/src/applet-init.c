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

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-session.h"
#include "applet-init.h"
#include <cairo-dock-wayland-manager.h>


CD_APPLET_DEFINE2_BEGIN ("Remote-Control",
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This plug-in lets you control your dock from the keyboard\n"
	"Press the shortcut (by default Super + Return), then either:\n"
	" - press the number of the icon that you want to activate"
	" - or use the arrows to navigate into the docks and sub-docks (Ctrl + Page up/down to change to another main dock)\n"
	" - or type the name of a launcher and press Tab to automatically jump to the next suitable launcher\n"
	"Press Enter to click on the icon, Shift+Enter for Shift+click, Alt+Enter for middle click, and Ctrl+Enter for left click\n"
	"Escape or the same shortkey will cancel."),
	"Fabounet (Fabrice Rey)")
	
	if (gldi_container_is_wayland_backend ())
		if (strcmp (gldi_wayland_get_detected_compositor(), "Wayfire"))
			return FALSE;
	
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
	CD_APPLET_REDEFINE_TITLE (N_("Control from keyboard"))
CD_APPLET_DEFINE2_END

static gboolean _menu_request (gpointer, GldiManager*)
{
	gldi_container_present (CAIRO_CONTAINER (g_pMainDock)); // currently no-op
	gldi_wayland_grab_keyboard (CAIRO_CONTAINER (g_pMainDock)); // try to grab the keyboard
	cd_do_on_shortkey_nav (NULL, NULL);
	return GLDI_NOTIFICATION_INTERCEPT;
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	myData.pKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortkey,
		D_("Enable/disable the keyboard control of the dock"),
		"Configuration", "shortkey",
		(CDBindkeyHandler) cd_do_on_shortkey_nav);
	// allow triggering with Wayfire's toggle_menu signal -- use GLDI_RUN_FIRST to have higher priority than GMenu that normally takes this signal
	gldi_object_register_notification (&myDesktopMgr, NOTIFICATION_MENU_REQUEST, (GldiNotificationFunc)_menu_request , GLDI_RUN_FIRST, NULL);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	cd_do_exit_session ();
	gldi_object_remove_notification (&myDesktopMgr, NOTIFICATION_MENU_REQUEST, (GldiNotificationFunc)_menu_request, NULL);

	gldi_object_unref (GLDI_OBJECT(myData.pKeyBinding));
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		cd_do_exit_session ();
		gldi_shortkey_rebind (myData.pKeyBinding, myConfig.cShortkey, NULL);
	}
CD_APPLET_RELOAD_END
