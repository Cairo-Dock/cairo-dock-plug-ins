/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey <fabounet@users.berlios.de>

******************************************************************************/
#define _BSD_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-nvidia.h"

CD_APPLET_INCLUDE_MY_VARS

static char  *s_cTmpFileConfig = NULL;
static char  *s_cTmpFile = NULL;


//Récupération de la température
void cd_nvidia_acquisition (void) {
	s_cTmpFile = g_strdup ("/tmp/nvidia.XXXXXX");
	int fds =mkstemp (s_cTmpFile);
	if (fds == -1)
	{
		g_free (s_cTmpFile);
		s_cTmpFile = NULL;
		return;
	}
	gchar *cCommand = g_strdup_printf ("bash %s/nvidia %s", MY_APPLET_SHARE_DATA_DIR, s_cTmpFile);
	system (cCommand);
	g_free (cCommand);
	close(fds);
}

void cd_nvidia_read_data (void) {
	if (s_cTmpFile == NULL)
		return ;
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	gint iGpuTemp;
	g_file_get_contents(s_cTmpFile, &cContent, &length, &erreur);
	if (erreur != NULL) {
		cd_warning("Attention : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		myData.bAcquisitionOK = FALSE;
	}
	else {
		iGpuTemp = atoi (cContent);
		if (iGpuTemp == 0) {
			cd_warning("nVidia : couldn't acquire GPU temperature\n is 'nvidia-settings' installed on your system and its version >= 1.0 ?");
			myData.bAcquisitionOK = FALSE;
		}
		else {
			myData.bAcquisitionOK = TRUE;
			myData.pGPUData.iGPUTemp = iGpuTemp;
		}
	}
	g_remove (s_cTmpFile);
	g_free (s_cTmpFile);
	s_cTmpFile = NULL;
}

gboolean cd_nvidia_update_from_data (void) {
	if (myData.bAcquisitionOK) {
		cd_nvidia_draw_icon ();
		return TRUE;
	}
	else {
		cd_nvidia_draw_no_data ();
		cd_warning ("Couldn't get infos from nvidia setting (may not be installed or too old), halt.");
		return FALSE;  // pas la peine d'insister.
	}
	
}



//Récupération de la config
void cd_nvidia_config_acquisition (void) {
	s_cTmpFileConfig = g_strdup ("/tmp/nvidia-config.XXXXXX");
	int fds =mkstemp (s_cTmpFileConfig);
	if (fds == -1)
	{
		g_free (s_cTmpFileConfig);
		s_cTmpFileConfig = NULL;
		return;
	}
	gchar *cCommand = g_strdup_printf ("bash %s/nvidia-config %s", MY_APPLET_SHARE_DATA_DIR, s_cTmpFileConfig);
	system (cCommand);
	g_free (cCommand);
	close(fds);
}

static gboolean _nvidia_get_values_from_file (gchar *cContent) {
	gchar **cInfopipesList = g_strsplit (cContent, "\n", -1);
	gchar *cOneInfopipe;
	gint i=0;
	
	g_free (myData.pGPUData.cGPUName);
	myData.pGPUData.cGPUName = NULL;
	g_free (myData.pGPUData.cDriverVersion);
	myData.pGPUData.cDriverVersion = NULL;
	
	for (i = 0; cInfopipesList[i] != NULL; i ++) {
		cOneInfopipe = cInfopipesList[i];
		if (*cOneInfopipe == '\0')
			continue;
		
		if ((i == 0) && (strcmp (cOneInfopipe,"nvidia") == 0)) {
			g_strfreev (cInfopipesList);
			return FALSE;
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
				myData.pGPUData.cGPUName = g_strdup (cOneInfopipe);
				gchar *str = strchr (myData.pGPUData.cGPUName, ')');
				if (str != NULL)
					*str = '\0';
			}
			else if (i == 2) { //Video Ram
				myData.pGPUData.iVideoRam = atoi (cOneInfopipe);
				myData.pGPUData.iVideoRam = myData.pGPUData.iVideoRam >> 10;  // passage en Mo.
			}
			else if (i == 3) { //Driver Version
				myData.pGPUData.cDriverVersion = g_strdup (cOneInfopipe);
			}
		}
	}
	
	cd_debug ("nVidia %s %dMB %sV %d°C", myData.pGPUData.cGPUName, myData.pGPUData.iVideoRam, myData.pGPUData.cDriverVersion, myData.pGPUData.iGPUTemp);
	
	g_strfreev (cInfopipesList);
	return TRUE;
}

void cd_nvidia_config_read_data (void) {
	if (s_cTmpFileConfig == NULL)
		return ;
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(s_cTmpFileConfig, &cContent, &length, &erreur);
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
	g_remove (s_cTmpFileConfig);
	g_free (s_cTmpFileConfig);
	s_cTmpFileConfig = NULL;
}

gboolean cd_nvidia_config_update_from_data (void) {
	if (myConfig.bCardName) {
		CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pGPUData.cGPUName);
	}
	return TRUE;
}
