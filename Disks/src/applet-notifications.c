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
#include "applet-disks.h"


CD_APPLET_ON_CLICK_BEGIN
	gldi_dialogs_remove_on_icon (myIcon);
	//~ if (myData.bAcquisitionOK)
	//~ {
		//~ gldi_dialog_show_temporary_with_icon_printf ("%s :\n  %s : %.2f%s\n  %s : %.2f%s",
			//~ myIcon, myContainer, 6e3,
			//~ MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			//~ D_("Total amount of data"),
			//~ D_("downloaded"), (double) myData.iReceivedBytes / (1024*1024), D_("MB"),
			//~ D_("uploaded"), (double) myData.iTransmittedBytes / (1024*1024), D_("MB"));
	//~ }
	//~ else
	//~ {
		//~ gchar *cQuestion = g_strdup_printf (D_("Interface '%s' doesn't seem to exist or is not readable.\n You may have to set up the interface you wish to monitor.\n Do you want to do it now?"), myConfig.cInterface);
		//~ gldi_dialog_show_with_question (cQuestion, myIcon, myContainer, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE, (CairoDockActionOnAnswerFunc) cairo_dock_open_module_config_on_demand, myApplet, NULL);
		//~ g_free (cQuestion);
	//~ }
CD_APPLET_ON_CLICK_END

/* Not used
static void _disks_recheck (GtkMenuItem *menu_item, GldiModuleInstance *myApplet) {
	gldi_task_stop (myData.pPeriodicTask);
	gldi_task_launch (myData.pPeriodicTask);
}
*/
static void _show_monitor_system (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	if (myConfig.cSystemMonitorCommand != NULL)
	{
		cairo_dock_launch_command (myConfig.cSystemMonitorCommand);
	}
	else if (g_iDesktopEnv == CAIRO_DOCK_KDE)
	{
		int r = system ("kde-system-monitor &");
		if (r < 0)
			cd_warning ("Not able to launch this command: kde-system-monitor &");
	}
	else
	{
		cairo_dock_fm_show_system_monitor ();
	}
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("System Monitor"), GLDI_ICON_NAME_MEDIA_PLAY, _show_monitor_system, CD_APPLET_MY_MENU);
	
	// Sub-Menu
	//~ if (! myData.bAcquisitionOK) {
		//~ CD_APPLET_ADD_IN_MENU (D_("Re-check interface"), _disks_recheck, pSubMenu);
		//~ CD_APPLET_ADD_SEPARATOR_IN_MENU (pSubMenu);
	//~ }
CD_APPLET_ON_BUILD_MENU_END

