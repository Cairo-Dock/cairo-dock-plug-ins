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
		
		myData.fPrevRamPercent = myData.fRamPercent;
		myData.fRamPercent = 100. * (myConfig.bShowFreeMemory ? myData.ramFree + myData.ramCached + myData.ramBuffers : myData.ramUsed - myData.ramCached - myData.ramBuffers) / myData.ramTotal;
		
		if (myConfig.bShowSwap)
		{
			goto_next_line  // SwapCached:
			goto_next_line  // Active:
			goto_next_line  // Inactive:
			goto_next_line  // HighTotal:
			goto_next_line  // HighFree:
			goto_next_line  // LowTotal:
			goto_next_line  // LowFree:
			
			while (strncmp (str, "SwapTotal", 9) != 0)
			{
				goto_next_line
			}
			get_value (myData.swapTotal)  // SwapTotal.
			cd_debug ("swapTotal : %lld", myData.swapTotal);
			goto_next_line
			get_value (myData.swapFree)  // SwapFree.
			cd_debug ("swapFree : %lld", myData.swapFree);
			
			myData.swapUsed = myData.swapTotal - myData.swapFree;
			
			myData.fPrevSwapPercent = myData.fSwapPercent;
			myData.fSwapPercent = 100. * myData.swapUsed / myData.swapTotal;  // pas de cache/buffers ?...
		}
		
		g_free (cContent);
	}
}
