#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-netspeed.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_ABOUT (D_("This is the netspeed applet\n made by parAdOxxx_ZeRo for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	cd_netspeed_launch_analyse();
CD_APPLET_ON_CLICK_END


static void _netspeed_recheck_ (GtkMenuItem *menu_item, gpointer *data) {
		if (myData.iSidTimer != 0) {
		  g_source_remove (myData.iSidTimer);
		  myData.iSidTimer = 0;
	  }
	  myConfig.iCheckInterval = myConfig.dCheckInterval;
	  cd_netspeed_launch_analyse();
}

CD_APPLET_ON_BUILD_MENU_BEGIN
		CD_APPLET_ADD_SUB_MENU ("netspeed", pSubMenu, CD_APPLET_MY_MENU)
		if (myData.interfaceFound == 0) {
	    		CD_APPLET_ADD_IN_MENU (D_("Waiting"), _netspeed_recheck_, pSubMenu)
	  	}
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
