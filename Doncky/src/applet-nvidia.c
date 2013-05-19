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
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>

#include "applet-struct.h"
#include "applet-nvidia.h"

void cd_sysmonitor_get_nvidia_data (GldiModuleInstance *myApplet)
{
	if (myConfig.bShowNvidia) // Une petite sécurité :-)
	{
		gchar *cCommand = g_strdup_printf ("nvidia-settings -q GPUCoreTemp -t");
		gchar *cResult = cairo_dock_launch_command_sync (cCommand);
		g_free (cCommand);
		
		gint iGpuTemp = 0;
		if (cResult != NULL)
			iGpuTemp = atoi (cResult);
		
		if (iGpuTemp == 0) {
			cd_warning("nVidia : couldn't acquire GPU temperature\n is 'nvidia-settings' installed on your system and its version >= 1.0 ?");
			myData.bAcquisitionOK = FALSE;
		}
		else {
			myData.iGPUTemp = iGpuTemp;
		}
				
		if (fabs (myData.fGpuTempPercent - myData.fPrevGpuTempPercent) > 1)
		{
			myData.fPrevGpuTempPercent = myData.fGpuTempPercent;
			myData.bNeedsUpdate = TRUE;
		}
	}
}


void cd_sysmonitor_get_nvidia_info (GldiModuleInstance *myApplet)
{
	if (myConfig.bShowNvidia) // Une petite sécurité :-)
	{
		gchar *cCommand = g_strdup_printf ("bash %s/nvidia-config", MY_APPLET_SHARE_DATA_DIR);
		gchar *cResult = cairo_dock_launch_command_sync (cCommand);
		g_free (cCommand);
		if (cResult == NULL || *cResult == '\n')
		{
			myData.cGPUName = g_strdup ("none");
			g_free (cResult);
			return ;
		}
		
		gchar **cInfopipesList = g_strsplit (cResult, "\n", -1);
		g_free (cResult);
		gchar *cOneInfopipe;
		gint i=0;
		
		g_free (myData.cGPUName);
		myData.cGPUName = NULL;
		g_free (myData.cDriverVersion);
		myData.cDriverVersion = NULL;
		
		for (i = 0; cInfopipesList[i] != NULL; i ++) {
			cOneInfopipe = cInfopipesList[i];
			if (*cOneInfopipe == '\0')
				continue;
			
			if ((i == 0) && (strcmp (cOneInfopipe,"nvidia") == 0)) {
				cd_warning ("problem while getting nVidia GPU temperature.");
				g_strfreev (cInfopipesList);
				return ;
			}
			else {
				if (i == 0) {
					gchar *str = g_strstr_len (cOneInfopipe, strlen (cOneInfopipe), "version");
					if (str != NULL) {
						str += 7;
						while (*str == ' ')
							str ++;
						gchar *str2 = strchr (str, ' ');
						if (str2 != NULL)
							*str2 = '\0';
						int iMajorVersion=0, iMinorVersion=0, iMicroVersion=0;
						cairo_dock_get_version_from_string (str, &iMajorVersion, &iMinorVersion, &iMicroVersion);
						/*if (iMajorVersion == 0 || (iMajorVersion == 1 && iMinorVersion < 0)) { /// A confirmer ...
							myData.bSettingsTooOld == TRUE;
							cd_warning ("Attention : your nvidia-settings's version is too old (%d.%d.%d)", iMajorVersion, iMinorVersion, iMicroVersion);
							break ;
						}*/
					}
				}
				else if (i == 1) { //GPU Name
					myData.cGPUName = g_strdup (cOneInfopipe);
					gchar *str = strchr (myData.cGPUName, ')');
					if (str != NULL)
						*str = '\0';
				}
				else if (i == 2) { //Video Ram
					myData.iVideoRam = atoi (cOneInfopipe);
					myData.iVideoRam = myData.iVideoRam >> 10;  // passage en Mo.
				}
				else if (i == 3) { //Driver Version
					myData.cDriverVersion = g_strdup (cOneInfopipe);
				}
			}
		}		
		g_strfreev (cInfopipesList);
	}
}
