#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-nvidia.h"

void cd_sysmonitor_get_nvidia_data (CairoDockModuleInstance *myApplet)
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
}


void cd_sysmonitor_get_nvidia_info (CairoDockModuleInstance *myApplet)
{
	gchar *cCommand = g_strdup_printf ("bash %s/nvidia-config", MY_APPLET_SHARE_DATA_DIR);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	if (cResult == NULL || *cResult == '\n')  // les 'echo ""' du script rajoutent des retours chariots.
	{
		myData.cGPUName = g_strdup ("none");
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
	
	cd_debug ("nVidia %s %dMB %sV %d°C", myData.cGPUName, myData.iVideoRam, myData.cDriverVersion, myData.iGPUTemp);
	
	g_strfreev (cInfopipesList);
}


void cd_nvidia_alert (CairoDockModuleInstance *myApplet)
{
	if (myData.bAlerted || ! myConfig.bAlert)
		return;
	
	cairo_dock_show_temporary_dialog_with_icon (D_("Alert ! Graphic Card core temperature has reached %d°C"), myIcon, myContainer, 4e3, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE, myData.iGPUTemp);
	
	if (myConfig.bAlertSound)
		cairo_dock_play_sound (myConfig.cSoundPath);
	
	myData.bAlerted = TRUE;
}
