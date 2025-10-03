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

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-menu.h"
#include "applet-recent.h"
#include "applet-init.h"
#include "cairo-dock-wayland-manager.h"


CD_APPLET_DEFINE2_BEGIN ("GMenu",
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_DESKTOP,
	N_("Displays the common Applications menu and the Recently used files.\n"
	"It is compatible with any XDG compliant menu (Gnome, XFCE, KDE, ...)\n"
	"Middle-click to open a dialog to quickly launch any command (you can set up a shortkey for it, like ALT+F2)\n"
	"You can also set up a shortkey to pop up the menu (like ALT+F1)"),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_REDEFINE_TITLE (N_("Applications Menu"))
CD_APPLET_DEFINE2_END

static gboolean _menu_request (G_GNUC_UNUSED gpointer ptr, G_GNUC_UNUSED GldiManager* pManager)
{
	gldi_container_present (CAIRO_CONTAINER (myDock)); // currently no-op
	gldi_wayland_grab_keyboard (CAIRO_CONTAINER (myDock)); // try to grab the keyboard
	cd_menu_show_menu ();
	return GLDI_NOTIFICATION_INTERCEPT;
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	
	myData.iPanelDefaultMenuIconSize = cairo_dock_search_icon_size (GTK_ICON_SIZE_LARGE_TOOLBAR);
	
	cd_menu_start ();
	
	myData.iShowQuit = myConfig.iShowQuit;
	myData.bLoadSettingsMenu = myConfig.bLoadSettingsMenu;

	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	
	// keyboard events
	myData.cKeyBinding = CD_APPLET_BIND_KEY (myConfig.cMenuShortkey,
		D_("Show/hide the Applications menu"),
		"Configuration", "menu shortkey",
		(CDBindkeyHandler) cd_menu_on_shortkey_menu);
	
	myData.cKeyBindingQuickLaunch = CD_APPLET_BIND_KEY (myConfig.cQuickLaunchShortkey,
		D_("Show/hide the quick-launch dialog"),
		"Configuration", "quick launch shortkey",
		(CDBindkeyHandler) cd_menu_on_shortkey_quick_launch);
	
	gldi_object_register_notification (&myDesktopMgr, NOTIFICATION_MENU_REQUEST, (GldiNotificationFunc)_menu_request , GLDI_RUN_AFTER, NULL);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	gldi_object_remove_notification (&myDesktopMgr, NOTIFICATION_MENU_REQUEST, (GldiNotificationFunc)_menu_request, NULL);

	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;

	// keyboard events
	gldi_object_unref (GLDI_OBJECT(myData.cKeyBinding));
	gldi_object_unref (GLDI_OBJECT(myData.cKeyBindingQuickLaunch));
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}

		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
		
		gldi_shortkey_rebind (myData.cKeyBinding, myConfig.cMenuShortkey, NULL);
		gldi_shortkey_rebind (myData.cKeyBindingQuickLaunch, myConfig.cQuickLaunchShortkey, NULL);
		
		// on reset ce qu'il faut.
		if (myData.pMenu != NULL
		&& (myConfig.iShowQuit != myData.iShowQuit
			|| myConfig.bLoadSettingsMenu != myData.bLoadSettingsMenu))
		{
			cd_menu_stop ();  // this is not very optimized but well...
		}
		myData.iShowQuit = myConfig.iShowQuit;
		myData.bLoadSettingsMenu = myConfig.bLoadSettingsMenu;
		
		// on reconstruit ce qu'il faut.
		if (myData.pMenu == NULL)
		{
			cd_menu_start ();
		}
		else  // menu deja existant, on rajoute/enleve les recents a la main.
		{
			if (! myConfig.bShowRecent)  // on ne veut plus des recent items.
			{
				if (myData.pRecentMenuItem != NULL)
				{
					gtk_widget_destroy (myData.pRecentMenuItem);
					myData.pRecentMenuItem = NULL;
				}
			}
			else  // on veut les recent items.
			{
				if (myData.pRecentMenuItem != NULL)  // ils existent deja.
				{
					if (myData.iNbRecentItems != myConfig.iNbRecentItems)  // rebuild the recent sub-menu
					{
						GtkWidget *pRecentMenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (myData.pRecentMenuItem));
						gtk_widget_destroy (pRecentMenu);
						cd_menu_append_recent_to_menu (myData.pMenu, myApplet);
					}
				}
				else  // il faut les construire.
				{
					cd_menu_append_recent_to_menu (myData.pMenu, myApplet);
				}
			}
		}
	}  // no need to do anything if the icons theme changes, because the gtk-images are loaded with a GIcon (idem for the Recent sub-menu)
CD_APPLET_RELOAD_END
