#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-nvidia.h"
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS


void cd_nvidia_draw_no_data (void) {
	if (myData.pGPUData.iGPUTemp != myData.iPreviousGPUTemp) {
		myData.iPreviousGPUTemp = myData.pGPUData.iGPUTemp;
		CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cBrokenUserImage, "broken.svg");
	}
}

void cd_nvidia_draw_icon (void) {
	if (myData.pGPUData.iGPUTemp == myData.iPreviousGPUTemp) { // aucun changement.
		if (myData.pGauge)
			return ;
		else {
			double fTempPercent;
			if (myData.pGPUData.iGPUTemp <= myConfig.iLowerLimit) {
				fTempPercent = 0;
			}
			else if (myData.pGPUData.iGPUTemp >= myConfig.iUpperLimit ) {
				fTempPercent = 1;
			}
			else {
				fTempPercent = 1. * (myData.pGPUData.iGPUTemp - myConfig.iLowerLimit) / (myConfig.iUpperLimit - myConfig.iLowerLimit);
			}
			CD_APPLET_RENDER_GRAPH_NEW_VALUE (myData.pGraph, fTempPercent);
		}
	}
	
	if (myData.bAlerted == TRUE && myData.pGPUData.iGPUTemp < myConfig.iAlertLimit)
		myData.bAlerted = FALSE; //On réinitialise l'alert quand la température descend en dessou de la limite.
	
	if (myData.bAlerted == FALSE && myData.pGPUData.iGPUTemp >= myConfig.iAlertLimit)
		cd_nvidia_alert ();

	switch (myConfig.iDrawTemp) {
		case MY_APPLET_TEMP_NONE :
			if (myIcon->cQuickInfo != NULL)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		break;
		case MY_APPLET_TEMP_ON_QUICKINFO :
			//if (myDock)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d°C", myData.pGPUData.iGPUTemp);
			//else
				//CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("nVidia:%d°C", myData.pGPUData.iGPUTemp)
		break;
		case MY_APPLET_TEMP_ON_NAME :
			CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("%s: %d°C", myData.pGPUData.cGPUName, myData.pGPUData.iGPUTemp);
		break;
		case MY_APPLET_TEMP_ON_ICON : /*Abandonné pour le moment
			/// c'est quoi que tu veux dessiner ?
			//TODO Dessin manuel, copier sur clock le dessin de la Date avec libpango
			
			gchar *cTemp = g_strdup_printf ("%d°C", myData.pGPUData.iGPUTemp);
			g_free (cTemp);
			
			if (myIcon->cQuickInfo != NULL)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);*/
		break;
	}
	
	double fTempPercent;
	if (myData.pGPUData.iGPUTemp <= myConfig.iLowerLimit) {
		fTempPercent = 0;
	}
	else if (myData.pGPUData.iGPUTemp >= myConfig.iUpperLimit ) {
		fTempPercent = 1;
	}
	else {
		fTempPercent = 1. * (myData.pGPUData.iGPUTemp - myConfig.iLowerLimit) / (myConfig.iUpperLimit - myConfig.iLowerLimit);
	}
	cd_debug("nVidia - Value has changed, redraw (%f)", fTempPercent);
	
	if (myData.pGauge) {
		CD_APPLET_RENDER_GAUGE (myData.pGauge, fTempPercent);
	}
	else {
		CD_APPLET_RENDER_GRAPH_NEW_VALUE (myData.pGraph, fTempPercent);
	}
	
	myData.iPreviousGPUTemp = myData.pGPUData.iGPUTemp;
}

static void _nvidia_temporary_dialog (gchar *cInfo) {
	gchar *cIconPath = g_strdup_printf("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_ICON_FILE);
	cairo_dock_show_temporary_dialog_with_icon (cInfo, myIcon, myContainer, 8000, cIconPath);
	g_free(cIconPath);
}

void cd_nvidia_bubble (void) {
	if (myData.bAcquisitionOK) {
		gchar *cInfo = g_strdup_printf ("nVidia.\n %s %s\n %s %dMB \n %s %s\n %s %d°C", D_("GPU Name:"), myData.pGPUData.cGPUName, D_("Video Ram:"), myData.pGPUData.iVideoRam, D_("Driver Version:"), myData.pGPUData.cDriverVersion, D_("Core Temparature:"), myData.pGPUData.iGPUTemp);
		_nvidia_temporary_dialog (cInfo);
		g_free (cInfo);
	}
	else if (myData.bSettingsTooOld) {
		_nvidia_temporary_dialog (D_("nVidia \nYour version of 'nvidia-settings' is too old\n You need at least the 1.0.0 version"));
	}
	else {
		_nvidia_temporary_dialog (D_("nVidia \nCouldn't acquire GPU temperature\n is 'nvidia-settings' installed on your system\n and configured to monitor GPU temperature ?"));
	}
}

void cd_nvidia_alert (void) {
	if (myData.bAlerted || ! myConfig.bAlert)
		return;
	
	gchar *cInfo = g_strdup_printf ("nVidia %s\n %s %s\n %s %d°C", D_("Alert!"), myData.pGPUData.cGPUName, D_("passed alert temperature level"), D_("Core Temparature:"), myData.pGPUData.iGPUTemp);
	_nvidia_temporary_dialog (cInfo);
	g_free (cInfo);
	
	if (myConfig.bAlertSound)
		cairo_dock_play_sound (myConfig.cSoundPath);
	
	myData.bAlerted = TRUE;
}
