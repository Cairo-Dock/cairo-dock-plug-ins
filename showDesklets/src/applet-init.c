/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("showDesklets", 1, 6, 2, CAIRO_DOCK_CATEGORY_DESKTOP)


CD_APPLET_INIT_BEGIN
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	cairo_dock_register_notification (CAIRO_DOCK_WINDOW_ACTIVATED, (CairoDockNotificationFunc) cd_show_desklet_active_window_changed, CAIRO_DOCK_RUN_AFTER, myApplet);
	
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cShowImage);
	cd_keybinder_bind (myConfig.cShortcut, (CDBindkeyHandler) cd_show_desklet_on_keybinding_pull, (gpointer)NULL);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	cairo_dock_remove_notification_func (CAIRO_DOCK_WINDOW_ACTIVATED, (CairoDockNotificationFunc) cd_show_desklet_active_window_changed, myApplet);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	/**if (myData.bHide)
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cShowImage)
	else
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cHideImage)*/
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myData.bHide)
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cShowImage);
		}
		else
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cHideImage);
		}
		
		cd_keybinder_bind (myConfig.cShortcut, (CDBindkeyHandler) cd_show_desklet_on_keybinding_pull, (gpointer)NULL);
	}
CD_APPLET_RELOAD_END
