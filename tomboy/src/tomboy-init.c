#include <stdlib.h>

#include "tomboy-draw.h"
#include "tomboy-config.h"
#include "tomboy-dbus.h"
#include "tomboy-notifications.h"
#include "tomboy-struct.h"
#include "tomboy-init.h"



CD_APPLET_DEFINITION ("TomBoy", 1, 5, 4, CAIRO_DOCK_CATEGORY_CONTROLER)

CD_APPLET_INIT_BEGIN (erreur)
	myConfig.defaultTitle = g_strdup (myIcon->acName);
	
	load_all_surfaces();
	
	myData.dbus_enable = dbus_connect_to_bus ();
	if (myData.dbus_enable)
	{
		dbus_detect_tomboy();
		getAllNotes();
		update_icon();
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceBroken)
	}
	
	if (myConfig.bNoDeletedSignal)
		myData.iSidCheckNotes = g_timeout_add ((int) (2000), (GSourceFunc) cd_tomboy_check_deleted_notes, (gpointer) NULL);
	
	//Enregistrement des notifications
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	
	if (myData.iSidCheckNotes != 0)
	{
		g_source_remove (myData.iSidCheckNotes);
		myData.iSidCheckNotes = 0;
	}
	dbus_disconnect_from_bus ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	load_all_surfaces();
	
	//\_______________ On redessine notre icone.
	if (myData.dbus_enable)
	{
		getAllNotes();
		update_icon();
		
		if (myConfig.bNoDeletedSignal && myData.iSidCheckNotes == 0)
			myData.iSidCheckNotes = g_timeout_add ((int) (2000), (GSourceFunc) cd_tomboy_check_deleted_notes, (gpointer) NULL);
		else if (! myConfig.bNoDeletedSignal && myData.iSidCheckNotes != 0)
		{
			g_source_remove (myData.iSidCheckNotes);
			myData.iSidCheckNotes = 0;
		}
	}
	else  // sinon on signale par l'icone appropriee que le bus n'est pas accessible.
	{
		CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pSurfaceBroken)
	}
CD_APPLET_RELOAD_END
