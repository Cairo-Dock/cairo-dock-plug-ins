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

#include "applet-struct.h"
#include "applet-cpusage.h"
#include "applet-rame.h"
#include "applet-nvidia.h"
#include "applet-top.h"
#include "applet-sensors.h"
#include "applet-notifications.h"


CD_APPLET_ON_CLICK_BEGIN
	if (myData.bAcquisitionOK)
	{
		cd_sysmonitor_start_top_dialog (myApplet);
	}
	else
	{
		gldi_dialogs_remove_on_icon (myIcon);
		gldi_dialog_show_temporary_with_icon (D_("The acquisition of one or more data has failed.\nYou should remove the data that couldn't be fetched."), myIcon, myContainer, 6e3, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
CD_APPLET_ON_CLICK_END


static void _pop_up_dialog_info (GldiModuleInstance *myApplet)
{
	if (myData.pTopDialog != NULL)  // we shouldn't get the click event anyway
		return;
	gldi_dialogs_remove_on_icon (myIcon);
	
	GString *pInfo = g_string_new ("");
	
	// CPU
	cd_sysmonitor_get_cpu_info (myApplet, pInfo);
	
	// uptime
	cd_sysmonitor_get_uptime_info (pInfo);
	
	// RAM
	cd_sysmonitor_get_ram_info (myApplet, pInfo);
	
	// CG
	cd_sysmonitor_get_nivdia_info (myApplet, pInfo);
	
	// sensors
	cd_sysmonitor_get_sensors_info (myApplet, pInfo);
	
	// On affiche tout ca.
	gldi_dialog_show_temporary_with_icon (pInfo->str,
		myIcon, myContainer,
		15e3,
		MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	
	g_string_free (pInfo, TRUE);
}

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myData.bInitialized && myData.bAcquisitionOK)
	{
		_pop_up_dialog_info (myApplet);
	}
	else
	{
		gldi_dialog_show_temporary_with_icon (D_("The acquisition of one or more data has failed.\nYou should remove the data that couldn't be fetched."), myIcon, myContainer, 5e3, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


static void _open_system_monitor (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	if (myConfig.cSystemMonitorCommand != NULL)
	{
		cairo_dock_launch_command (myConfig.cSystemMonitorCommand);
	}
	else
	{
		cairo_dock_fm_show_system_monitor ();
	}
}
static void _show_info (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	_pop_up_dialog_info (myApplet);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Open the System-Monitor"), GTK_STOCK_EXECUTE, _open_system_monitor, CD_APPLET_MY_MENU);
	
	gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Show info"), D_("middle-click"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GTK_STOCK_DIALOG_INFO, _show_info, CD_APPLET_MY_MENU);
	g_free (cLabel);
CD_APPLET_ON_BUILD_MENU_END
