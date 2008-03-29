
#include "stdlib.h"
#include "string.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-load-icons.h"
#include "applet-draw.h"



AppletConfig myConfig;
AppletData myData;


CD_APPLET_DEFINITION ("switcher", 1, 4, 7, CAIRO_DOCK_CATEGORY_DESKTOP)

static void _load_surfaces (void)
{
	GString *sImagePath = g_string_new ("");
	
	if (myData.pSurface != NULL)
		cairo_surface_destroy (myData.pSurface);
	if (myConfig.cDefaultIcon != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cDefaultIcon);
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
cd_message ("ok default");
	}
	else
	{
		g_string_printf (sImagePath, "%s/default.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
cd_message ("ok default 2");
	}
	
	if (myData.pBrokenSurface != NULL)
		cairo_surface_destroy (myData.pBrokenSurface);
	if (myConfig.cBrokenIcon != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cBrokenIcon);
		myData.pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else
	{
		g_string_printf (sImagePath, "%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
cd_message ("ok broken");
	}
	
	g_string_free (sImagePath, TRUE);
}


CD_APPLET_INIT_BEGIN (erreur)
cairo_dock_register_notification (CAIRO_DOCK_DESKTOP_CHANGED, (CairoDockNotificationFunc) cd_switcher_launch_measure, CAIRO_DOCK_RUN_AFTER);
cairo_dock_register_notification (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED, (CairoDockNotificationFunc) cd_switcher_launch_measure, CAIRO_DOCK_RUN_AFTER);

_load_surfaces();


//cd_switcher_launch_measure ();
//cd_switcher_launch_measure ();
myData.loadaftercompiz = g_timeout_add (1000, (GSourceFunc) cd_switcher_launch_measure, (gpointer) NULL);

	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	
	g_source_remove (myData.loadaftercompiz);
	myData.loadaftercompiz = 0;
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	_load_surfaces();
	
	//cd_switcher_launch_measure ();
	
	myData.loadaftercompiz = g_timeout_add (1000, (GSourceFunc) cd_switcher_launch_measure, (gpointer) NULL);
CD_APPLET_RELOAD_END
