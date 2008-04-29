#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-cpusage.h"
#include "cairo-dock.h"

CD_APPLET_INCLUDE_MY_VARS


#define CPUSAGE_DATA_PIPE "/proc/stat"
#define USER_HZ 100.

static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;

/*gboolean cd_cpusage_getUsage(void) {
 	gchar *cContent = NULL;
	gsize length=0;
	GError *tmp_erreur = NULL;
	g_file_get_contents(CPUSAGE_TMP_FILE, &cContent, &length, &tmp_erreur);
	if (tmp_erreur != NULL) {
		cd_message("Attention : %s\n", tmp_erreur->message);
		g_error_free(tmp_erreur);
		CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF("N/A");
		return FALSE;
	}
	else {
		gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
		g_free(cContent);
		gchar *cOneInfopipe;
		gchar **recup;
		GList *pList = NULL;
		double *pValue;
		static unsigned int cpu_user = 0, cpu_user_nice = 0, cpu_system = 0, cpu_idle = 0 ;
		unsigned int new_cpu_user = 0, new_cpu_user_nice = 0, new_cpu_system = 0, new_cpu_idle = 0, cpu_usage = 0, cpu_usage_time = 0, cpu_total_time = 0;
    		// On recupere dans le fichier les donnees sur le total cpu
		cOneInfopipe = cInfopipesList[0];		
		if(strcmp(cOneInfopipe,"stop") == 0) {
			cd_error("cpusage -> Error -> Wrong /tmp/cpusage file >< !\n");	
			return FALSE;
		}
		recup = g_strsplit(cOneInfopipe,">", -1);
		new_cpu_user = atoll(recup[0]);
		new_cpu_user_nice = atoll(recup[1]);
		new_cpu_system = atoll(recup[2]);
		new_cpu_idle = atoll(recup[3]);
		cpu_usage_time  = (new_cpu_user + new_cpu_user_nice + new_cpu_system) - (cpu_user + cpu_user_nice + cpu_system);
		cpu_total_time = cpu_usage_time + (new_cpu_idle - cpu_idle);
		
		// On ignore le reste pour l'instant...
		//TODO Les multi cpu / core

		if((cpu_total_time != 0)  && (cpu_user != 0))
		{
			cpu_usage = (unsigned int) ((cpu_usage_time * 100) / cpu_total_time);
		}
		else
		{
			cpu_usage = 0;
		}
		if(myData.inDebug)
		{
		cairo_dock_show_temporary_dialog(
				"new_cpu_user %u, new_cpu_user_nice %u, \n new_cpu_system %u, new_cpu_idle %u, \n \
				cpu_user %u, cpu_user_nice %u, \n cpu_system %u, cpu_idle %u, \n cpu_usage %u, \n \
				cpu_usage_time : %u, cpu_total_time : %u",
				myIcon, myContainer, myConfig.iCheckInterval,
				new_cpu_user, new_cpu_user_nice, new_cpu_system, new_cpu_idle, 
				cpu_user, cpu_user_nice, cpu_system, cpu_idle, cpu_usage,
				cpu_usage_time, cpu_total_time);
		}
		else
		{
		cd_debug(
				"new_cpu_user %u, new_cpu_user_nice %u, \n new_cpu_system %u, new_cpu_idle %u, \n \
				cpu_user %u, cpu_user_nice %u, \n cpu_system %u, cpu_idle %u, \n cpu_usage %u, \n \
				cpu_usage_time : %u, cpu_total_time : %u",
				new_cpu_user, new_cpu_user_nice, new_cpu_system, new_cpu_idle, 
				cpu_user, cpu_user_nice, cpu_system, cpu_idle, cpu_usage,
				cpu_usage_time, cpu_total_time);
		}
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF(myDock ? "%u%%" : "cpu:%u%%", cpu_usage);

		cpu_user = new_cpu_user;
		cpu_user_nice = new_cpu_user_nice;
		cpu_system = new_cpu_system;
		cpu_idle = new_cpu_idle;

		make_cd_Gauge(myDrawContext,myDock,myIcon,myData.pGauge,(double) cpu_usage / 100);
		//make_cd_Gauge_multiValue(myDrawContext,myDock,myIcon,myData.pGauge,pList);		
		CD_APPLET_REDRAW_MY_ICON
		g_strfreev (cInfopipesList);
	}  

	return TRUE;
}*/


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
	g_return_if_fail (fTimeElapsed != 0);
	g_timer_start (myData.pClock);
	
	FILE *fd = fopen (CPUSAGE_DATA_PIPE, "r");
	gchar *tmp = fgets (cContent, 512, fd);  // on ne prend que la 1ere ligne, moyenne (somme ?) de tous les processeurs.
	fclose (fd);
	if (tmp == NULL)
	{
		cd_warning ("impossible to open %s", CPUSAGE_DATA_PIPE);
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
			myData.cpu_usage = 100. * (1. - (new_cpu_idle - myData.cpu_idle) / USER_HZ / fTimeElapsed);
		}
		myData.bAcquisitionOK = TRUE;
		myData.cpu_user = new_cpu_user;
		myData.cpu_user_nice = new_cpu_user_nice;
		myData.cpu_system = new_cpu_system;
		myData.cpu_idle = new_cpu_idle;
	}
}


void cd_cpusage_update_from_data (void)
{
	if ( ! myData.bAcquisitionOK)
	{
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle)
		else if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF("N/A");
		make_cd_Gauge (myDrawContext, myContainer, myIcon, myData.pGauge, 0.);
		
		cairo_dock_downgrade_frequency_state (myData.pMeasureTimer);
	}
	else
	{
		cairo_dock_set_normal_frequency_state (myData.pMeasureTimer);
		
		if (! myData.bInitialized)
		{
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF(myDock ? "..." : D_("Loading"));
			make_cd_Gauge (myDrawContext, myContainer, myIcon, myData.pGauge, 0.);
			myData.bInitialized = TRUE;
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
					gchar *cInfoTitle = g_strdup_printf ("CPU : %.1f%%", myData.cpu_usage);
					CD_APPLET_SET_NAME_FOR_MY_ICON (cInfoTitle)
					g_free (cInfoTitle);
				}
			}
			
			make_cd_Gauge (myDrawContext, myContainer, myIcon, myData.pGauge, (double) myData.cpu_usage / 100);
		}
	}
}
