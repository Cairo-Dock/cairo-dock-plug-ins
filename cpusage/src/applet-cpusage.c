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
#define USER_HZ 100.

CD_APPLET_INCLUDE_MY_VARS


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
	static char cContent[512+1];
	
	FILE *fd = fopen (CPUSAGE_PROC_INFO_PIPE, "r");
	
	if (fd == NULL)
	{
		cd_warning ("can't open %s, assuming their is only 1 core", CPUSAGE_PROC_INFO_PIPE);
		myData.iNbCPU = 1;
	}
	else
	{
		gchar *str;
		while (fgets (cContent, 512, fd) != NULL)
		{
			if (myData.cModelName == NULL && strncmp (cContent, "model name", 10) == 0)
			{
				str = strchr (cContent, ':');
				if (str != NULL)
				{
					myData.cModelName = g_strdup (str + 2);  // on saute l'espace apres le ':'.
				}
			}
			else if (myData.iFrequency == 0 && strncmp (cContent, "cpu MHz", 7) == 0)
			{
				str = strchr (cContent, ':');
				if (str != NULL)
				{
					myData.iFrequency = atoi (str + 2);  // on saute l'espace apres le ':'.
				}
			}
			else if (myData.iNbCPU == 0 && strncmp (cContent, "cpu cores", 9) == 0)
			{
				str = strchr (cContent, ':');
				if (str != NULL)
				{
					myData.iNbCPU = atoi (str + 2);  // on saute l'espace apres le ':'.
				}
			}
		}
	}
	myData.iNbCPU = MAX (myData.iNbCPU, 1);
	fclose (fd);
}


#define go_to_next_value(tmp) \
	tmp ++; \
	while (g_ascii_isdigit (*tmp)) \
		tmp ++; \
	while (*tmp == ' ') \
		tmp ++; \
	if (*tmp == '\0') { \
		cd_warning ("problem when reading pipe"); \
		myData.bAcquisitionOK = FALSE; \
		return ; \
	}
void cd_cpusage_read_data (void)
{
	static char cContent[512+1];
	
	g_timer_stop (myData.pClock);
	double fTimeElapsed = g_timer_elapsed (myData.pClock, NULL);
	g_timer_start (myData.pClock);
	g_return_if_fail (fTimeElapsed > 0.1);  // en conf, c'est 1s minimum.
	
	FILE *fd = fopen (CPUSAGE_DATA_PIPE, "r");
	if (fd == NULL)
	{
		cd_warning ("can't open %s", CPUSAGE_DATA_PIPE);
		myData.bAcquisitionOK = FALSE;
		return ;
	}
	
	gchar *tmp = fgets (cContent, 512, fd);  // on ne prend que la 1ere ligne, somme de tous les processeurs.
	fclose (fd);
	if (tmp == NULL)
	{
		cd_warning ("can't read %s", CPUSAGE_DATA_PIPE);
		myData.bAcquisitionOK = FALSE;
		return ;
	}
	
	guint new_cpu_user = 0, new_cpu_user_nice = 0, new_cpu_system = 0, new_cpu_idle = 0;
	tmp += 3;  // on saute 'cpu'.
	while (*tmp == ' ')  // on saute les espaces.
		tmp ++;
	new_cpu_user = atoi (tmp);
	
	go_to_next_value(tmp)
	new_cpu_user_nice = atoi (tmp);
	
	go_to_next_value(tmp)
	new_cpu_system = atoi (tmp);
	
	go_to_next_value(tmp)
	new_cpu_idle = atoi (tmp);
	
	if (myData.bInitialized)  // la 1ere iteration on ne peut pas calculer la frequence.
	{
		myData.cpu_usage = 100. * (1. - (new_cpu_idle - myData.cpu_idle) / USER_HZ / myData.iNbCPU / fTimeElapsed);
		cd_debug ("CPU(%d) user : %d -> %d / nice : %d -> %d / sys : %d -> %d / idle : %d -> %d",
			myData.iNbCPU,
			myData.cpu_user, new_cpu_user,
			myData.cpu_user_nice, new_cpu_user_nice,
			myData.cpu_system, new_cpu_system,
			myData.cpu_idle, new_cpu_idle);
		cd_debug ("=> CPU user : %.3f / nice : %.3f / sys : %.3f / idle : %.3f",
			(new_cpu_user - myData.cpu_user) / USER_HZ / myData.iNbCPU / fTimeElapsed,
			(new_cpu_user_nice - myData.cpu_user_nice) / USER_HZ / myData.iNbCPU / fTimeElapsed,
			(new_cpu_system - myData.cpu_system) / USER_HZ / myData.iNbCPU / fTimeElapsed,
			(new_cpu_idle - myData.cpu_idle) / USER_HZ / myData.iNbCPU / fTimeElapsed);
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


gboolean cd_cpusage_update_from_data (void)
{
	if ( ! myData.bAcquisitionOK)
	{
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle)
		else if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
		cairo_dock_render_gauge (myDrawContext, myContainer, myIcon, myData.pGauge, 0.);
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
			if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_NONE)
			{
				if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				{
					CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ((myData.cpu_usage < 10 ? "%.1f%%" : "%.0f%%"), myData.cpu_usage)
				}
				else
				{
					if (myDock)
						CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("CPU : %.1f%%", myData.cpu_usage)
				}
			}
			
			cairo_dock_render_gauge (myDrawContext, myContainer, myIcon, myData.pGauge, (double) myData.cpu_usage / 100);
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

void cd_cpusage_free_process (CDProcess *pProcess)
{
	if (pProcess == NULL)
		return ;
	g_free (pProcess->cName);
	g_free (pProcess);
}


void cd_cpusage_get_process_times (double fTime, double fTimeElapsed)
{
	static gchar cFilePathBuffer[20+1];  // /proc/12345/stat
	static gchar cContent[512+1];
	
	cd_debug ("%s (%.2f)", __func__, fTimeElapsed);
	GError *erreur = NULL;
	GDir *dir = g_dir_open (CD_CPUSAGE_PROC_FS, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
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
			cd_warning ("can't read %s", cFilePathBuffer);
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
		iNewCpuTime = atoi (tmp);  // user.
		jump_to_next_value (tmp);
		iNewCpuTime += atoi (tmp);  // system.
		/*jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);  // on saute le nice.
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		jump_to_next_value (tmp);
		new_vsize = atoi (tmp);
		new_rss = atoi (tmp);*/
		
		//g_print ("%s : %d -> %d\n", pProcess->cName, pProcess->iCpuTime, iNewCpuTime);
		if (pProcess->iCpuTime != 0 && fTimeElapsed != 0)
			pProcess->fCpuPercent = (iNewCpuTime - pProcess->iCpuTime) / USER_HZ / myData.iNbCPU / fTimeElapsed;
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
