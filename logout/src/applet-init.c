
#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-init.h"


#define MY_APPLET_USER_DATA_DIR "logout"


CairoDockDesktopEnv my_logout_iDesktopEnv;


CD_APPLET_DEFINITION ("logout", 1, 4, 5)


CD_APPLET_INIT_BEGIN (erreur)
	//\_______________ Notre action depend du bureau.
	my_logout_iDesktopEnv = cairo_dock_guess_environment ();
	/*if (my_logout_iDesktopEnv == CAIRO_DOCK_UNKNOWN_ENV)
	{
		 g_set_error (erreur, 1, 1, "couldn't guess desktop environment, this module will not be active");
		return NULL;
	}*/
	
	//\_______________ On enregistre nos notifications.
	cairo_dock_register_first_notifications (CAIRO_DOCK_CLICK_ICON,
		(CairoDockNotificationFunc) CD_APPLET_ON_CLICK,
		CAIRO_DOCK_BUILD_MENU,
		(CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU,
		-1);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	cairo_dock_remove_notification_funcs (CAIRO_DOCK_CLICK_ICON,
		(CairoDockNotificationFunc) CD_APPLET_ON_CLICK,
		CAIRO_DOCK_BUILD_MENU,
		(CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU,
		-1);
CD_APPLET_STOP_END
