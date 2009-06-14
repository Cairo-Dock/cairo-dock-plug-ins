
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

#define CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON(pValues) cairo_dock_render_new_data_on_icon (myIcon, myContainer, myDrawContext, pValues)


void cd_sysmonitor_get_data (CairoDockModuleInstance *myApplet)
{
	if (myConfig.bShowCpu)
	{
		cd_sysmonitor_get_cpu_data (myApplet);
	}
	if (myConfig.bShowRam)
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
	if ( ! myData.bAcquisitionOK)
	{
		cd_warning ("One or more datas couldn't be retrieved");
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");  // plus discret qu'une bulle de dialogue.
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		double fValues[CD_SYSMONITOR_NB_MAX_VALUES];
		memset (fValues, 0, sizeof (fValues));
		CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (fValues);
	}
	else
	{
		if (! myData.bInitialized)
		{
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (myDock ? "..." : D_("Loading"));
			double fValues[CD_SYSMONITOR_NB_MAX_VALUES];
			memset (fValues, 0, sizeof (fValues));
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (fValues);
		}
		else
		{
			if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_NONE)
			{
				if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				{
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ((myDesklet ?
							(myData.cpu_usage < 10 ? "CPU:%.1f%%" : "CPU:%.0f%%") :
							(myData.cpu_usage < 10 ? "%.1f%%" : "%.0f%%")),
						myData.cpu_usage);
				}
				else
				{
					if (myDock)
						CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("CPU : %.1f%%", myData.cpu_usage);
				}
			}
			
			double fValues[CD_SYSMONITOR_NB_MAX_VALUES];
			int i = 0;
			if (myConfig.bShowCpu)
			{
				fValues[i++] = (double) myData.cpu_usage / 100;
			}
			if (myConfig.bShowRam)
			{
				fValues[i++] = (double) (myConfig.bShowFreeMemory ? myData.ramFree + myData.ramCached : myData.ramUsed - myData.ramCached) / myData.ramTotal;
			}
			if (myConfig.bShowSwap)
			{
				fValues[i++] = (double) (myData.swapTotal ? (myConfig.bShowFreeMemory ? myData.swapFree : myData.swapUsed) / myData.swapTotal : 0.);
			}
			if (myConfig.bShowNvidia)
			{
				double fTempPercent;
				if (myData.iGPUTemp <= myConfig.iLowerLimit)
					fTempPercent = 0;
				else if (myData.iGPUTemp >= myConfig.iUpperLimit )
					fTempPercent = 1.;
				else
					fTempPercent = (double) (myData.iGPUTemp - myConfig.iLowerLimit) / (myConfig.iUpperLimit - myConfig.iLowerLimit);
				fValues[i++] = fTempPercent;
				if (myData.bAlerted && myData.iGPUTemp < myConfig.iAlertLimit)
					myData.bAlerted = FALSE; //On réinitialise l'alerte quand la température descend en dessou de la limite.
				
				if (!myData.bAlerted && myData.iGPUTemp >= myConfig.iAlertLimit)
					cd_nvidia_alert (myApplet);
			}
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (fValues);
		}
	}
	return myData.bAcquisitionOK;
}
