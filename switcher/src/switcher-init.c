
#include "stdlib.h"
#include <cairo-dock.h>

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-draw.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"

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

_load_surfaces();
//if (myConfig.bCurrentView)#include "applet-config.h"

//		{

		//CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pBrokenSurface)
	//
//CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurface)
	//	gboolean bTest = FALSE;
		//if (bTest)
		//{
			//myData.iCurrentVolume = 100;


			//switcher_draw_main_dock_icon (myData.pSurface);
cd_switcher_launch_measure ();
CD_APPLET_REDRAW_MY_ICON

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
	
	
	//\_________________ On libere toutes nos ressources.
	reset_config ();
	reset_data ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN

	_load_surfaces();
//if (myConfig.bCurrentView)
//		{

		//CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pBrokenSurface)
	//CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurface)
	//	gboolean bTest = FALSE;
		//if (bTest)
		//{
			//myData.iCurrentVolume = 100;


			//switcher_draw_main_dock_icon (myData.pSurface);
cd_switcher_launch_measure ();

CD_APPLET_RELOAD_END
