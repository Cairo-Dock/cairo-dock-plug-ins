/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-wifi.h"
#include "applet-draw.h"


CD_APPLET_ON_CLICK_BEGIN
	cairo_dock_remove_dialog_if_any (myIcon);
	cd_wifi_bubble();
CD_APPLET_ON_CLICK_END


static void _wifi_recheck_wireless_extension (GtkMenuItem *menu_item, gpointer data) {
	cairo_dock_stop_task (myData.pTask);
	cairo_dock_launch_task (myData.pTask);
}
static void _cd_wifi_show_config (GtkMenuItem *menu_item, gpointer data) {  /// a mettre dans les plug-ins d'integration.
	if (myConfig.cUserCommand != NULL) {
		cairo_dock_launch_command (myConfig.cUserCommand);
		return;
	}
	
	const gchar *cCommand = NULL;
	/**if (g_iDesktopEnv == CAIRO_DOCK_GNOME || g_iDesktopEnv == CAIRO_DOCK_XFCE) {
		int iMajor, iMinor, iMicro;
		cairo_dock_get_gnome_version (&iMajor, &iMinor, &iMicro);
		if (iMajor == 2 && iMinor < 22) {
			cCommand = "gksu network-admin";
		}
		else {
		  if (iMajor == 2 && iMinor > 22)
		    cCommand = "nm-connection-editor";
		  else {
			  cCommand = "network-admin";
			}
		}
	}
	else if (g_iDesktopEnv == CAIRO_DOCK_KDE) { //Ajouter les lignes de KDE
		//cCommand = 
	}*/
	cCommand = "nm-connection-editor";  // network-admin n'est plus present depuis Intrepid, et nm-connection-editor marche aussi sous KDE.
	
	cairo_dock_launch_command (cCommand);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	if (! myData.bWirelessExt)
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Check for Wireless Extension"), GTK_STOCK_REFRESH, _wifi_recheck_wireless_extension, CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Network Administration"), GTK_STOCK_PREFERENCES, _cd_wifi_show_config, CD_APPLET_MY_MENU);
CD_APPLET_ON_BUILD_MENU_END


static void toggle_wlan(void)
{
	DBusGProxy *dbus_proxy_nm = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			"/org/freedesktop/NetworkManager",
			"org.freedesktop.NetworkManager");
	g_return_if_fail (dbus_proxy_nm != NULL);
	
	guint state = 0;
	dbus_g_proxy_call (dbus_proxy_nm, "state", NULL,
		G_TYPE_INVALID,
		G_TYPE_UINT, &state,
		G_TYPE_INVALID);
	cd_debug ("current network state : %d", state);
	if (state == 3)  // actif
	{
		dbus_g_proxy_call_no_reply (dbus_proxy_nm, "sleep",
			G_TYPE_INVALID,
			G_TYPE_INVALID);
	}
	else if (state == 1)  // inactif
	{
		dbus_g_proxy_call_no_reply (dbus_proxy_nm, "wake",
			G_TYPE_INVALID,
			G_TYPE_INVALID);
	}
	
	g_object_unref (dbus_proxy_nm);
}
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	toggle_wlan ();
	//cairo_dock_launch_task (myData.pTask);
	//cairo_dock_remove_dialog_if_any (myIcon);
CD_APPLET_ON_MIDDLE_CLICK_END
