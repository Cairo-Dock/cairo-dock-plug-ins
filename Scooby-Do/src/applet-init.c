/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-session.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("Scooby-Do",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_PLUG_IN,
	N_("A Gnome-Do-like plug-in.\n"
	"It lets you control the dock and quickly launch any command with the keyboard :\n"
	" Start a session with the shortkey of your choice, Escape, Return or shortkey to close it.\n"
	" use the arrows to navigate into the docks and sub-docks,\n"
	" or type a command to find the corresponding launcher, or to launch any command\n"
	" Press Tab to automatically jump to the next suitable launcher\n"
	" Press enter to click the icon or validate the command, Shift+Enter for Shift+click, Alt+Enter for middle click, and Ctrl+Enter for left click."),
	"Fabounet (Fabrice Rey)")


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	
	//cairo_dock_register_notification (CAIRO_DOCK_ENTER_DOCK, (CairoDockNotificationFunc) cd_do_enter_container, CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_DOCK, (CairoDockNotificationFunc) cd_do_update_container, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_RENDER_DOCK, (CairoDockNotificationFunc) cd_do_render, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_KEY_PRESSED, (CairoDockNotificationFunc) cd_do_key_pressed, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_STOP_ICON, (CairoDockNotificationFunc) cd_do_check_icon_stopped, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_WINDOW_ACTIVATED, (CairoDockNotificationFunc) cd_do_check_active_dock, CAIRO_DOCK_RUN_AFTER, NULL);
	
	cd_keybinder_bind (myConfig.cShortkey, (CDBindkeyHandler) cd_do_on_shortkey, myApplet);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	cairo_dock_remove_notification_func (CAIRO_DOCK_RENDER_DOCK, (CairoDockNotificationFunc) cd_do_render, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_UPDATE_DOCK, (CairoDockNotificationFunc) cd_do_update_container, NULL);
	//cairo_dock_remove_notification_func (CAIRO_DOCK_ENTER_DOCK, (CairoDockNotificationFunc) cd_do_enter_container, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_KEY_PRESSED, (CairoDockNotificationFunc) cd_do_key_pressed, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_STOP_ICON, (CairoDockNotificationFunc) cd_do_check_icon_stopped, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_WINDOW_ACTIVATED, (CairoDockNotificationFunc) cd_do_check_active_dock, NULL);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		cd_keybinder_bind (myConfig.cShortkey, (CDBindkeyHandler) cd_do_on_shortkey, myApplet);  // shortkey were unbinded during reset_config.
		if (myData.sCurrentText != NULL)
		{
			/// recharger surfaces / textures
			
			
			cairo_dock_redraw_container (g_pMainDock);
		}
	}
CD_APPLET_RELOAD_END
