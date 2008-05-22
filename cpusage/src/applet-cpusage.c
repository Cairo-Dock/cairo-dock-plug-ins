#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-cpusage.h"
#include "cairo-dock.h"

#define CPUSAGE_DATA_PIPE "/proc/stat"
#define CPUSAGE_PROC_INFO_PIPE "/proc/cpuinfo"
#define USER_HZ 100.

CD_APPLET_INCLUDE_MY_VARS


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
		cd_warning ("problem when readgin pipe"); \
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
	gchar *tmp = fgets (cContent, 512, fd);  // on ne prend que la 1ere ligne, somme de tous les processeurs.
	fclose (fd);
	if (tmp == NULL)
	{
		cd_warning ("can't open %s", CPUSAGE_DATA_PIPE);
		myData.bAcquisitionOK = FALSE;
		return ;
	}
	else
	{
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
}


void cd_cpusage_update_from_data (void)
{
	if ( ! myData.bAcquisitionOK)
	{
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle)
		else if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
		make_cd_Gauge (myDrawContext, myContainer, myIcon, myData.pGauge, 0.);
		
		cairo_dock_downgrade_frequency_state (myData.pMeasureTimer);
	}
	else
	{
		cairo_dock_set_normal_frequency_state (myData.pMeasureTimer);
		
		if (! myData.bInitialized)
		{
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (myDock ? "..." : D_("Loading"));
			make_cd_Gauge (myDrawContext, myContainer, myIcon, myData.pGauge, 0.);
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
			
			make_cd_Gauge (myDrawContext, myContainer, myIcon, myData.pGauge, (double) myData.cpu_usage / 100);
		}
	}
}
