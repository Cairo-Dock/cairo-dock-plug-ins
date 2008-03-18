#include <stdlib.h>

#include "tomboy-draw.h"
#include "tomboy-config.h"
#include "tomboy-dbus.h"
#include "tomboy-notifications.h"
#include "tomboy-struct.h"
#include "tomboy-init.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_DEFINITION ("TomBoy", 1, 5, 3, CAIRO_DOCK_CATEGORY_DESKTOP)

CD_APPLET_INIT_BEGIN (erreur)
	myConfig.defaultTitle = g_strdup (myIcon->acName);
	
	load_all_surfaces();
	
	//Si le bus n'a pas encore ete acquis, on le recupere.
	if (!myData.dbus_enable) myData.dbus_enable = dbus_get_dbus();
	
	//Si le bus a ete acquis, on y connecte nos signaux.
	if (myData.dbus_enable)
	{
		dbus_connect_to_bus ();
		dbus_detect_tomboy();
		getAllNotes();
		update_icon();
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceBroken)
	}
	
	//Enregistrement des notifications
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	
	dbus_disconnect_from_bus ();
	
	reset_config ();
	reset_data ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	load_all_surfaces();
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		myConfig.defaultTitle = g_strdup (myIcon->acName);  // libere dans le reset_config() precedemment appele.
	}
	
	//\_______________ On redessine notre icone.
	if (myData.dbus_enable)
	{
		dbus_connect_to_bus ();
		dbus_detect_tomboy();
		getAllNotes();
		update_icon();
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceBroken)
	}
CD_APPLET_RELOAD_END
