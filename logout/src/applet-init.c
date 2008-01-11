
#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-init.h"


CairoDockDesktopEnv my_logout_iDesktopEnv;


CD_APPLET_DEFINITION ("logout", 1, 4, 6)


CD_APPLET_INIT_BEGIN (erreur)
	//\_______________ Notre action depend du bureau.
	my_logout_iDesktopEnv = cairo_dock_guess_environment ();
	/*if (my_logout_iDesktopEnv == CAIRO_DOCK_UNKNOWN_ENV)
	{
		 g_set_error (erreur, 1, 1, "couldn't guess desktop environment, this module will not be active");
		return NULL;
	}*/
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_STOP_END
