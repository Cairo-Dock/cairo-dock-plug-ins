/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-menu.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("GMenu", 2, 0, 0, CAIRO_DOCK_CATEGORY_DESKTOP)


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	
	if (myConfig.bShowRecent)
		myData.pRecentManager = gtk_recent_manager_get_default ();
	myData.pMenu = create_main_menu (myApplet);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	
	cd_keybinder_bind (myConfig.cMenuShortkey, (CDBindkeyHandler) cd_menu_on_shortkey_menu, NULL);
	cd_keybinder_bind (myConfig.cQuickLaunchShortkey, (CDBindkeyHandler) cd_menu_on_shortkey_quick_launch, NULL);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	if (myData.iSidFakeMenuIdle != 0)
		g_source_remove (myData.iSidFakeMenuIdle);
	if (myData.iSidCreateMenuIdle != 0)
		g_source_remove (myData.iSidCreateMenuIdle);
	if (myData.iSidTreeChangeIdle != 0)
		g_source_remove (myData.iSidTreeChangeIdle);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
		
		cd_keybinder_bind (myConfig.cMenuShortkey, (CDBindkeyHandler) cd_menu_on_shortkey_menu, NULL);  // shortkey were unbinded during reset_config.
		cd_keybinder_bind (myConfig.cQuickLaunchShortkey, (CDBindkeyHandler) cd_menu_on_shortkey_quick_launch, NULL);
		
		if (myConfig.bShowRecent && myData.pRecentManager == NULL)
			myData.pRecentManager = gtk_recent_manager_get_default ();
		
		if (! myConfig.bShowRecent && myData.pRecentMenuItem != NULL)
		{
			gtk_widget_destroy (myData.pRecentMenuItem);
			myData.pRecentMenuItem = NULL;
		}
		
		if (myData.pMenu != NULL &&
			(myConfig.bHasIcons != myData.bIconsLoaded) || (myConfig.bShowRecent && myData.pRecentMenuItem == NULL))
		{
			gtk_widget_destroy (myData.pMenu);
			myData.pMenu = NULL;
			myData.pRecentMenuItem = NULL;
		}
		
		if (myData.pMenu == NULL)
		{
			myData.pMenu = create_main_menu (myApplet);
		}
	}
CD_APPLET_RELOAD_END
