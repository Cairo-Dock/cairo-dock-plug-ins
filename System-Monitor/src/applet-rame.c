#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-rame.h"

#define RAME_DATA_PIPE CD_SYSMONITOR_PROC_FS"/meminfo"

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
	iValue = atoll (str);
void cd_sysmonitor_get_ram_data (CairoDockModuleInstance *myApplet)
{
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents (RAME_DATA_PIPE, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning("ram : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		myData.bAcquisitionOK = FALSE;
	}
	else
	{
		int iNumLine = 1;
		gchar *str = cContent;
		
		get_value (myData.ramTotal)  // MemTotal
		cd_debug ("ramTotal : %lld", myData.ramTotal);
		
		goto_next_line
		get_value (myData.ramFree)  // MemFree
		cd_debug ("ramFree : %lld", myData.ramFree);
		
		myData.ramUsed = myData.ramTotal - myData.ramFree;
		goto_next_line
		get_value (myData.ramBuffers)  // Buffers.
		
		goto_next_line
		get_value (myData.ramCached)  // Cached.
		cd_debug ("ramCached : %lld", myData.ramCached);
		
		goto_next_line  // SwapCached:
		goto_next_line  // Active:
		goto_next_line  // Inactive:
		goto_next_line  // HighTotal:
		goto_next_line  // HighFree:
		goto_next_line  // LowTotal:
		goto_next_line  // LowFree:
		
		goto_next_line
		get_value (myData.swapTotal)  // SwapTotal.
		cd_debug ("swapTotal : %lld", myData.swapTotal);
		
		goto_next_line
		get_value (myData.swapFree)  // SwapFree.
		cd_debug ("swapFree : %lld", myData.swapFree);
		
		myData.swapUsed = myData.swapTotal - myData.swapFree;
		
		g_free (cContent);
		myData.bAcquisitionOK = TRUE;
	}
}

gboolean cd_rame_update_from_data (CairoDockModuleInstance *myApplet)
{
	if ( ! myData.bAcquisitionOK)
	{
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		else if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
		if (myData.pGauge)
		{
			CD_APPLET_RENDER_GAUGE (myData.pGauge, 0.);
		}
		else
		{
			CD_APPLET_RENDER_GRAPH (myData.pGraph);
		}
	}
	else
	{
		if (! myData.bInitialized)
		{
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (myDock ? "..." : D_("Loading"));
			if (myData.pGauge)
			{
				CD_APPLET_RENDER_GAUGE (myData.pGauge, 0.);
			}
			else
			{
				CD_APPLET_RENDER_GRAPH (myData.pGraph);
			}
		}
		else
		{
			//double fRamPercent = 100. * (myData.ramUsed - myData.ramCached) / myData.ramTotal;
			//double fSwapPercent = 100. * myData.swapUsed / myData.swapTotal;
			double fRamPercent = 100. * (myConfig.bShowFreeMemory ? myData.ramFree + myData.ramCached : myData.ramUsed - myData.ramCached) / myData.ramTotal;
			double fSwapPercent = (myData.swapTotal ? 100. * (myConfig.bShowFreeMemory ? myData.swapFree : myData.swapUsed) / myData.swapTotal : 0.);
			cd_debug ("fRamPercent : %.2f %% ; fSwapPercent : %.2f %%", fRamPercent, fSwapPercent);
			gboolean bRamNeedsUpdate = (fabs (myData.fPrevRamPercent - fRamPercent) > .1);
			gboolean bSwapNeedsUpdate = (myConfig.bShowSwap && fabs (myData.fPrevSwapPercent - fSwapPercent) > .1);
			if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_NONE && (bRamNeedsUpdate || bSwapNeedsUpdate))
			{
				GString *sInfo = g_string_new ("");
				if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL || myDesklet)
					g_string_assign (sInfo, "RAM : ");
				
				g_string_append_printf (sInfo, (fRamPercent < 10 ? "%.1f%%" : "%.0f%%"), fRamPercent);
				if (myConfig.bShowSwap)
				{
					g_string_append_c (sInfo, '\n');
					if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL || myDesklet)
						g_string_append_printf (sInfo, "SWAP: ");
					g_string_append_printf (sInfo, (fSwapPercent < 10 ? "%.1f%%" : "%.0f%%"), fSwapPercent);
				}
				
				if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				{
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (sInfo->str);
				}
				else
				{
					CD_APPLET_SET_NAME_FOR_MY_ICON (sInfo->str);
				}
				g_string_free (sInfo, TRUE);
			}
			
			if (! myConfig.bShowSwap)
			{
				if (myData.pGauge && bRamNeedsUpdate)
				{
					CD_APPLET_RENDER_GAUGE (myData.pGauge, fRamPercent / 100);
				}
				else if (myData.pGraph)
				{
					CD_APPLET_RENDER_GRAPH_NEW_VALUE (myData.pGraph, fRamPercent / 100);
				}
			}
			else
			{
				if (myData.pGauge && (bRamNeedsUpdate || bSwapNeedsUpdate))
				{
					GList *pList = NULL;  /// un tableau ca serait plus sympa ...
					double *pValue = g_new (double, 1);
					*pValue = (double) fRamPercent / 100;
					pList = g_list_append (pList, pValue);
					pValue = g_new (double, 1);
					*pValue = (double) fSwapPercent / 100;
					pList = g_list_append (pList, pValue);
					CD_APPLET_RENDER_GAUGE_MULTI_VALUE (myData.pGauge, pList);
					g_list_foreach (pList, (GFunc) g_free, NULL);
					g_list_free (pList);
				}
				else if (myData.pGraph)
				{
					CD_APPLET_RENDER_GRAPH_NEW_VALUES (myData.pGraph, fRamPercent / 100, fSwapPercent / 100);
				}
			}
			
			if (bRamNeedsUpdate)
				myData.fPrevRamPercent = fRamPercent;
			if (bSwapNeedsUpdate)
				myData.fPrevSwapPercent = fSwapPercent;
		}
	}
	return myData.bAcquisitionOK;
}
