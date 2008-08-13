#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>

#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-rame.h"
#include "cairo-dock.h"

CD_APPLET_INCLUDE_MY_VARS

#define RAME_DATA_PIPE "/proc/meminfo"
#define CD_RAME_PROC_FS "/proc"

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
void cd_rame_read_data (CairoDockModuleInstance *myApplet)
{
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
		if (! myData.bInitialized)
			myData.bInitialized = TRUE;
	}
}

gboolean cd_rame_update_from_data (CairoDockModuleInstance *myApplet)
{
	if ( ! myData.bAcquisitionOK)
	{
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle)
		else if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF("N/A");
		cairo_dock_render_gauge(myDrawContext,myContainer,myIcon,myData.pGauge,(double) 0);
	}
	else
	{
		if (! myData.bInitialized)
		{
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (myDock ? "..." : D_("Loading"));
			cairo_dock_render_gauge (myDrawContext, myContainer, myIcon, myData.pGauge, 0.);
		}
		else
		{
			double fRamPercent = 100. * (myData.ramUsed - myData.ramCached) / myData.ramTotal;
			double fSwapPercent = 100. * myData.swapUsed / myData.swapTotal;
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
					if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
						g_string_append_printf (sInfo, "SWAP: ");
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
			{
				if (bRamNeedsUpdate)
					cairo_dock_render_gauge (myDrawContext, myContainer, myIcon, myData.pGauge, fRamPercent / 100);
			}
			else
			{
				if (bRamNeedsUpdate || bSwapNeedsUpdate)
				{
					GList *pList = NULL;  /// un tableau ca serait plus sympa ...
					double *pValue = g_new (double, 1);
					*pValue = (double) fRamPercent / 100;
					pList = g_list_append (pList, pValue);
					pValue = g_new (double, 1);
					*pValue = (double) fSwapPercent / 100;
					pList = g_list_append (pList, pValue);
					cairo_dock_render_gauge_multi_value (myDrawContext, myContainer, myIcon, myData.pGauge, pList);
					g_list_foreach (pList, (GFunc) g_free, NULL);
					g_list_free (pList);
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



#define jump_to_next_value(tmp) \
	while (*tmp != ' ' && *tmp != '\0') \
		tmp ++; \
	if (*tmp == '\0') { \
		cd_warning ("problem when reading pipe"); \
		break ; \
	} \
	while (*tmp == ' ') \
		tmp ++; \

void cd_rame_free_process (CDProcess *pProcess)
{
	if (pProcess == NULL)
		return ;
	g_free (pProcess->cName);
	g_free (pProcess);
}


void cd_rame_get_process_memory (void)
{
	static gchar cFilePathBuffer[20+1];  // /proc/12345/stat
	static gchar cContent[512+1];
	
	cd_debug ("");
	GError *erreur = NULL;
	GDir *dir = g_dir_open (CD_RAME_PROC_FS, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	if (myData.pTopList == NULL)
	{
		myData.pTopList = g_new0 (CDProcess *, myConfig.iNbDisplayedProcesses);
		myData.iNbDisplayedProcesses = myConfig.iNbDisplayedProcesses;
	}
	if (myData.pPreviousTopList == NULL)
		myData.pPreviousTopList = g_new0 (CDProcess *, myConfig.iNbDisplayedProcesses);
	if (myData.iMemPageSize == 0)
		myData.iMemPageSize = sysconf(_SC_PAGESIZE);
	
	int i;
	for (i = 0; i < myConfig.iNbDisplayedProcesses; i ++)
		cd_rame_free_process (myData.pPreviousTopList[i]);
	memcpy (myData.pPreviousTopList, myData.pTopList, myConfig.iNbDisplayedProcesses * sizeof (CDProcess *));
	memset (myData.pTopList, 0, myConfig.iNbDisplayedProcesses * sizeof (CDProcess *));
	
	const gchar *cPid;
	gchar *tmp, *cName;
	CDProcess *pProcess;
	int iVmSize, iVmRSS, iTotalMemory;  // Quantité de mémoire totale utilisée / (Virtual Memory Resident Stack Size) Taille de la pile en mémoire.
	int j;
	while ((cPid = g_dir_read_name (dir)) != NULL)
	{
		if (! g_ascii_isdigit (*cPid))
			continue;
		
		snprintf (cFilePathBuffer, 20, "/proc/%s/stat", cPid);
		int pipe = open (cFilePathBuffer, O_RDONLY);
		int iPid = atoi (cPid);
		if (pipe <= 0)  // pas de pot le process s'est termine depuis qu'on a ouvert le repertoire.
		{
			continue ;
		}
		
		if (read (pipe, cContent, sizeof (cContent)) <= 0)
		{
			cd_warning ("can't read %s", cFilePathBuffer);
			close (pipe);
			continue;
		}
		close (pipe);
		
		pProcess = g_new0 (CDProcess, 1);
		pProcess->iPid = iPid;
		
		tmp = cContent;
		jump_to_next_value (tmp);  // on saute le pid.
			
		cName = tmp + 1;  // on saute la '('.
		gchar *str = cName;
		while (*str != ')' && *str != '\0')
			str ++;
		
		jump_to_next_value (tmp);  // on saute le nom.
		jump_to_next_value (tmp);  // on saute l'etat.
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);  // on saute le temps user.
		jump_to_next_value (tmp);  // on saute le temps system.
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);  // on saute le nice.
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		iVmSize = atoi (tmp);
		jump_to_next_value (tmp);
		iVmRSS = atoi (tmp);
		
		iTotalMemory = iVmRSS * myData.iMemPageSize;
		if (iTotalMemory > 0)
		{
			i = myConfig.iNbDisplayedProcesses - 1;
			while (i >= 0 && (myData.pTopList[i] == NULL || iTotalMemory > myData.pTopList[i]->iMemAmount))
				i --;
			if (i != myConfig.iNbDisplayedProcesses - 1)
			{
				i ++;
				for (j = myConfig.iNbDisplayedProcesses - 2; j >= i; j --)
					myData.pTopList[j+1] = myData.pTopList[j];
				
				pProcess = g_new0 (CDProcess, 1);
				pProcess->iPid = iPid;
				pProcess->cName = g_strndup (cName, str - cName);
				pProcess->iMemAmount = iTotalMemory;
				myData.pTopList[i] = pProcess;
			}
		}
	}
	
	g_dir_close (dir);
}

void cd_rame_clean_all_processes (void)
{
	int i;
	for (i = 0; i < myData.iNbDisplayedProcesses; i ++)
	{
		cd_rame_free_process (myData.pTopList[i]);
		cd_rame_free_process (myData.pPreviousTopList[i]);
	}
	memset (myData.pTopList, 0, myData.iNbDisplayedProcesses * sizeof (CDProcess *));
	memset (myData.pPreviousTopList, 0, myData.iNbDisplayedProcesses * sizeof (CDProcess *));
}
