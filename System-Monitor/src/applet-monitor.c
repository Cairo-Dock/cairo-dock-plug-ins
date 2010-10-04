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

#include <fcntl.h>
#include <unistd.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-cpusage.h"
#include "applet-rame.h"
#include "applet-nvidia.h"
#ifdef HAVE_SENSORS
#include "applet-sensors.h"
#endif
#include "applet-monitor.h"


void cd_sysmonitor_get_data (CairoDockModuleInstance *myApplet)
{
	myData.bNeedsUpdate = FALSE;
	
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
		if ((myData.iTimerCount % 3) == 0)  // la temperature ne varie pas tres vite et le script nvidia-settings est lourd, on decide donc de ne mettre a jour qu'une fois sur 3.
			cd_sysmonitor_get_nvidia_data (myApplet);
	}
#ifdef HAVE_SENSORS
	if (myConfig.bShowCpuTemp || myConfig.bShowFanSpeed)
	{
		cd_sysmonitor_get_sensors_data (myApplet);
	}
#endif
	
	if (! myData.bInitialized)
	{
		cd_sysmonitor_get_cpu_info (myApplet);  // on le fait ici plutot que lors de la demande d'info pour avoir le nombre de CPUs.
		myData.bInitialized = TRUE;
	}
	myData.iTimerCount ++;
}


gboolean cd_sysmonitor_update_from_data (CairoDockModuleInstance *myApplet)
{
	static double s_fValues[CD_SYSMONITOR_NB_MAX_VALUES];
	CD_APPLET_ENTER;
	
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
			
			if (/**myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON || */(myDock && myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL))  // on affiche les valeurs soit en info-rapide, soit sur l'etiquette en mode dock.
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
				if (myConfig.bShowCpuTemp)
				{
					g_string_append_printf (sInfo, "%s%d°C%s",
						(myDesklet ? "CPU:" : ""),
						myData.iCPUTemp,
						(bOneLine ? " - " : "\n"));
				}
				if (myConfig.bShowFanSpeed)
				{
					g_string_append_printf (sInfo, "%s%drpm%s",
						(myDesklet ? "FAN:" : ""),
						myData.iFanSpeed,
						(bOneLine ? " - " : "\n"));
				}
				sInfo->str[sInfo->len-(bOneLine?3:1)] = '\0';
				if (bOneLine)
					CD_APPLET_SET_NAME_FOR_MY_ICON (sInfo->str);
				else
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (sInfo->str);
				g_string_free (sInfo, TRUE);
			}
			
			if (myData.bNeedsUpdate || myConfig.iDisplayType == CD_SYSMONITOR_GRAPH)
			{
				int i = 0;
				if (myConfig.bShowCpu)
				{
					s_fValues[i++] = myData.fCpuPercent / 100.;
				}
				if (myConfig.bShowRam)
				{
					s_fValues[i++] = myData.fRamPercent / 100.;
				}
				if (myConfig.bShowSwap)
				{
					s_fValues[i++] = (myData.swapTotal ? (myConfig.bShowFreeMemory ? (double)myData.swapFree : (double)myData.swapUsed) / myData.swapTotal : 0.);
				}
				if (myConfig.bShowNvidia)
				{
					s_fValues[i++] = myData.fGpuTempPercent / 100.;
					if (myData.bAlerted && myData.iGPUTemp < myConfig.iAlertLimit)
						myData.bAlerted = FALSE; //On reinitialise l'alerte quand la temperature descend en dessous de la limite.
					
					if (!myData.bAlerted && myData.iGPUTemp >= myConfig.iAlertLimit)
						cd_nvidia_alert (myApplet);
				}
#ifdef HAVE_SENSORS
				if (myConfig.bShowCpuTemp)
				{
					s_fValues[i++] = myData.fCpuTempPercent / 100.;
					if (!myData.bCPUAlerted && myData.bCpuTempAlarm)
						cd_cpu_alert (myApplet);
				}
				if (myConfig.bShowFanSpeed)
				{
					s_fValues[i++] = myData.fFanSpeedPercent / 100.;
					if (!myData.bFanAlerted && myData.bFanAlarm)
						cd_fan_alert (myApplet);
				}
#endif
				CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (s_fValues);
			}
		}
	}
	
	CD_APPLET_LEAVE (myData.bAcquisitionOK);
}


void cd_sysmonitor_format_value (CairoDataRenderer *pRenderer, int iNumValue, gchar *cFormatBuffer, int iBufferLength, CairoDockModuleInstance *myApplet)
{
	double fValue = cairo_data_renderer_get_normalized_current_value_with_latency (pRenderer, iNumValue);
	int i = -1;
	if (myConfig.bShowCpu)
	{
		i ++;
		if (i == iNumValue)  // c'est celle-la qu'on veut afficher.
		{
			snprintf (cFormatBuffer, iBufferLength, fValue < .0995 ? "%.1f%%" : (fValue < 1 ? " %.0f%%" : "%.0f%%"), fValue * 100.);
			return ;
		}
	}
	if (myConfig.bShowRam)
	{
		i ++;
		if (i == iNumValue)
		{
			snprintf (cFormatBuffer, iBufferLength, fValue < .0995 ? "%.1f%%" : (fValue < 1 ? " %.0f%%" : "%.0f%%"), fValue * 100.);
			return ;
		}
	}
	if (myConfig.bShowSwap)
	{
		i ++;
		if (i == iNumValue)
		{
			snprintf (cFormatBuffer, iBufferLength, fValue < .0995 ? "%.1f%%" : (fValue < 1 ? " %.0f%%" : "%.0f%%"), fValue * 100.);
			return ;
		}
	}
	if (myConfig.bShowNvidia)
	{
		i ++;
		if (i == iNumValue)
		{
			double fTemp = myConfig.iLowerLimit + fValue * (myConfig.iUpperLimit - myConfig.iLowerLimit);
			snprintf (cFormatBuffer, iBufferLength, fTemp < 100. ? " %.0f�" : "%.0f�", fTemp);
			return ;
		}
	}
	if (myConfig.bShowCpuTemp)
	{
		i ++;
		if (i == iNumValue)
		{
			double fTemp = 0 + fValue * (100 - 0);
			snprintf (cFormatBuffer, iBufferLength, fTemp < 100. ? " %.0f�" : "%.0f�", fTemp);
			return ;
		}
	}
	if (myConfig.bShowFanSpeed)
	{
		i ++;
		if (i == iNumValue)
		{
			double fSpeed = fValue * myData.fMaxFanSpeed;
			snprintf (cFormatBuffer, iBufferLength, fSpeed < 100. ? " %.0f" : "%.0f", fSpeed);  // 'rpm' est trop long a ecrire.
			return ;
		}
	}
	snprintf (cFormatBuffer, iBufferLength, fValue < .0995 ? "%.1f" : (fValue < 1 ? " %.0f" : "%.0f"), fValue * 100.);  // on ne devrait jamais pouvoir arriver la.
}
