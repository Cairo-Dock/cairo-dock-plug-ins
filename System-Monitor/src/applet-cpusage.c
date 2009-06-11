
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-cpusage.h"

#define CPUSAGE_DATA_PIPE		CD_SYSMONITOR_PROC_FS"/stat"
#define CPUSAGE_UPTIME_PIPE		CD_SYSMONITOR_PROC_FS"/uptime"
#define CPUSAGE_LOADAVG_PIPE	CD_SYSMONITOR_PROC_FS"/loadavg"
#define CPUSAGE_PROC_INFO_PIPE	CD_SYSMONITOR_PROC_FS"/cpuinfo"


void cd_sysmonitor_get_uptime (gchar **cUpTime, gchar **cActivityTime)
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


void cd_sysmonitor_get_cpu_info (CairoDockModuleInstance *myApplet)
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
		return ; }

void cd_sysmonitor_get_cpu_data (CairoDockModuleInstance *myApplet)
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
}
