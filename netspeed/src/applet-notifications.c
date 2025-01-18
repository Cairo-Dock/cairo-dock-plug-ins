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
#include "applet-netspeed.h"


CD_APPLET_ON_CLICK_BEGIN
	gldi_dialogs_remove_on_icon (myIcon);
	if (myData.bAcquisitionOK)
	{
		gldi_dialog_show_temporary_with_icon_printf ("%s :\n  %s : %.2f%s\n  %s : %.2f%s",
			myIcon, myContainer, 6e3,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			D_("Total amount of data"),
			D_("downloaded"), (double) myData.iReceivedBytes / (1024*1024), D_("MB"),
			D_("uploaded"), (double) myData.iTransmittedBytes / (1024*1024), D_("MB"));
	}
	else
	{
		gchar *cQuestion;
		if (myConfig.iStringLen == 0)
			cQuestion = g_strdup (D_("No interface found.\n"
				"Please be sure that at least one interface is available\n"
				" and that you have the right to monitor it"));
		else
			cQuestion = g_strdup_printf (D_("Interface '%s' doesn't seem to exist or is not readable.\n"
				"You may have to set up the interface you wish to monitor.\n"
				"Do you want to do it now?"), myConfig.cInterface);
		gldi_dialog_show_with_question (cQuestion, myIcon, myContainer,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			(CairoDockActionOnAnswerFunc) cairo_dock_open_module_config_on_demand,
			myApplet, NULL);
		g_free (cQuestion);
	}
CD_APPLET_ON_CLICK_END

static void _nm_sleep (GldiModuleInstance *myApplet)
{
	DBusGProxy *pDbusProxy = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			"/org/freedesktop/NetworkManager",
			"org.freedesktop.DBus.Properties");
	g_return_if_fail (pDbusProxy != NULL);
	
	guint state = cairo_dock_dbus_get_property_as_uint (pDbusProxy,
		"org.freedesktop.NetworkManager",
		"State");
	g_object_unref (pDbusProxy);
	cd_debug ("current network state : %d", state);
	
	pDbusProxy = cairo_dock_create_new_system_proxy (
			"org.freedesktop.NetworkManager",
			"/org/freedesktop/NetworkManager",
			"org.freedesktop.NetworkManager");
	g_return_if_fail (pDbusProxy != NULL);
	dbus_g_proxy_call_no_reply (pDbusProxy, "Sleep",
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN, state == 3,  // 3 = actif
		G_TYPE_INVALID);
	g_object_unref (pDbusProxy);
}
static void _netspeed_sleep (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	_nm_sleep (myApplet);
}
static void _netspeed_recheck (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	gldi_task_stop (myData.pPeriodicTask);
	gldi_task_launch (myData.pPeriodicTask);
}
static void _show_system_monitor (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	if (myConfig.cSystemMonitorCommand != NULL)
	{
		cairo_dock_launch_command_full (myConfig.cSystemMonitorCommand, NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
	}
	else
	{
		cairo_dock_fm_show_system_monitor ();
	}
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Enable/disable network"), D_("middle-click"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GLDI_ICON_NAME_MEDIA_PAUSE, _netspeed_sleep, CD_APPLET_MY_MENU);
	g_free (cLabel);
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Open the System-Monitor"), GLDI_ICON_NAME_EXECUTE, _show_system_monitor, CD_APPLET_MY_MENU);
	
	if (! myData.bAcquisitionOK)
	{
		CD_APPLET_ADD_IN_MENU (D_("Re-check interface"), _netspeed_recheck, CD_APPLET_MY_MENU);
	}

CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	
	_nm_sleep (myApplet);
	
CD_APPLET_ON_MIDDLE_CLICK_END
