#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-netspeed.h"


CD_APPLET_ON_CLICK_BEGIN
	cairo_dock_remove_dialog_if_any (myIcon);
	if (myData.bAcquisitionOK)
	{
		cairo_dock_show_temporary_dialog("%s :\n  %s : %.2f%s\n  %s : %.2f%s", myIcon, myContainer, 5e3, D_("Total amount of data"), D_("downloaded"), (double) myData.iReceivedBytes / (1024*1204), D_("MB"), D_("uploaded"), (double) myData.iTransmittedBytes / (1024*1204), D_("MB"));
	}
	else
	{
		cairo_dock_show_temporary_dialog(D_("Interface '%s' seems to not exist or is not readable.\n You may need to open the configuration panel of this applet, and choose the interface you wish to monitor."), myIcon, myContainer, 5e3, myConfig.cInterface);
	}
CD_APPLET_ON_CLICK_END


static void _netspeed_recheck (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet) {
	cairo_dock_stop_measure_timer (myData.pMeasureTimer);
	cairo_dock_launch_measure (myData.pMeasureTimer);
}
static void _show_monitor_system (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	if (myConfig.cSystemMonitorCommand != NULL)
	{
		cairo_dock_launch_command (myConfig.cSystemMonitorCommand);
	}
	else if (g_iDesktopEnv == CAIRO_DOCK_KDE)
	{
		system ("kde-system-monitor");
	}
	else
	{
		cairo_dock_fm_show_system_monitor ();
	}
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	CD_APPLET_ADD_IN_MENU (D_("Monitor System"), _show_monitor_system, pSubMenu);
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
