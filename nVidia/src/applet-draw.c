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
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("N/A");
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cBrokenUserImage, "broken.svg");
	}
}

void cd_nvidia_draw_icon (void) {
	gboolean bNeedRedraw = FALSE;
	
	if (myConfig.bCardName)
		CD_APPLET_SET_NAME_FOR_MY_ICON (myData.pGPUData.cGPUName);
	
	if (myData.pGPUData.iGPUTemp != myData.iPreviousGPUTemp) {
		double fTempPercent;
		if (myData.pGPUData.iGPUTemp <= myConfig.iLowerLimit) {
			fTempPercent = 0;
		}
		else if (myData.pGPUData.iGPUTemp >= myConfig.iUpperLimit ) {
			fTempPercent = 1;
		}
		else {
			fTempPercent = (myConfig.iUpperLimit - myConfig.iLowerLimit) / (myData.pGPUData.iGPUTemp - myConfig.iLowerLimit);
			fTempPercent = 1 - (fTempPercent / 10);
		}
		cd_debug("nVidia - Value have changed, redraw (%f)", fTempPercent);
		make_cd_Gauge (myDrawContext, myContainer, myIcon, myData.pGauge, (double) fTempPercent);
		
		if (myData.bAlerted == TRUE && myData.pGPUData.iGPUTemp < myConfig.iAlertLimit)
			myData.bAlerted = FALSE; //On réinitialise l'alert quand la température descend en dessou de la limite.
		
		if (myData.bAlerted == FALSE && myData.pGPUData.iGPUTemp > myConfig.iAlertLimit)
			cd_nvidia_alert ();
		
		bNeedRedraw = TRUE;
	}
	
	switch (myConfig.iDrawTemp) {
		case MY_APPLET_TEMP_NONE :
			if (myIcon->cQuickInfo != NULL) {
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF(NULL);
				bNeedRedraw = TRUE;
			}
		break;
		case MY_APPLET_TEMP_ON_QUICKINFO :
			if (myData.pGPUData.iGPUTemp != myData.iPreviousGPUTemp) {
				gchar *cTemp = g_strdup_printf("%d°C", myData.pGPUData.iGPUTemp);
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (cTemp);
				g_free (cTemp);
				bNeedRedraw = TRUE;
			}
		break;
		case MY_APPLET_TEMP_ON_NAME :
			if (myData.pGPUData.iGPUTemp != myData.iPreviousGPUTemp) {
				gchar *cNameTemp = g_strdup_printf("%s: %d°C", myData.pGPUData.cGPUName, myData.pGPUData.iGPUTemp);
				CD_APPLET_SET_NAME_FOR_MY_ICON(cNameTemp);
				g_free (cNameTemp);
				
				if (myIcon->cQuickInfo != NULL)
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF(NULL);
				bNeedRedraw = TRUE;
			}
		break;
		case MY_APPLET_TEMP_ON_ICON : //Abandonner pour le moment
			if (myData.pGPUData.iGPUTemp != myData.iPreviousGPUTemp) {
				//Dessin manuel, copier sur clock
				gchar *cTemp = g_strdup_printf("%d°C", myData.pGPUData.iGPUTemp);
				g_free (cTemp);
				
				if (myIcon->cQuickInfo != NULL)
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF(NULL);
				bNeedRedraw = TRUE;
			}
		break;
	}
	
	if (bNeedRedraw) {
		myData.iPreviousGPUTemp = myData.pGPUData.iGPUTemp;
		CD_APPLET_REDRAW_MY_ICON
	}
}

static void _nvidia_temporary_dialog (GString *sInfo) {
	gchar *cIconPath = g_strdup_printf("%s/%s", MY_APPLET_SHARE_DATA_DIR, "icon.svg");
	cd_debug ("%s (%s)", sInfo->str, cIconPath);
	cairo_dock_show_temporary_dialog_with_icon (sInfo->str, myIcon, myContainer, 12000, cIconPath);
	g_free(cIconPath);
}

void cd_nvidia_bubble(void) {
	GString *sInfo = g_string_new ("");
	g_string_printf (sInfo, "nVidia.\n %s %s\n %s %dMB \n %s %s\n %s %d°C", D_("GPU Name:"), myData.pGPUData.cGPUName, D_("Video Ram:"), myData.pGPUData.iVideoRam, D_("Driver Version:"), myData.pGPUData.cDriverVersion, D_("Core Temparature:"), myData.pGPUData.iGPUTemp);
	_nvidia_temporary_dialog (sInfo);
	g_string_free (sInfo, TRUE);
}

void cd_nvidia_alert(void) {
	if (myData.bAlerted == TRUE || myConfig.bAlert == FALSE)
		return;
	
	GString *sInfo = g_string_new ("");
	g_string_printf (sInfo, "nVidia %s\n %s %s\n %s %d°C", D_("Alert!"), myData.pGPUData.cGPUName, D_("passed alert temperature level"), D_("Core Temparature:"), myData.pGPUData.iGPUTemp);
	_nvidia_temporary_dialog (sInfo);
	g_string_free (sInfo, TRUE);
	myData.bAlerted = TRUE;
}
