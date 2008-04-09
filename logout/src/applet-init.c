
#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("logout", 1, 5, 4, CAIRO_DOCK_CATEGORY_DESKTOP)


CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet)
	{
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	if (myIcon->acFileName == NULL)
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE)
	}
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
	{
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	if (myIcon->acFileName == NULL)
	{
		CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE)
	}
CD_APPLET_RELOAD_END
