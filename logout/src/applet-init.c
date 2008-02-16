
#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"

AppletConfig myConfig;
AppletData myData;

CD_APPLET_DEFINITION ("logout", 1, 4, 7)


CD_APPLET_INIT_BEGIN (erreur)
{
	if (myDesklet != NULL)
	{
		myIcon->fWidth = MAX (1, myDesklet->iWidth - 2 * g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - 2 * g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius;
		myIcon->fDrawY = g_iDockRadius;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;
	}
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
}
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
{
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	reset_config ();
	reset_data ();
}
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
{
	if (myDesklet != NULL)
	{
		myIcon->fWidth = MAX (1, myDesklet->iWidth - 2 * g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - 2 * g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius;
		myIcon->fDrawY = g_iDockRadius;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;
	}
}
CD_APPLET_RELOAD_END
