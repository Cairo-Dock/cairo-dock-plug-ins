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
#include "applet-notifications.h"


CD_APPLET_ON_CLICK_BEGIN
	if (myData.bAcquisitionOK)
	{
		if (myData.pTopDialog != NULL)
			cd_sysmonitor_stop_top_dialog (myApplet);
		else
			cd_sysmonitor_start_top_dialog (myApplet);
	}
	else
	{
		if (myData.pTopDialog == NULL)
			cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_show_temporary_dialog_with_icon (D_("The acquisition of one or more data has failed.\nYou should remove the data that couldn't be fetched."), myIcon, myContainer, 6e3, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
CD_APPLET_ON_CLICK_END


#define _convert_from_kb(s) (int) (((s >> 20) == 0) ? (s >> 10) : (s >> 20))
#define _unit(s) (((s >> 20) == 0) ? D_("Mb") : D_("Gb"))
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myData.bInitialized && myData.bAcquisitionOK)
	{
		if (myData.pTopDialog != NULL || cairo_dock_remove_dialog_if_any (myIcon))
			CD_APPLET_LEAVE (CAIRO_DOCK_INTERCEPT_NOTIFICATION);
		
		// On recupere l'uptime.
		gchar *cUpTime = NULL, *cActivityTime = NULL, *cGCInfos = NULL;
		cd_sysmonitor_get_uptime (&cUpTime, &cActivityTime);
		// On recupere les donnees de la CG.
		if (myData.cGPUName == NULL)  // nvidia-config n'a encore jamais ete appele.
			cd_sysmonitor_get_nvidia_info (myApplet);
		if (myData.cGPUName && strcmp (myData.cGPUName, "none") != 0)  // nvidia-config est passe.
		{
			if (!myConfig.bShowNvidia)
				cd_sysmonitor_get_nvidia_data (myApplet);  // le thread ne passe pas par la => pas de conflit.
			cGCInfos = g_strdup_printf ("\n%s : %s\n %s : %d%s \n %s : %s\n %s : %d°C", D_("GPU model"), myData.cGPUName, D_("Video Ram"), myData.iVideoRam, D_("Mb"), D_("Driver Version"), myData.cDriverVersion, D_("Core Temperature"), myData.iGPUTemp);
		}
		// On recupere la RAM.
		if (!myConfig.bShowRam && ! myConfig.bShowSwap)
			cd_sysmonitor_get_ram_data (myApplet);  // le thread ne passe pas par la => pas de conflit.
		
		// On affiche tout ca.
		unsigned long long ram = myData.ramFree + myData.ramCached + myData.ramBuffers;
		cairo_dock_show_temporary_dialog_with_icon_printf ("%s : %s\n %s : %d MHz (%d %s)\n %s : %s / %s : %s\n%s : %d%s - %s : %d%s\n %s : %d%s - %s : %d%s%s",
			myIcon, myContainer, cGCInfos ? 15e3 : 12e3,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			D_("CPU model"), myData.cModelName,
			D_("Frequency"), myData.iFrequency,
			myData.iNbCPU, D_("core(s)"),
			D_("Uptime"), cUpTime,
			D_("Activity time"), cActivityTime,
			D_("Memory"), _convert_from_kb (myData.ramTotal), _unit (myData.ramTotal),
			D_("Available"), _convert_from_kb (ram), _unit (ram),
			D_("Cached"), _convert_from_kb (myData.ramCached), _unit (myData.ramCached),
			D_("Buffers"), _convert_from_kb (myData.ramBuffers), _unit (myData.ramBuffers),
			cGCInfos ? cGCInfos : "");
		g_free (cUpTime);
		g_free (cActivityTime);
		g_free (cGCInfos);
	}
	else
	{
		cairo_dock_show_temporary_dialog_with_icon (D_("The acquisition of one or more data has failed.\nYou should remove the data that couldn't be fetched."), myIcon, myContainer, 5e3, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


static void _show_monitor_system (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	if (myConfig.cSystemMonitorCommand != NULL)
	{
		cairo_dock_launch_command (myConfig.cSystemMonitorCommand);
	}
	else if (g_iDesktopEnv == CAIRO_DOCK_KDE)
	{
		int r = system ("kde-system-monitor &");
	}
	else
	{
		cairo_dock_fm_show_system_monitor ();
	}
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	
	CD_APPLET_ADD_IN_MENU (D_("System Monitor"), _show_monitor_system, CD_APPLET_MY_MENU);
	
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END
