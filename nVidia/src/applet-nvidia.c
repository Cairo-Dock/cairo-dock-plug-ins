#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-nvidia.h"

CD_APPLET_INCLUDE_MY_VARS

#define CD_NVIDIA_CONFIG_TMP_FILE "/tmp/nvidia-config"


void cd_nvidia_acquisition (void) {
	GError *erreur = NULL;
	gchar *cCommand = g_strdup_printf("bash %s/nvidia", MY_APPLET_SHARE_DATA_DIR);
	g_spawn_command_line_async (cCommand, &erreur);
	g_free (cCommand);
}

void cd_nvidia_read_data (void) {
	const gchar *cGpuTemp = g_getenv ("CAIRO_DOCK_GPU_TEMP");
	if (cGpuTemp == NULL) {
		cd_warning("nVidia : couldn't acquire GPU temperature\n is 'nvidia-settings' installed on your system and its version >= 1.0 ?");
		myData.bAcquisitionOK = FALSE;
	}
	else {
		myData.bAcquisitionOK = TRUE;
		myData.pGPUData.iGPUTemp = atoi(cGpuTemp);
	}
}

void cd_nvidia_update_from_data (void) {
	if (myData.bAcquisitionOK) {
		cd_nvidia_draw_icon ();
		cairo_dock_set_normal_frequency_state (myData.pMeasureTimer);
	}
	else {
		cd_nvidia_draw_no_data ();
		cd_warning ("Couldn't get infos from nvidia setting (may not be installed or too old), halt.");
		cairo_dock_stop_measure_timer (myData.pMeasureTimer);  // pas la peine d'insister.
	}
}

void cd_nvidia_config_acquisition (void) {
	gchar *cCommand = g_strdup_printf("bash %s/nvidia-config", MY_APPLET_SHARE_DATA_DIR);
	system (cCommand);
	g_free (cCommand);
}

static gboolean _nvidia_get_values_from_file (gchar *cContent) {
	gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
	gchar *cOneInfopipe;
	gint flink=0, mlink=0, i=0,prcnt=0;
	
	g_free (myData.pGPUData.cGPUName);
	myData.pGPUData.cGPUName = NULL;
	g_free (myData.pGPUData.cDriverVersion);
	myData.pGPUData.cDriverVersion = NULL;
	
	for (i = 0; cInfopipesList[i] != NULL; i ++) {
		cOneInfopipe = cInfopipesList[i];
		if (*cOneInfopipe == '\0')
			continue;
		
		if ((i == 0) && (strcmp(cOneInfopipe,"nvidia") == 0)) {
			g_strfreev (cInfopipesList);
			return FALSE;
		}
		else {
			if (i == 0) {
				gchar *str = g_strstr_len (cOneInfopipe, strlen (cOneInfopipe), "version");
				g_print ("str : %s\n", str);
				if (str != NULL) {
					str += 7;
					while (*str == ' ')
						str ++;
					gchar *str2 = strchr (str, ' ');
					if (str2 != NULL)
						*str2 = '\0';
					int iMajorVersion=0, iMinorVersion=0, iMicroVersion=0;
					cairo_dock_get_version_from_string (str, &iMajorVersion, &iMinorVersion, &iMicroVersion);
					g_print ("%d.%d.%d\n", iMajorVersion, iMinorVersion, iMicroVersion);
					if (iMajorVersion == 0 || (iMajorVersion == 1 && iMinorVersion < 0)) { /// A confirmer ...
						myData.bSettingsTooOld == TRUE;
						cd_warning ("Attention : your nvidia-settings's version is too old (%d.%d.%d)", iMajorVersion, iMinorVersion, iMicroVersion);
						break ;
					}
				}
			}
			else if (i == 1) { //GPU Name
				myData.pGPUData.cGPUName = g_strdup (cOneInfopipe);
				gchar *str = strchr (myData.pGPUData.cGPUName, ')');
				if (str != NULL)
					*str = '\0';
			}
			else if (i == 2) { //Video Ram
				myData.pGPUData.iVideoRam = atoi(cOneInfopipe);
				myData.pGPUData.iVideoRam = myData.pGPUData.iVideoRam >> 10;  // passage en Mo.
			}
			else if (i == 3) { //Driver Version
				myData.pGPUData.cDriverVersion = g_strdup (cOneInfopipe);
			}
		}
	}
	
	cd_debug("nVidia %s %dMB %sV %d°C", myData.pGPUData.cGPUName, myData.pGPUData.iVideoRam, myData.pGPUData.cDriverVersion, myData.pGPUData.iGPUTemp);
	
	g_strfreev (cInfopipesList);
	return TRUE;
}
void cd_nvidia_config_read_data (void) {
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(CD_NVIDIA_CONFIG_TMP_FILE, &cContent, &length, &erreur);
	if (erreur != NULL) {
		cd_warning("Attention : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		myData.bAcquisitionOK = FALSE;
	}
	else {
		gboolean bAcquisitionOK = _nvidia_get_values_from_file (cContent);
		g_free (cContent);
	}
}

void cd_nvidia_config_update_from_data (void) {
	if (myConfig.bCardName) {
		CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pGPUData.cGPUName);
	}
}



