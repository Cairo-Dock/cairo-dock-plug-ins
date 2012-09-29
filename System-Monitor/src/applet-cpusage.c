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
#include "applet-cpusage.h"

#define CPUSAGE_DATA_PIPE		CD_SYSMONITOR_PROC_FS"/stat"
#define CPUSAGE_UPTIME_PIPE		CD_SYSMONITOR_PROC_FS"/uptime"
#define CPUSAGE_LOADAVG_PIPE	CD_SYSMONITOR_PROC_FS"/loadavg"
#define CPUSAGE_PROC_INFO_PIPE	CD_SYSMONITOR_PROC_FS"/cpuinfo"


void cd_sysmonitor_get_uptime_info (GString *pInfo)
{
	FILE *fd = fopen (CPUSAGE_UPTIME_PIPE, "r");
	if (fd == NULL)
	{
		cd_warning ("can't open %s", CPUSAGE_UPTIME_PIPE);
		return ;
	}
	
	double fUpTime = 0, fIdleTime = 0;
	int r = fscanf (fd, "%lf %lf\n", &fUpTime, &fIdleTime);
	if (r == EOF)
		cd_warning ("Failed to use fscanf");
	fclose (fd);
	
	const int minute = 60;
	const int hour = minute * 60;
	const int day = hour * 24;
	int iUpTime = (int) fUpTime, iActivityTime = (int) (fUpTime - fIdleTime);
	
	g_string_append_printf (pInfo, "\n  %s : %d %s, %d:%02d:%02d / %s : %d %s, %d:%02d:%02d",
		D_("Uptime"),
		iUpTime / day, D_("day(s)"),
		(iUpTime % day) / hour,
		(iUpTime % hour) / minute,
		iUpTime % minute,
		D_("Activity time"),
		iActivityTime / day, D_("day(s)"),
		(iActivityTime % day) / hour,
		(iActivityTime % hour) / minute,
		iActivityTime % minute);
}


static void _get_cpu_info (CairoDockModuleInstance *myApplet)
{
	gchar *cContent = NULL;
	gsize length=0;
	g_file_get_contents (CPUSAGE_PROC_INFO_PIPE, &cContent, &length, NULL);
	if (cContent == NULL)
	{
		cd_warning ("sysmonitor : can't open %s, assuming their is only 1 CPU with 1 core", CPUSAGE_PROC_INFO_PIPE);
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
				line = str;  // optimisation : on se place apres ce qu'on a trouve.
			
			str = strchr (line, '\n');
			if (str == NULL)
				break ;  // on cherche tous les processeurs.
			line = str + 1;
		}
		while (TRUE);
	}
	myData.iNbCPU = MAX (myData.iNbCPU, 1);
	cd_debug ("sysmonitor : %d CPU/core(s) found", myData.iNbCPU);
	g_free (cContent);
}

void cd_sysmonitor_get_cpu_info (CairoDockModuleInstance *myApplet, GString *pInfo)
{
	if (myData.iNbCPU == 0)  // on n'a encore jamais initialise ca.
		_get_cpu_info (myApplet);
	
	if (pInfo != NULL)
		g_string_append_printf (pInfo,"%s : %s\n  %s : %d MHz (%d %s)",
			D_("CPU model"), myData.cModelName,
			D_("Frequency"), myData.iFrequency,
			myData.iNbCPU, D_("core(s)"));  // on ne met pas les infos du thread, car la charge CPU varie trop vite pour etre pertinente a un instant donne.
}


#define go_to_next_value(tmp) \
	tmp ++; \
	while (g_ascii_isdigit (*tmp)) \
		tmp ++; \
	while (*tmp == ' ') \
		tmp ++; \
	if (*tmp == '\0') { \
		cd_warning ("sysmonitor : problem when reading pipe"); \
		myData.bAcquisitionOK = FALSE; \
		return ; }

void cd_sysmonitor_get_cpu_data (CairoDockModuleInstance *myApplet)
{
	static char cContent[512+1];
	
	if (myData.iNbCPU == 0)  // on n'a encore jamais initialise ca.
		_get_cpu_info (myApplet);  // met toujours iNbCPU a 1 au moins.
	
	FILE *fd = fopen (CPUSAGE_DATA_PIPE, "r");
	if (fd == NULL)
	{
		cd_warning ("sysmonitor : can't open %s", CPUSAGE_DATA_PIPE);
		myData.bAcquisitionOK = FALSE;
		return ;
	}
	
	gchar *tmp = fgets (cContent, 512, fd);  // on ne prend que la 1ere ligne, somme de tous les processeurs.
	fclose (fd);
	if (tmp == NULL)
	{
		cd_warning ("sysmonitor : can't read %s", CPUSAGE_DATA_PIPE);
		myData.bAcquisitionOK = FALSE;
		return ;
	}
	
	double fTimeElapsed = g_timer_elapsed (myData.pClock, NULL);
	g_timer_start (myData.pClock);
	g_return_if_fail (fTimeElapsed > 0.1 || !myData.bInitialized);  // en conf, c'est 1s minimum.
	
	long long int new_cpu_user = 0, new_cpu_user_nice = 0, new_cpu_system = 0, new_cpu_idle = 0;
	tmp += 3;  // on saute 'cpu'.
	while (*tmp == ' ')  // on saute les espaces.
		tmp ++;
	new_cpu_user = atoll (tmp);
	
	go_to_next_value(tmp)
	new_cpu_user_nice = atoll (tmp);
	//g_print ("%lld --> %lld\n", myData.cpu_user, new_cpu_user);
	
	go_to_next_value(tmp)
	new_cpu_system = atoll (tmp);
	
	go_to_next_value(tmp)
	new_cpu_idle = atoll (tmp);
	
	if (myData.bInitialized)  // la 1ere iteration on ne peut pas calculer la frequence.
	{
		myData.fCpuPercent = 100. * (1. - (new_cpu_idle - myData.cpu_idle) / myConfig.fUserHZ / myData.iNbCPU / fTimeElapsed);
		if (myData.fCpuPercent < 0)  // peut arriver car le fichier pipe est pas mis a jour tous les dt, donc il y'a potentiellement un ecart de dt avec la vraie valeur. Ca plus le temps d'execution.  
			myData.fCpuPercent = 0;
		if (fabs (myData.fCpuPercent - myData.fPrevCpuPercent) > 1)
		{
			myData.fPrevCpuPercent = myData.fCpuPercent;
			myData.bNeedsUpdate = TRUE;
		}
		/*cd_debug ("CPU(%d) user : %d -> %d / nice : %d -> %d / sys : %d -> %d / idle : %d -> %d",
			myData.iNbCPU,
			myData.cpu_user, new_cpu_user,
			myData.cpu_user_nice, new_cpu_user_nice,
			myData.cpu_system, new_cpu_system,
			myData.cpu_idle, new_cpu_idle);
		cd_debug ("=> CPU user : %.3f / nice : %.3f / sys : %.3f / idle : %.3f",
			(new_cpu_user - myData.cpu_user) / myConfig.fUserHZ / myData.iNbCPU / fTimeElapsed,
			(new_cpu_user_nice - myData.cpu_user_nice) / myConfig.fUserHZ / myData.iNbCPU / fTimeElapsed,
			(new_cpu_system - myData.cpu_system) / myConfig.fUserHZ / myData.iNbCPU / fTimeElapsed,
			(new_cpu_idle - myData.cpu_idle) / myConfig.fUserHZ / myData.iNbCPU / fTimeElapsed);*/
	}
	myData.cpu_user = new_cpu_user;
	myData.cpu_user_nice = new_cpu_user_nice;
	myData.cpu_system = new_cpu_system;
	myData.cpu_idle = new_cpu_idle;
}
