#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-nvidia.h"

CD_APPLET_INCLUDE_MY_VARS

#define NVIDIA_TMP_FILE "/tmp/nvidia"

void cd_nvidia_acquisition (void) {
	gchar *cCommand = g_strdup_printf("bash %s/nvidia", MY_APPLET_SHARE_DATA_DIR);
	system (cCommand);
	g_free (cCommand);
}

static gboolean _nvidia_get_values_from_file (gchar *cContent) {
	gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
	gchar *cOneInfopipe;
	int flink=0, mlink=0, i=0,prcnt=0;
	for (i = 0; cInfopipesList[i] != NULL; i ++) {
		cOneInfopipe = cInfopipesList[i];
		if (*cOneInfopipe == '\0')
			continue;
		
		if ((i == 0) && (strcmp(cOneInfopipe,"nvidia") == 0)) {
			g_strfreev (cInfopipesList);
			return FALSE;
		}
		else {
			if (i == 0) { //GPU Name
				myData.pGPUData.cGPUName = g_strdup (cOneInfopipe);
				gchar *str = strchr (myData.pGPUData.cGPUName, ')');
				if (str != NULL)
					*str = '\0';
			}
			else if (i == 1) { //Video Ram
				myData.pGPUData.iVideoRam = atoi(cOneInfopipe);
				myData.pGPUData.iVideoRam = myData.pGPUData.iVideoRam / 1024;
			}
			else if (i == 2) { //Driver Version
				myData.pGPUData.cDriverVersion = g_strdup (cOneInfopipe);
			}
			else if (i == 3) { //GPU Temperature
				myData.pGPUData.iGPUTemp = atoi(cOneInfopipe);
			}
		}
	}
	
	cd_debug("nVidia %s %dMB %sV %d°C", myData.pGPUData.cGPUName, myData.pGPUData.iVideoRam, myData.pGPUData.cDriverVersion, myData.pGPUData.iGPUTemp);
	
	g_strfreev (cInfopipesList);  // on le libere a la fin car cESSID pointait dessus.
	return TRUE;
}

void cd_nvidia_read_data (void) {
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(NVIDIA_TMP_FILE, &cContent, &length, &erreur);
	if (erreur != NULL) {
		cd_warning("Attention : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		myData.bAcquisitionOK = FALSE;
	}
	else {
		myData.bAcquisitionOK = _nvidia_get_values_from_file (cContent);
		g_free (cContent);
	}
}


void cd_nvidia_update_from_data (void) {
	if (myData.bAcquisitionOK) {
		cd_nvidia_draw_icon ();
		cairo_dock_set_normal_frequency_state (myData.pMeasureTimer);
	}
	else {
		cd_nvidia_draw_no_data ();
		cd_warning ("Couldn't get infos from nvidia setting (may not be installed), halt.");
		cairo_dock_stop_measure_timer (myData.pMeasureTimer);
	}
}
