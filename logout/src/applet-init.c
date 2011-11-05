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
#include "applet-logout.h"
#include "applet-init.h"

CD_APPLET_DEFINE_BEGIN ("logout",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_APPLET_DESKTOP,
	N_("A very simple applet that adds an icon to log out from your session\n"
	"Left click to log out or change the user, middle click to shutdown or restart\n"
	"  (you can invert the buttons if you prefer to shutdown on left-click)."
	"You can also lock the screen from the menu on right-click,\n"
	"  and program an automatic shutdown at a given time.\n"),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_REDEFINE_TITLE (N_("Log out"))
CD_APPLET_DEFINE_END

static const gchar *s_cShortkeyDescription[CD_NB_ACTIONS] = {"Log out", "Shut down", "Lock screen"};  // same names as in the config file, so no need to add N_()

CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME)  // on prend le controle de l'icone de la fenetre.
		CD_APPLET_MANAGE_APPLICATION ("gnome-session");  // x-session-manager before 2.28
	else if (g_iDesktopEnv == CAIRO_DOCK_XFCE)
		CD_APPLET_MANAGE_APPLICATION ("xfce4-session-logout");  // x-session-manager before 4.8
	else if (g_iDesktopEnv == CAIRO_DOCK_KDE)
		CD_APPLET_MANAGE_APPLICATION ("ksmserver");  /// pas du tout sur...
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	
	// shortkey
	
	myData.pKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortkey,
		D_(s_cShortkeyDescription[myConfig.iActionOnClick]),
		"Configuration", "shortkey",
		(CDBindkeyHandler) cd_logout_on_keybinding_pull);
	
	myData.pKeyBinding2 = CD_APPLET_BIND_KEY (myConfig.cShortkey2,
		D_(s_cShortkeyDescription[myConfig.iActionOnMiddleClick]),
		"Configuration", "shortkey2",
		(CDBindkeyHandler) cd_logout_on_keybinding_pull2);
	
	//\_______________ On (re)lance l'eteignage programme.
	cd_logout_set_timer ();
	
	//\_______________ On surveille le reboot necessaire.
	cairo_dock_fm_add_monitor_full (CD_REBOOT_NEEDED_FILE, FALSE, NULL, (CairoDockFMMonitorCallback) cd_logout_check_reboot_required, NULL);
	cd_logout_check_reboot_required_init ();
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	cd_keybinder_unbind (myData.pKeyBinding);
	cd_keybinder_unbind (myData.pKeyBinding2);
	
	CD_APPLET_MANAGE_APPLICATION (NULL);  // on relache le controle de l'icone de la fenetre.
	
	cairo_dock_discard_task (myData.pTask);
	
	if (myData.iSidTimer != 0)
		g_source_remove (myData.iSidTimer);
	
	cairo_dock_fm_remove_monitor_full (CD_REBOOT_NEEDED_FILE, FALSE, NULL);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.

		// the icon can be changed.
		cd_logout_check_reboot_required_init ();
		
		cd_keybinder_rebind (myData.pKeyBinding, myConfig.cShortkey, D_(s_cShortkeyDescription[myConfig.iActionOnClick]));
		cd_keybinder_rebind (myData.pKeyBinding2, myConfig.cShortkey2, D_(s_cShortkeyDescription[myConfig.iActionOnMiddleClick]));
	}
CD_APPLET_RELOAD_END
