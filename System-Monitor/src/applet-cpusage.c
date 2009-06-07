
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>

#include <fcntl.h>
#include <unistd.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-cpusage.h"
#include "cairo-dock.h"

#define CD_CPUSAGE_PROC_FS "/proc"
#define CPUSAGE_DATA_PIPE "/proc/stat"
#define CPUSAGE_UPTIME_PIPE "/proc/uptime"
#define CPUSAGE_LOADAVG_PIPE "/proc/loadavg"
#define CPUSAGE_PROC_INFO_PIPE "/proc/cpuinfo"


void cd_cpusage_get_uptime (gchar **cUpTime, gchar **cActivityTime)
{
	FILE *fd = fopen (CPUSAGE_UPTIME_PIPE, "r");
	if (fd == NULL)
	{
		cd_warning ("can't open %s", CPUSAGE_UPTIME_PIPE);
		return ;
	}
	
	double fUpTime = 0, fIdleTime = 0;
	fscanf (fd, "%lf %lf\n", &fUpTime, &fIdleTime);
	fclose (fd);
	
	const int minute = 60;
	const int hour = minute * 60;
	const int day = hour * 24;
	int iUpTime = (int) fUpTime, iActivityTime = (int) (fUpTime - fIdleTime);
	*cUpTime = g_strdup_printf ("%ld %s, %ld:%02ld:%02ld",
		iUpTime / day, D_("day(s)"),
		(iUpTime % day) / hour,
		(iUpTime % hour) / minute,
		iUpTime % minute);
	*cActivityTime = g_strdup_printf ("%ld %s, %ld:%02ld:%02ld",
		iActivityTime / day, D_("day(s)"),
		(iActivityTime % day) / hour,
		(iActivityTime % hour) / minute,
		iActivityTime % minute);
}


void cd_cpusage_get_cpu_info (void)
{
	gchar *cContent = NULL;
	gsize length=0;
	g_file_get_contents (CPUSAGE_PROC_INFO_PIPE, &cContent, &length, NULL);
	if (cContent == NULL)
	{
		cd_warning ("cpusage : can't open %s, assuming their is only 1 CPU with 1 core", CPUSAGE_PROC_INFO_PIPE);
		myData.iNbCPU = 1;
	}
	else
	{
		gchar *line = cContent;
		gchar *str;
		do
		{
			str = NULL;
			if (myData.cModelName == NULL && strncmp (line, "model name", 10) == 0)
			{
				str = strchr (line+10, ':');
				if (str != NULL)
				{
					gchar *str2 = strchr (str+2, '\n');
					if (str2 != NULL)
					{
						*str2 = '\0';
						myData.cModelName = g_strdup (str + 2);  // on saute l'espace apres le ':'.
						*str2 = '\n';
					}
				}
			}
			else if (myData.iFrequency == 0 && strncmp (line, "cpu MHz", 7) == 0)
			{
				str = strchr (line+7, ':');
				if (str != NULL)
				{
					myData.iFrequency = atoi (str + 2);  // on saute l'espace apres le ':'.
				}
			}
			else if (strncmp (line, "processor", 9) == 0)  // processeur virtuel.
			{
				cd_debug ("  found 1 virtual processor");
				myData.iNbCPU ++;
			}
			/*else if (strncmp (line, "physical id", 11) == 0)  // processeur physique.
			{
				
			}
			else if (strncmp (line, "cpu cores", 9) == 0)  // nbre de coeurs de ce processeur physique.
			{
				
			}*/
			
			if (str != NULL)
				line = str;  // optimisation : on se place au milieu de la ligne.
			
			str = strchr (line, '\n');
			if (str == NULL)
				break ;  // on cherche tous les processeurs.
			line = str + 1;
		}
		while (TRUE);
	}
	myData.iNbCPU = MAX (myData.iNbCPU, 1);
	cd_debug ("cpusage : %d CPU/core(s) found", myData.iNbCPU);
	g_free (cContent);
}


#define go_to_next_value(tmp) \
	tmp ++; \
	while (g_ascii_isdigit (*tmp)) \
		tmp ++; \
	while (*tmp == ' ') \
		tmp ++; \
	if (*tmp == '\0') { \
		cd_warning ("cpusage : problem when reading pipe"); \
		myData.bAcquisitionOK = FALSE; \
		return ; \
	}
void cd_cpusage_read_data (CairoDockModuleInstance *myApplet)
{
	static char cContent[512+1];
	
	FILE *fd = fopen (CPUSAGE_DATA_PIPE, "r");
	if (fd == NULL)
	{
		cd_warning ("cpusage : can't open %s", CPUSAGE_DATA_PIPE);
		myData.bAcquisitionOK = FALSE;
		return ;
	}
	
	gchar *tmp = fgets (cContent, 512, fd);  // on ne prend que la 1ere ligne, somme de tous les processeurs.
	fclose (fd);
	if (tmp == NULL)
	{
		cd_warning ("cpusage : can't read %s", CPUSAGE_DATA_PIPE);
		myData.bAcquisitionOK = FALSE;
		return ;
	}
	
	g_timer_stop (myData.pClock);
	double fTimeElapsed = g_timer_elapsed (myData.pClock, NULL);
	g_timer_start (myData.pClock);
	g_return_if_fail (fTimeElapsed > 0.1);  // en conf, c'est 1s minimum.
	
	long long int new_cpu_user = 0, new_cpu_user_nice = 0, new_cpu_system = 0, new_cpu_idle = 0;
	tmp += 3;  // on saute 'cpu'.
	while (*tmp == ' ')  // on saute les espaces.
		tmp ++;
	new_cpu_user = atoll (tmp);
	
	go_to_next_value(tmp)
	new_cpu_user_nice = atoll (tmp);
	
	go_to_next_value(tmp)
	new_cpu_system = atoll (tmp);
	
	go_to_next_value(tmp)
	new_cpu_idle = atoll (tmp);
	
	if (myData.bInitialized)  // la 1ere iteration on ne peut pas calculer la frequence.
	{
		myData.cpu_usage = 100. * (1. - (new_cpu_idle - myData.cpu_idle) / myConfig.fUserHZ / myData.iNbCPU / fTimeElapsed);
		if (myData.cpu_usage < 0)  // peut arriver car le fichier pipe est pas mis a jour tous les dt, donc il y'a potentiellement un ecart de dt avec la vraie valeur. Ca plus le temps d'execution.  
			myData.cpu_usage = 0;
		cd_debug ("CPU(%d) user : %d -> %d / nice : %d -> %d / sys : %d -> %d / idle : %d -> %d",
			myData.iNbCPU,
			myData.cpu_user, new_cpu_user,
			myData.cpu_user_nice, new_cpu_user_nice,
			myData.cpu_system, new_cpu_system,
			myData.cpu_idle, new_cpu_idle);
		cd_debug ("=> CPU user : %.3f / nice : %.3f / sys : %.3f / idle : %.3f",
			(new_cpu_user - myData.cpu_user) / myConfig.fUserHZ / myData.iNbCPU / fTimeElapsed,
			(new_cpu_user_nice - myData.cpu_user_nice) / myConfig.fUserHZ / myData.iNbCPU / fTimeElapsed,
			(new_cpu_system - myData.cpu_system) / myConfig.fUserHZ / myData.iNbCPU / fTimeElapsed,
			(new_cpu_idle - myData.cpu_idle) / myConfig.fUserHZ / myData.iNbCPU / fTimeElapsed);
	}
	myData.bAcquisitionOK = TRUE;
	myData.cpu_user = new_cpu_user;
	myData.cpu_user_nice = new_cpu_user_nice;
	myData.cpu_system = new_cpu_system;
	myData.cpu_idle = new_cpu_idle;
	
	if (! myData.bInitialized)
	{
		cd_cpusage_get_cpu_info ();
		myData.bInitialized = TRUE;
	}
}


gboolean cd_cpusage_update_from_data (CairoDockModuleInstance *myApplet)
{
	if ( ! myData.bAcquisitionOK)
	{
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		else if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
		double fValue = 0.;
		cairo_dock_render_new_data_on_icon (myIcon, myContainer, myDrawContext, &fValue);
		/*if (myData.pGauge)
		{
			CD_APPLET_RENDER_GAUGE (myData.pGauge, 0.);
		}
		else
		{
			CD_APPLET_RENDER_GRAPH (myData.pGraph);
		}*/
	}
	else
	{
		if (! myData.bInitialized)
		{
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (myDock ? "..." : D_("Loading"));
			double fValue = 0.;
			cairo_dock_render_new_data_on_icon (myIcon, myContainer, myDrawContext, &fValue);
			/*if (myData.pGauge)
			{
				CD_APPLET_RENDER_GAUGE (myData.pGauge, 0.);
			}
			else
			{
				CD_APPLET_RENDER_GRAPH (myData.pGraph);
			}*/
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
			
			double fValue = (double) myData.cpu_usage / 100;
			double test[4] = {fValue, fValue/2, fValue/2, fValue};
			cairo_dock_render_new_data_on_icon (myIcon, myContainer, myDrawContext, test/*&fValue*/);
			/*if (myData.pGauge)
			{
				CD_APPLET_RENDER_GAUGE (myData.pGauge, (double) myData.cpu_usage / 100);
			}
			else
			{
				CD_APPLET_RENDER_GRAPH_NEW_VALUE (myData.pGraph, (double) myData.cpu_usage / 100);
			}*/
		}
	}
	return myData.bAcquisitionOK;
}



#define jump_to_next_value(tmp) \
	while (*tmp != ' ' && *tmp != '\0') \
		tmp ++; \
	if (*tmp == '\0') { \
		cd_warning ("cpusage : problem when reading pipe"); \
		break ; \
	} \
	while (*tmp == ' ') \
		tmp ++; \

void cd_cpusage_free_process (CDProcess *pProcess)
{
	if (pProcess == NULL)
		return ;
	g_free (pProcess->cName);
	g_free (pProcess);
}


void cd_cpusage_get_process_times (double fTime, double fTimeElapsed)
{
	static gchar cFilePathBuffer[20+1];  // /proc/12345/stat + 4octets de marge.
	static gchar cContent[512+1];
	
	cd_debug ("%s (%.2f)", __func__, fTimeElapsed);
	GError *erreur = NULL;
	GDir *dir = g_dir_open (CD_CPUSAGE_PROC_FS, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("cpusage : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	if (myData.pProcessTable == NULL)
		myData.pProcessTable = g_hash_table_new_full (g_int_hash, g_int_equal, NULL, (GDestroyNotify) cd_cpusage_free_process);  // la cle est dans la valeur.
	if (myData.pTopList == NULL)
		myData.pTopList = g_new0 (CDProcess *, myConfig.iNbDisplayedProcesses);
	else
		memset (myData.pTopList, 0, myConfig.iNbDisplayedProcesses * sizeof (CDProcess *));
	
	const gchar *cPid;
	gchar *tmp;
	CDProcess *pProcess;
	int iNewCpuTime;
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
			cd_warning ("cpusage : can't read %s", cFilePathBuffer);
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
			*tmp ++;  // on saute la '('.
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
		
		//g_print ("%s : %d -> %d\n", pProcess->cName, pProcess->iCpuTime, iNewCpuTime);
		if (pProcess->iCpuTime != 0 && fTimeElapsed != 0)
			pProcess->fCpuPercent = (iNewCpuTime - pProcess->iCpuTime) / myConfig.fUserHZ / myData.iNbCPU / fTimeElapsed;
		pProcess->iCpuTime = iNewCpuTime;
		
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
	
	g_dir_close (dir);
}


static gboolean _cd_clean_one_old_processes (int *iPid, CDProcess *pProcess, double *fTime)
{
	if (pProcess->fLastCheckTime < *fTime)
		return TRUE;
	return FALSE;
}
void cd_cpusage_clean_old_processes (double fTime)
{
	g_hash_table_foreach_remove (myData.pProcessTable, (GHRFunc) _cd_clean_one_old_processes, &fTime);
}


void cd_cpusage_clean_all_processes (void)
{
	g_hash_table_remove_all (myData.pProcessTable);
}
