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
#include <math.h>

#include <fcntl.h>
#include <unistd.h>

#include "applet-struct.h"
#include "applet-cpusage.h"  // cd_sysmonitor_get_cpu_info
#include "applet-top.h"


static void _cd_sysmonitor_free_process (CDProcess *pProcess)
{
	if (pProcess == NULL)
		return ;
	g_free (pProcess->cName);
	g_free (pProcess);
}


static inline void _cd_sysmonitor_insert_process_in_top_list (CDTopSharedMemory *pSharedMemory, CDProcess *pProcess)
{
	int i, j;
	if (pSharedMemory->bSortTopByRam)
	{
		if (pProcess->iMemAmount > 0)
		{
			i = pSharedMemory->iNbDisplayedProcesses - 1;
			while (i >= 0 && (pSharedMemory->pTopList[i] == NULL || pProcess->iMemAmount > pSharedMemory->pTopList[i]->iMemAmount))
				i --;
			if (i != pSharedMemory->iNbDisplayedProcesses - 1)
			{
				i ++;
				for (j = pSharedMemory->iNbDisplayedProcesses - 2; j >= i; j --)
					pSharedMemory->pTopList[j+1] = pSharedMemory->pTopList[j];
				pSharedMemory->pTopList[i] = pProcess;
			}
		}
	}
	else
	{
		if (pProcess->fCpuPercent > 0)
		{
			i = pSharedMemory->iNbDisplayedProcesses - 1;
			while (i >= 0 && (pSharedMemory->pTopList[i] == NULL || pProcess->fCpuPercent > pSharedMemory->pTopList[i]->fCpuPercent))
				i --;
			if (i != pSharedMemory->iNbDisplayedProcesses - 1)
			{
				i ++;
				//g_print ("  fCpuPercent:%.2f%% => rang %d\n", 100*pProcess->fCpuPercent, i);
				for (j = pSharedMemory->iNbDisplayedProcesses - 2; j >= i; j --)
					pSharedMemory->pTopList[j+1] = pSharedMemory->pTopList[j];
				pSharedMemory->pTopList[i] = pProcess;
			}
		}
	}
}

#define jump_to_next_value(tmp) \
	while (*tmp != ' ' && *tmp != '\0') \
		tmp ++; \
	if (*tmp == '\0') { \
		cd_warning ("system monitor : problem when reading pipe"); \
		break ; \
	} \
	while (*tmp == ' ') \
		tmp ++; \

static void _cd_sysmonitor_get_process_data (CDTopSharedMemory *pSharedMemory, double fTime, double fTimeElapsed)
{
	static gchar cFilePathBuffer[20+1];  // /proc/12345/stat + 4octets de marge.
	static gchar cContent[512+1];
	
	cd_debug ("%s (%.2f)", __func__, fTimeElapsed);
	GError *erreur = NULL;
	GDir *dir = g_dir_open (CD_SYSMONITOR_PROC_FS, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("sysmonitor : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	if (pSharedMemory->pProcessTable == NULL)
		pSharedMemory->pProcessTable = g_hash_table_new_full (g_int_hash, g_int_equal, NULL, (GDestroyNotify) _cd_sysmonitor_free_process);  // a table of (pid*, process*), the pid* points directly into the process.
	if (pSharedMemory->pTopList == NULL)
		pSharedMemory->pTopList = g_new0 (CDProcess *, pSharedMemory->iNbDisplayedProcesses);  // list of the processes to be displayed (points directly into the table).
	else
		memset (pSharedMemory->pTopList, 0, pSharedMemory->iNbDisplayedProcesses * sizeof (CDProcess *));
	if (pSharedMemory->iMemPageSize == 0)
		pSharedMemory->iMemPageSize = sysconf(_SC_PAGESIZE);
	
	const gchar *cPid;
	gchar *tmp;
	CDProcess *pProcess;
	int iNewCpuTime;
	unsigned long long iVmRSS, iTotalMemory;  // Quantite de memoire totale utilisee / (Virtual Memory Resident Stack Size) Taille de la pile en memoire.
	while ((cPid = g_dir_read_name (dir)) != NULL)
	{
		if (! g_ascii_isdigit (*cPid))
			continue;
		
		snprintf (cFilePathBuffer, 20, "/proc/%s/stat", cPid);
		int pipe = open (cFilePathBuffer, O_RDONLY);
		int iPid = atoi (cPid);
		if (pipe <= 0)  // pas de pot le process s'est termine depuis qu'on a ouvert le repertoire.
		{
			g_hash_table_remove (pSharedMemory->pProcessTable, &iPid);
			continue ;
		}
		
		if (read (pipe, cContent, sizeof (cContent)) <= 0)
		{
			cd_warning ("sysmonitor : can't read %s", cFilePathBuffer);
			close (pipe);
			continue;
		}
		close (pipe);
		
		pProcess = g_hash_table_lookup (pSharedMemory->pProcessTable, &iPid);
		if (pProcess == NULL)
		{
			pProcess = g_new0 (CDProcess, 1);
			pProcess->iPid = iPid;
			g_hash_table_insert (pSharedMemory->pProcessTable, &pProcess->iPid, pProcess);
		}
		pProcess->fLastCheckTime = fTime;
		
		tmp = cContent;
		jump_to_next_value (tmp);  // on saute le pid.
		if (pProcess->cName == NULL)
		{
			tmp ++;  // on saute la '('.
			gchar *str = tmp;
			while (*str != ')' && *str != '\0')
				str ++;
			pProcess->cName = g_strndup (tmp, str - tmp);
		}
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
		iNewCpuTime = atoll (tmp);  // user.
		jump_to_next_value (tmp);
		iNewCpuTime += atoll (tmp);  // system.
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);  // on saute le nice.
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		// iVmSize = atoll (tmp);
		jump_to_next_value (tmp);
		iVmRSS = atoll (tmp);
		iTotalMemory = iVmRSS * pSharedMemory->iMemPageSize;
		
		//g_print ("%s : %d -> %d\n", pProcess->cName, pProcess->iCpuTime, iNewCpuTime);
		if (pProcess->iCpuTime != 0 && fTimeElapsed != 0)
			pProcess->fCpuPercent = (iNewCpuTime - pProcess->iCpuTime) / pSharedMemory->fUserHZ / pSharedMemory->iNbCPU / fTimeElapsed;
		pProcess->iCpuTime = iNewCpuTime;
		pProcess->iMemAmount = iTotalMemory;
		
		_cd_sysmonitor_insert_process_in_top_list (pSharedMemory, pProcess);
	}
	
	g_dir_close (dir);
}

static gboolean _clean_one_old_processes (int *iPid, CDProcess *pProcess, double *fTime)
{
	if (pProcess->fLastCheckTime < *fTime)
		return TRUE;
	return FALSE;
}

static void _cd_sysmonitor_get_top_list (CDTopSharedMemory *pSharedMemory)
{
	// get the elapsed time since the last 'top'.
	double fTimeElapsed;
	if (pSharedMemory->pTopClock == NULL)
	{
		pSharedMemory->pTopClock = g_timer_new ();
		fTimeElapsed = 0.;
	}
	else
	{
		g_timer_stop (pSharedMemory->pTopClock);
		fTimeElapsed = g_timer_elapsed (pSharedMemory->pTopClock, NULL);
		g_timer_start (pSharedMemory->pTopClock);
	}
	
	// get the current time.
	GTimeVal time_val;
	g_get_current_time (&time_val);  // on pourrait aussi utiliser un compteur statique a la fonction ...
	double fTime = time_val.tv_sec + time_val.tv_usec * 1e-6;
	
	// get the data for all processes.
	_cd_sysmonitor_get_process_data (pSharedMemory, fTime, fTimeElapsed);
	
	// clean the table from old processes.
	g_hash_table_foreach_remove (pSharedMemory->pProcessTable, (GHRFunc) _clean_one_old_processes, &fTime);
}


static gboolean _cd_sysmonitor_update_top_list (CDTopSharedMemory *pSharedMemory)
{
	GldiModuleInstance *myApplet = pSharedMemory->pApplet;
	CD_APPLET_ENTER;
	
	// determine the max length of process names.
	CDProcess *pProcess;
	int i;
	guint iNameLength = 0;
	for (i = 0; i < pSharedMemory->iNbDisplayedProcesses; i ++)
	{
		pProcess = pSharedMemory->pTopList[i];
		if (pProcess == NULL || pProcess->cName == NULL)
			break;
		iNameLength = MAX (iNameLength, strlen (pProcess->cName));
	}
	
	// write the processes in the form "name (pid)    : 15.2% - 12.3Mb".
	gchar *cSpaces = g_new0 (gchar, iNameLength+6+1);  // name + pid(<1e6) + '\0'
	memset (cSpaces, ' ', iNameLength);
	int iNbSpaces;
	GString *sTopInfo = g_string_new ("");
	for (i = 0; i < pSharedMemory->iNbDisplayedProcesses; i ++)
	{
		pProcess = pSharedMemory->pTopList[i];
		if (pProcess == NULL || pProcess->cName == NULL)
			break;
		// determine the number of spaces needed to have a correct alignment.
		iNbSpaces = iNameLength - strlen (pProcess->cName);
		if (pProcess->iPid < 1e5)  // no PID is >= 1e6; if ever it was, there would just be a small offset of the cpu and ram values displayed.
		{
			if (pProcess->iPid < 1e4)
			{
				if (pProcess->iPid < 1e3)
				{
					if (pProcess->iPid < 1e2)
					{
						if (pProcess->iPid < 1e1)
							iNbSpaces += 5;
						else
							iNbSpaces += 4;
					}
					else
						iNbSpaces += 3;
				}
				else
					iNbSpaces += 2;
			}
			else
				iNbSpaces += 1;
		}
		cSpaces[iNbSpaces] = '\0';
		g_string_append_printf (sTopInfo, "  %s (%d)%s: %.1f%%  %s-  %.1f%s\n",
			pProcess->cName,
			pProcess->iPid,
			cSpaces,
			100 * pProcess->fCpuPercent,
			(pProcess->fCpuPercent > .1 ? "" : " "),
			(double) pProcess->iMemAmount / (myConfig.bTopInPercent && myData.ramTotal ? 10.24 * myData.ramTotal : 1024 * 1024),
			(myConfig.bTopInPercent && myData.ramTotal ? "%" : D_("Mb")));
		cSpaces[iNbSpaces] = ' ';
	}
	g_free (cSpaces);
	
	// display the info on the dialog.
	if (sTopInfo->len == 0)  // empty list, let the default message ("loading").
	{
		g_string_free (sTopInfo, TRUE);
		CD_APPLET_LEAVE (TRUE);
	}
	else  // remove the trailing \n.
	{
		sTopInfo->str[sTopInfo->len-1] = '\0';
	}
	
	cairo_dock_render_dialog_with_new_data (myData.pTopDialog, (CairoDialogRendererDataPtr) sTopInfo->str);
	g_string_free (sTopInfo, TRUE);
	
	// update the dialog title with the total number of processes if it has changed.
	if (myData.iNbProcesses != g_hash_table_size (pSharedMemory->pProcessTable))
	{
		myData.iNbProcesses = g_hash_table_size (pSharedMemory->pProcessTable);
		gchar *cTitle = g_strdup_printf ("  [ Top %d / %d ] :", pSharedMemory->iNbDisplayedProcesses, myData.iNbProcesses);
		gldi_dialog_set_message (myData.pTopDialog, cTitle);
		g_free (cTitle);
	}
	
	// update the sort for the next step.
	pSharedMemory->bSortTopByRam = myData.bSortTopByRam;
	
	CD_APPLET_LEAVE (TRUE);
}


static void _free_shared_memory (CDTopSharedMemory *pSharedMemory)
{
	g_hash_table_destroy (pSharedMemory->pProcessTable);
	g_free (pSharedMemory->pTopList);
	g_timer_destroy (pSharedMemory->pTopClock);
	g_free (pSharedMemory);
}
static void cd_sysmonitor_launch_top_task (GldiModuleInstance *myApplet)
{
	g_return_if_fail (myData.pTopTask == NULL);
	
	myData.iNbProcesses = 0;
	if (myData.iNbCPU == 0)
		cd_sysmonitor_get_cpu_info (myApplet, NULL);
	
	CDTopSharedMemory *pSharedMemory = g_new0 (CDTopSharedMemory, 1);
	pSharedMemory->iNbDisplayedProcesses = myConfig.iNbDisplayedProcesses;
	pSharedMemory->fUserHZ = myConfig.fUserHZ;
	pSharedMemory->iNbCPU = myData.iNbCPU;
	pSharedMemory->pApplet = myApplet;
	
	myData.pTopTask = cairo_dock_new_task_full (myConfig.iProcessCheckInterval,
		(CairoDockGetDataAsyncFunc) _cd_sysmonitor_get_top_list,
		(CairoDockUpdateSyncFunc) _cd_sysmonitor_update_top_list,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	cairo_dock_launch_task (myData.pTopTask);
}

static void _sort_one_process (int *iPid, CDProcess *pProcess, CDTopSharedMemory *pSharedMemory)
{
	_cd_sysmonitor_insert_process_in_top_list (pSharedMemory, pProcess);
}
static void _on_change_order (int iClickedButton, GtkWidget *pInteractiveWidget, GldiModuleInstance *myApplet, CairoDialog *pDialog)
{
	if (iClickedButton == 2 || iClickedButton == -2)  // 'close' button or Escape, just return and let the dialog be destroyed.
	{
		return;
	}
	gboolean bSortByRamNew = (iClickedButton == 1);
	if (bSortByRamNew != myData.bSortTopByRam)  // we'll sort the result immediately, so that the user doesn't have to wait until the next measure to see the result.
	{
		myData.bSortTopByRam = bSortByRamNew;
		
		cairo_dock_stop_task (myData.pTopTask);  // blocks until the thread terminates.
		
		CDTopSharedMemory *pSharedMemory = myData.pTopTask->pSharedMemory;  // this is ok only because we stopped the task beforehand.
		pSharedMemory->bSortTopByRam = bSortByRamNew;
		if (pSharedMemory->pTopList != NULL && pSharedMemory->iNbDisplayedProcesses != 0)
		{
			memset (pSharedMemory->pTopList, 0, pSharedMemory->iNbDisplayedProcesses * sizeof (CDProcess *));  // on re-trie tout suivant le nouvel ordre.
			g_hash_table_foreach (pSharedMemory->pProcessTable, (GHFunc) _sort_one_process, pSharedMemory);
			_cd_sysmonitor_update_top_list (pSharedMemory);  // on redessine.
		}
		
		cairo_dock_launch_task_delayed (myData.pTopTask, 1000. * myConfig.iProcessCheckInterval);  // restart the task with a delay equal to the interval, to keep the measure accurate.
	}
	gldi_object_ref (GLDI_OBJECT (pDialog));  // keep the dialog alive.
}
static void _on_dialog_destroyed (GldiModuleInstance *myApplet)
{
	// discard the 'top' task.
	cairo_dock_discard_task (myData.pTopTask);
	myData.pTopTask = NULL;
	
	// no more dialog.
	myData.pTopDialog = NULL;
}
void cd_sysmonitor_start_top_dialog (GldiModuleInstance *myApplet)
{
	g_return_if_fail (myData.pTopDialog == NULL);
	gldi_dialogs_remove_on_icon (myIcon);
	// build an interactive widget that will be used to display the top list.
	gchar *cTitle = g_strdup_printf ("  [ Top %d ] :", myConfig.iNbDisplayedProcesses);
	GtkWidget *pInteractiveWidget = _gtk_vbox_new (0);
	gtk_widget_set_size_request (pInteractiveWidget,
		myConfig.pTopTextDescription->iSize * 15,
		myConfig.pTopTextDescription->iSize * myConfig.iNbDisplayedProcesses);  // approximatif au depart.
	
	// build the dialog.
	CairoDialogAttr attr;
	memset (&attr, 0, sizeof (CairoDialogAttr));
	attr.cText = cTitle;
	attr.cImageFilePath = MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE;
	attr.pInteractiveWidget = pInteractiveWidget;
	attr.pActionFunc = (CairoDockActionOnAnswerFunc) _on_change_order;
	attr.pUserData = myApplet;
	attr.pFreeDataFunc = (GFreeFunc) _on_dialog_destroyed;
	const gchar *cButtons[] = {MY_APPLET_SHARE_DATA_DIR"/button-cpu.svg", MY_APPLET_SHARE_DATA_DIR"/button-ram.svg", "cancel", NULL};
	attr.cButtonsImage = cButtons;
	attr.pIcon = myIcon;
	attr.pContainer = myContainer;
	myData.pTopDialog = gldi_dialog_new (&attr);
	
	g_free (cTitle);
	g_return_if_fail (myData.pTopDialog != NULL);
	
	// set a dialog renderer of type 'text'.
	const gpointer pConfig[2] = {myConfig.pTopTextDescription, (const gpointer)D_("Loading")};
	cairo_dock_set_dialog_renderer_by_name (myData.pTopDialog, "Text", (CairoDialogRendererConfigPtr) pConfig);
	
	// launch the 'top' task.
	cd_sysmonitor_launch_top_task (myApplet);
}
