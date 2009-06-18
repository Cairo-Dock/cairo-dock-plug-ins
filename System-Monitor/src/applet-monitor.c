
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-cpusage.h"
#include "applet-rame.h"
#include "applet-nvidia.h"
#include "applet-monitor.h"


void cd_sysmonitor_get_data (CairoDockModuleInstance *myApplet)
{
	if (myConfig.bShowCpu)
	{
		cd_sysmonitor_get_cpu_data (myApplet);
	}
	if (myConfig.bShowRam || myConfig.bShowSwap)
	{
		cd_sysmonitor_get_ram_data (myApplet);
	}
	if (myConfig.bShowNvidia)
	{
		cd_sysmonitor_get_nvidia_data (myApplet);
	}
	
	if (! myData.bInitialized)
	{
		cd_sysmonitor_get_cpu_info (myApplet);
		myData.bInitialized = TRUE;
	}
}


gboolean cd_sysmonitor_update_from_data (CairoDockModuleInstance *myApplet)
{
	static double s_fValues[CD_SYSMONITOR_NB_MAX_VALUES];
	
	if ( ! myData.bAcquisitionOK)
	{
		cd_warning ("One or more datas couldn't be retrieved");
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");  // plus discret qu'une bulle de dialogue.
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		memset (s_fValues, 0, sizeof (s_fValues));
		CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (s_fValues);
	}
	else
	{
		if (! myData.bInitialized)
		{
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (myDock ? "..." : D_("Loading"));
			memset (s_fValues, 0, sizeof (s_fValues));
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (s_fValues);
		}
		else
		{
			// Copier les donnes en memoire partagee...
			
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON || (myDock && myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL))  // on affiche les valeurs soit en info-rapide, soit sur l'etiquette en mode dock.
			{
				gboolean bOneLine = (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL);
				GString *sInfo = g_string_new ("");
				if (myConfig.bShowCpu)
				{
					g_string_printf (sInfo, (myData.fCpuPercent < 10 ? "%s%.1f%%%s" : "%s%.0f%%%s"),
						(myDesklet ? "CPU:" : ""),
						myData.fCpuPercent,
						(bOneLine ? " - " : "\n"));
				}
				if (myConfig.bShowRam)
				{
					g_string_append_printf (sInfo, (myData.fRamPercent < 10 ? "%s%.1f%%%s" : "%s%.0f%%%s"),
						(myDesklet ? "RAM:" : ""),
						myData.fRamPercent,
						(bOneLine ? " - " : "\n"));
				}
				if (myConfig.bShowSwap)
				{
					g_string_append_printf (sInfo, (myData.fSwapPercent < 10 ? "%s%.1f%%%s" : "%s%.0f%%%s"),
						(myDesklet ? "SWAP:" : ""),
						myData.fSwapPercent,
						(bOneLine ? " - " : "\n"));
				}
				if (myConfig.bShowNvidia)
				{
					g_string_append_printf (sInfo, "%s%d°C%s",
						(myDesklet ? "GPU:" : ""),
						myData.iGPUTemp,
						(bOneLine ? " - " : "\n"));
				}
				sInfo->str[sInfo->len-(bOneLine?3:1)] = '\0';
				if (bOneLine)
					CD_APPLET_SET_NAME_FOR_MY_ICON (sInfo->str);
				else
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (sInfo->str);
				g_string_free (sInfo, TRUE);
			}
			
			int i = 0;
			if (myConfig.bShowCpu)
			{
				s_fValues[i++] = (double) myData.fCpuPercent / 100;
			}
			if (myConfig.bShowRam)
			{
				s_fValues[i++] = myData.fRamPercent / 100;
			}
			if (myConfig.bShowSwap)
			{
				s_fValues[i++] = (double) (myData.swapTotal ? (myConfig.bShowFreeMemory ? myData.swapFree : myData.swapUsed) / myData.swapTotal : 0.);
			}
			if (myConfig.bShowNvidia)
			{
				s_fValues[i++] = myData.fGpuTempPercent;
				if (myData.bAlerted && myData.iGPUTemp < myConfig.iAlertLimit)
					myData.bAlerted = FALSE; //On réinitialise l'alerte quand la température descend en dessou de la limite.
				
				if (!myData.bAlerted && myData.iGPUTemp >= myConfig.iAlertLimit)
					cd_nvidia_alert (myApplet);
			}
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (s_fValues);
		}
	}
	return myData.bAcquisitionOK;
}
