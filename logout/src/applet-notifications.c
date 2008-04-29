
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"


CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (_D("This is a very simple logout applet\n made by Fabrice Rey for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
{
	if (myConfig.cUserAction != NULL)
	{
		cairo_dock_launch_command (myConfig.cUserAction);
	}
	else
	{
		gboolean bLoggedOut = cairo_dock_fm_logout ();
		if (! bLoggedOut)
		{
			if (g_iDesktopEnv == CAIRO_DOCK_KDE)
			{
				int answer = cairo_dock_ask_question_and_wait ("Log out ?", myIcon, myContainer);
				if (answer == GTK_RESPONSE_YES)
				{
					system ("dcop ksmserver default logout 0 0 0");  // kdmctl shutdown reboot forcenow  // kdeinit_shutdown
				}
			}
			else
			{
				cd_warning ("couldn't guess what to do to log out.");
			}
		}
	}
}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
{
        CD_APPLET_ADD_SUB_MENU ("Logout", pSubMenu, CD_APPLET_MY_MENU);
        CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
}
CD_APPLET_ON_BUILD_MENU_END
