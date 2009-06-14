#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>

#include "applet-struct.h"
#include "applet-top.h"


void cd_sysmonitor_free_process (CDProcess *pProcess)
{
	if (pProcess == NULL)
		return ;
	g_free (pProcess->cName);
	g_free (pProcess);
}

static gboolean _cd_clean_one_old_processes (int *iPid, CDProcess *pProcess, double *fTime)
{
	if (pProcess->fLastCheckTime < *fTime)
		return TRUE;
	return FALSE;
}
void cd_sysmonitor_clean_old_processes (CairoDockModuleInstance *myApplet, double fTime)
{
	g_hash_table_foreach_remove (myData.pProcessTable, (GHRFunc) _cd_clean_one_old_processes, &fTime);
}

void cd_sysmonitor_clean_all_processes (CairoDockModuleInstance *myApplet)
{
	g_hash_table_remove_all (myData.pProcessTable);
}


static inline void _cd_sysmonitor_insert_process_in_top_list (CairoDockModuleInstance *myApplet, CDProcess *pProcess)
{
	int i, j;
	if (myData.bSortTopByRam)
	{
		if (pProcess->iMemAmount > 0)
		{
			i = myConfig.iNbDisplayedProcesses - 1;
			while (i >= 0 && (myData.pTopList[i] == NULL || pProcess->iMemAmount > myData.pTopList[i]->iMemAmount))
				i --;
			if (i != myConfig.iNbDisplayedProcesses - 1)
			{
				i ++;
				for (j = myConfig.iNbDisplayedProcesses - 2; j >= i; j --)
					myData.pTopList[j+1] = myData.pTopList[j];
				myData.pTopList[i] = pProcess;
			}
		}
	}
	else
	{
		if (pProcess->fCpuPercent > 0)
		{
			i = myConfig.iNbDisplayedProcesses - 1;
			while (i >= 0 && (myData.pTopList[i] == NULL || pProcess->fCpuPercent > myData.pTopList[i]->fCpuPercent))
				i --;
			if (i != myConfig.iNbDisplayedProcesses - 1)
			{
				i ++;
				//g_print ("  fCpuPercent:%.2f%% => rang %d\n", 100*pProcess->fCpuPercent, i);
				for (j = myConfig.iNbDisplayedProcesses - 2; j >= i; j --)
					myData.pTopList[j+1] = myData.pTopList[j];
				myData.pTopList[i] = pProcess;
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

static void _cd_sysmonitor_get_process_data (CairoDockModuleInstance *myApplet, double fTime, double fTimeElapsed)
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
	
	if (myData.pProcessTable == NULL)
		myData.pProcessTable = g_hash_table_new_full (g_int_hash, g_int_equal, NULL, (GDestroyNotify) cd_sysmonitor_free_process);  // la cle est dans la valeur.
	if (myData.pTopList == NULL)
		myData.pTopList = g_new0 (CDProcess *, myConfig.iNbDisplayedProcesses);
	else
		memset (myData.pTopList, 0, myConfig.iNbDisplayedProcesses * sizeof (CDProcess *));
	if (myData.iMemPageSize == 0)
		myData.iMemPageSize = sysconf(_SC_PAGESIZE);
	
	const gchar *cPid;
	gchar *tmp;
	CDProcess *pProcess;
	int iNewCpuTime;
	unsigned long long iVmSize, iVmRSS, iTotalMemory;  // Quantité de mémoire totale utilisée / (Virtual Memory Resident Stack Size) Taille de la pile en mémoire.
	int i, j;
	while ((cPid = g_dir_read_name (dir)) != NULL)
	{
		if (! g_ascii_isdigit (*cPid))
			continue;
		
		snprintf (cFilePathBuffer, 20, "/proc/%s/stat", cPid);
		int pipe = open (cFilePathBuffer, O_RDONLY);
		int iPid = atoi (cPid);
		if (pipe <= 0)  // pas de pot le process s'est termine depuis qu'on a ouvert le repertoire.
		{
			g_hash_table_remove (myData.pProcessTable, &iPid);
			continue ;
		}
		
		if (read (pipe, cContent, sizeof (cContent)) <= 0)
		{
			cd_warning ("sysmonitor : can't read %s", cFilePathBuffer);
			close (pipe);
			continue;
		}
		close (pipe);
		
		pProcess = g_hash_table_lookup (myData.pProcessTable, &iPid);
		if (pProcess == NULL)
		{
			pProcess = g_new0 (CDProcess, 1);
			pProcess->iPid = iPid;
			g_hash_table_insert (myData.pProcessTable, &pProcess->iPid, pProcess);
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
		iVmSize = atoll (tmp);
		jump_to_next_value (tmp);
		iVmRSS = atoll (tmp);
		iTotalMemory = iVmRSS * myData.iMemPageSize;
		
		//g_print ("%s : %d -> %d\n", pProcess->cName, pProcess->iCpuTime, iNewCpuTime);
		if (pProcess->iCpuTime != 0 && fTimeElapsed != 0)
			pProcess->fCpuPercent = (iNewCpuTime - pProcess->iCpuTime) / myConfig.fUserHZ / myData.iNbCPU / fTimeElapsed;
		pProcess->iCpuTime = iNewCpuTime;
		pProcess->iMemAmount = iTotalMemory;
		
		_cd_sysmonitor_insert_process_in_top_list (myApplet, pProcess);
	}
	
	g_dir_close (dir);
}


static void _cd_sysmonitor_get_top_list (CairoDockModuleInstance *myApplet)
{
	// on recupere le delta T.
	g_timer_stop (myData.pTopClock);
	double fTimeElapsed = g_timer_elapsed (myData.pTopClock, NULL);
	g_timer_start (myData.pTopClock);
	GTimeVal time_val;
	g_get_current_time (&time_val);  // on pourrait aussi utiliser un compteur statique a la fonction ...
	double fTime = time_val.tv_sec + time_val.tv_usec * 1e-6;
	// on recupere les donnees de tous les processus.
	_cd_sysmonitor_get_process_data (myApplet, fTime, fTimeElapsed);
	// on nettoie la table des vieux processus.
	cd_sysmonitor_clean_old_processes (myApplet, fTime);
}

static gboolean _cd_sysmonitor_update_top_list (CairoDockModuleInstance *myApplet)
{
	// On ecrit les processus dans l'ordre.
	CDProcess *pProcess;
	int i, iNameLength=0;
	for (i = 0; i < myConfig.iNbDisplayedProcesses; i ++)
	{
		pProcess = myData.pTopList[i];
		if (pProcess == NULL || pProcess->cName == NULL)
			break;
		iNameLength = MAX (iNameLength, strlen (pProcess->cName));
	}
	
	gchar *cSpaces = g_new0 (gchar, iNameLength+1);
	memset (cSpaces, ' ', iNameLength);
	int iOffset;
	GString *sTopInfo = g_string_new ("");
	for (i = 0; i < myConfig.iNbDisplayedProcesses; i ++)
	{
		pProcess = myData.pTopList[i];
		if (pProcess == NULL || pProcess->cName == NULL)
			break;
		iOffset = iNameLength-strlen (pProcess->cName);
		if (pProcess->iPid < 1e5)
		{
			if (pProcess->iPid < 1e4)
			{
				if (pProcess->iPid < 1e3)
				{
					if (pProcess->iPid < 1e2)
					{
						if (pProcess->iPid < 1e1)
							iOffset += 5;
						else
							iOffset += 4;
					}
					else
						iOffset += 3;
				}
				else
					iOffset += 2;
			}
			else
				iOffset += 1;
		}
		cSpaces[iOffset] = '\0';
		g_string_append_printf (sTopInfo, "  %s (%d)%s: %.1f%%  %s-  %.1f%s\n",
			pProcess->cName,
			pProcess->iPid,
			cSpaces,
			100 * pProcess->fCpuPercent,
			(pProcess->fCpuPercent > .1 ? "" : " "),
			(double) pProcess->iMemAmount / (myConfig.bTopInPercent && myData.ramTotal ? 10.24 * myData.ramTotal : 1024 * 1024),
			(myConfig.bTopInPercent && myData.ramTotal ? "%" : D_("Mb")));
		cSpaces[iOffset] = ' ';
	}
	g_free (cSpaces);
	if (i == 0)  // liste vide.
	{
		g_string_free (sTopInfo, TRUE);
		return TRUE;
	}
	sTopInfo->str[sTopInfo->len-1] = '\0';
	
	// on affiche ca sur le dialogue.
	cairo_dock_render_dialog_with_new_data (myData.pTopDialog, (CairoDialogRendererDataPtr) sTopInfo->str);
	g_string_free (sTopInfo, TRUE);
	
	
	if (myData.iNbProcesses != g_hash_table_size (myData.pProcessTable))
	{
		myData.iNbProcesses = g_hash_table_size (myData.pProcessTable);
		gchar *cTitle = g_strdup_printf ("  [ Top %d / %d ] :", myConfig.iNbDisplayedProcesses, myData.iNbProcesses);
		cairo_dock_set_dialog_message (myData.pTopDialog, cTitle);
		g_free (cTitle);
	}
	return TRUE;
}



void cd_sysmonitor_stop_top_dialog (CairoDockModuleInstance *myApplet)
{
	if (myData.pTopDialog == NULL)
		return ;
	// on arrete la mesure.
	cairo_dock_stop_measure_timer (myData.pTopMeasureTimer);
	// on detruit le dialogue.
	cairo_dock_dialog_unreference (myData.pTopDialog);
	myData.pTopDialog = NULL;
	cairo_surface_destroy (myData.pTopSurface);
	myData.pTopSurface = NULL;
	g_timer_destroy (myData.pTopClock);
	myData.pTopClock = NULL;
	// on libere la liste des processus.
	cd_sysmonitor_clean_all_processes (myApplet);
}

void cd_sysmonitor_start_top_dialog (CairoDockModuleInstance *myApplet)
{
	g_return_if_fail (myData.pTopDialog == NULL);
	// on cree le dialogue.
	gchar *cTitle = g_strdup_printf ("  [ Top %d ] :", myConfig.iNbDisplayedProcesses);
	GtkWidget *pInteractiveWidget = gtk_vbox_new (FALSE, 0);
	gtk_widget_set_size_request (pInteractiveWidget,
		myConfig.pTopTextDescription->iSize * 15,
		myConfig.pTopTextDescription->iSize * myConfig.iNbDisplayedProcesses);  // approximatif au depart.
	myData.pTopDialog = cairo_dock_show_dialog_full (cTitle,
		myIcon,
		myContainer,
		0,
		MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
		pInteractiveWidget,
		NULL,
		NULL,
		NULL);
	g_free (cTitle);
	g_return_if_fail (myData.pTopDialog != NULL);
	
	gpointer pConfig[2] = {myConfig.pTopTextDescription, "Loading ..."};
	cairo_dock_set_dialog_renderer_by_name (myData.pTopDialog, "Text", myDrawContext, (CairoDialogRendererConfigPtr) pConfig);
	
	// on lance la mesure.
	myData.pTopClock = g_timer_new ();
	myData.iNbProcesses = 0;
	if (myData.pTopMeasureTimer == NULL)
		myData.pTopMeasureTimer = cairo_dock_new_measure_timer (myConfig.iProcessCheckInterval,
			NULL,
			(CairoDockReadTimerFunc) _cd_sysmonitor_get_top_list,
			(CairoDockUpdateTimerFunc) _cd_sysmonitor_update_top_list,
			myApplet);
	cairo_dock_launch_measure (myData.pTopMeasureTimer);
}
