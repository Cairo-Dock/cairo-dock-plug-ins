#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-rame.h"
#include "cairo-dock.h"

CD_APPLET_INCLUDE_MY_VARS

#define RAME_DATA_PIPE "/proc/meminfo"


#define goto_next_line \
	str = strchr (str, '\n'); \
	if (str == NULL) { \
		myData.bAcquisitionOK = FALSE; \
		return; \
	} \
	str ++;
#define get_value(iValue) \
	str = strchr (str, ':'); \
	if (str == NULL) { \
		myData.bAcquisitionOK = FALSE; \
		g_free (cContent); \
		return; \
	} \
	str ++; \
	while (*str == ' ') \
		str ++; \
	iValue = atoi (str);
void cd_rame_read_data (void)
{
	g_timer_stop (myData.pClock);
	double fTimeElapsed = g_timer_elapsed (myData.pClock, NULL);
	g_return_if_fail (fTimeElapsed > 0.1);
	g_timer_start (myData.pClock);
	
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents (RAME_DATA_PIPE, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning("Attention : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		myData.bAcquisitionOK = FALSE;
	}
	else
	{
		int iNumLine = 1;
		gchar *str = cContent;
		
		get_value (myData.ramTotal)  // MemTotal:
		
		goto_next_line
		get_value (myData.ramFree)  // MemFree:
		
		myData.ramUsed = myData.ramTotal - myData.ramFree;
		
		goto_next_line
		get_value (myData.ramBuffers)  // Buffers:
		
		goto_next_line
		get_value (myData.ramCached)  // Cached:
		
		goto_next_line  // SwapCached:
		goto_next_line  // Active:
		goto_next_line  // Inactive:
		goto_next_line  // HighTotal:
		goto_next_line  // HighFree:
		goto_next_line  // LowTotal:
		goto_next_line  // LowFree:
		
		goto_next_line
		get_value (myData.swapTotal)  // SwapTotal:
		
		goto_next_line
		get_value (myData.swapFree)  // SwapFree:
		
		myData.swapUsed = myData.swapTotal - myData.swapFree;
		
		g_free (cContent);
		myData.bAcquisitionOK = TRUE;
	}
}

void cd_rame_update_from_data (void)
{
	if ( ! myData.bAcquisitionOK)
	{
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle)
		else if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF("N/A");
		make_cd_Gauge(myDrawContext,myContainer,myIcon,myData.pGauge,(double) 0);
		
		cairo_dock_downgrade_frequency_state (myData.pMeasureTimer);
	}
	else
	{
		cairo_dock_set_normal_frequency_state (myData.pMeasureTimer);
		
		double fRamPercent = 100. * (myData.ramUsed - myData.ramCached) / myData.ramTotal;
		double fSwapPercent = 100. * myData.swapUsed / myData.swapTotal;
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_NONE)
		{
			GString *sInfo = g_string_new ("");
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL || myDesklet)
				g_string_printf (sInfo, "RAM:");
			
			g_string_append_printf (sInfo, (fRamPercent < 10 ? "%.1f%%" : "%.0f%%"), fRamPercent);
			if (myConfig.bShowSwap)
			{
				g_string_append_c (sInfo, '\n');
				if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
					g_string_append_printf (sInfo, "SWAP");
				g_string_append_printf (sInfo, (fSwapPercent < 10 ? "%.1f%%" : "%.0f%%"), fSwapPercent);
			}
			
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (sInfo->str)
			}
			else
			{
				CD_APPLET_SET_NAME_FOR_MY_ICON (sInfo->str)
			}
			g_string_free (sInfo, TRUE);
		}
		
		if (! myConfig.bShowSwap)
			make_cd_Gauge (myDrawContext, myContainer, myIcon, myData.pGauge, fRamPercent / 100);
		else
		{
			GList *pList = NULL;  /// un tableau ca serait plus sympa ...
			double *pValue = g_new (double, 1);
			*pValue = (double) fRamPercent / 100;
			pList = g_list_append (pList, pValue);
			pValue = g_new (double, 1);
			*pValue = (double) fSwapPercent / 100;
			pList = g_list_append (pList, pValue);
			make_cd_Gauge_multiValue (myDrawContext, myContainer, myIcon, myData.pGauge, pList);
			g_list_foreach (pList, (GFunc) g_free, NULL);
			g_list_free (pList);
		}
	}
}
