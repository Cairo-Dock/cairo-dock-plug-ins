#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-netspeed.h"

CD_APPLET_INCLUDE_MY_VARS

CD_APPLET_ABOUT (D_("This is the netspeed applet\n made by parAdOxxx_ZeRo for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	cairo_dock_remove_dialog_if_any (myIcon);
	if (myData.bAcquisitionOK)
	{
		cairo_dock_show_temporary_dialog("%s :\n  %s : %.2f%s\n  %s : %.2f%s", myIcon, myContainer, 5e3, D_("Total amount of data"), D_("downloaded"), (double) myData.iReceivedBytes / (1024*1204), D_("MB"), D_("uploaded"), (double) myData.iTransmittedBytes / (1024*1204), D_("MB"));
	}
	else
		cairo_dock_show_temporary_dialog(D_("Interface '%s' seems to not exist or is not readable"), myIcon, myContainer, 5e3, myConfig.cInterface);
CD_APPLET_ON_CLICK_END


static void _netspeed_recheck (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet) {
	cairo_dock_stop_measure_timer (myData.pMeasureTimer);
	cairo_dock_launch_measure (myData.pMeasureTimer);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("netspeed", pSubMenu, CD_APPLET_MY_MENU);
	if (! myData.bAcquisitionOK) {
		CD_APPLET_ADD_IN_MENU (D_("Re-check interface"), _netspeed_recheck, pSubMenu);
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	
	if (myData.dbus_proxy_nm == NULL)
		myData.dbus_proxy_nm = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			"/org/freedesktop/NetworkManager",
			"org.freedesktop.NetworkManager");
	g_return_val_if_fail (myData.dbus_proxy_nm != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
	
	guint state = 0;
	dbus_g_proxy_call (myData.dbus_proxy_nm, "state", NULL,
		G_TYPE_INVALID,
		G_TYPE_UINT, &state,
		G_TYPE_INVALID);
	cd_debug ("current network state : %d", state);
	if (state == 3)  // actif
	{
		dbus_g_proxy_call_no_reply (myData.dbus_proxy_nm, "sleep",
			G_TYPE_INVALID,
			G_TYPE_INVALID);
	}
	else if (state == 1)  // inactif
	{
		dbus_g_proxy_call_no_reply (myData.dbus_proxy_nm, "wake",
			G_TYPE_INVALID,
			G_TYPE_INVALID);
	}
	
CD_APPLET_ON_MIDDLE_CLICK_END
