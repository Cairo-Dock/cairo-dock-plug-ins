
#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("logout", 1, 4, 7)


CD_APPLET_INIT_BEGIN (erreur)
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_STOP_END
