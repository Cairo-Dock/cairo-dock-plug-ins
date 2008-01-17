
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <glib/gi18n.h>

#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (_D("This is the CD_APPLET_NAME applet\n made by CD_MY_NAME for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("CD_APPLET_NAME", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
