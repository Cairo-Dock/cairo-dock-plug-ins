#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-netspeed.h"
#include "cairo-dock.h"

CD_APPLET_INCLUDE_MY_VARS


#define NETSPEED_TMP_FILE "/tmp/netspeed"

static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;

gboolean cd_netspeed_timer (gpointer data) {
	cd_netspeed_launch_analyse();
	return TRUE;
}

gpointer cd_netspeed_threaded_calculation (gpointer data) {
	cd_netspeed_get_data();
	
	g_atomic_int_set (&s_iThreadIsRunning, 0);
	cd_message ("*** fin du thread netspeed");
	return NULL;
}

void cd_netspeed_get_data (void) {
	gchar *cCommand = g_strdup_printf("bash %s/netspeed", MY_APPLET_SHARE_DATA_DIR);
	system (cCommand);
	g_free (cCommand);
}

static gboolean _cd_netspeed_check_for_redraw (gpointer data) {
	int iThreadIsRunning = g_atomic_int_get (&s_iThreadIsRunning);
	cd_message ("%s (%d)", __func__, iThreadIsRunning);
	if (! iThreadIsRunning) {
		s_iSidTimerRedraw = 0;
		if (myIcon == NULL) {
			g_print ("annulation du chargement de netspeed (myIcon == NULL)\n");
			return FALSE;
		}
		
		gboolean bResultOK = cd_netspeed_getRate();
		
		//\_______________________ On lance le timer si necessaire.
		if (myData.iSidTimer == 0) {
			myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_netspeed_timer, NULL);
		}
		return FALSE;
	}
	return TRUE;
}

void cd_netspeed_launch_analyse (void) {
	cd_message (" ");
	if (g_atomic_int_compare_and_exchange (&s_iThreadIsRunning, 0, 1)) {  //il etait egal a 0, on lui met 1 et on lance le thread.
		cd_message (" ==> lancement du thread de calcul");
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) cd_netspeed_threaded_calculation, NULL, FALSE, &erreur);
		if (erreur != NULL) {
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
				
		if (s_iSidTimerRedraw == 0) {
			s_iSidTimerRedraw = g_timeout_add (333, (GSourceFunc) _cd_netspeed_check_for_redraw, (gpointer) NULL);
		}
		
	}
}


gboolean cd_netspeed_getRate(void) {
  	gchar *cContent = NULL;
	gsize length=0;
	GError *tmp_erreur = NULL;
	g_file_get_contents(NETSPEED_TMP_FILE, &cContent, &length, &tmp_erreur);
	if (tmp_erreur != NULL) {
		cd_message("Attention : %s\n", tmp_erreur->message);
		g_error_free(tmp_erreur);
		CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("N/A");
		/*CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pBad);*/

		return FALSE;
	}
	else {
		gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
		g_free(cContent);
		gchar *cOneInfopipe;
		gchar **recup, *interface;
    		static unsigned long long nUp = 0, nDown = 0;
    		static unsigned long long time = 0;
    		static unsigned long long maxRate = 0;
    		unsigned long long newTime, newNUp, newNDown, upRate = 0, downRate = 0, sumRate = 0;
    		int i;
		newTime = 0;
		newNUp = 0;
		newNDown = 0;
		for (i = 0; cInfopipesList[i] != NULL; i ++) {
			cOneInfopipe = cInfopipesList[i];
			if ((i == 0) && (strcmp(cOneInfopipe,"netspeed") == 0)) {
				CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		    		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("N/A");
		    		/*CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pBad);*/
		    		cd_message("No interface found, timer stopped.\n");
				myData.interfaceFound = 0;
				g_strfreev (cInfopipesList);
		    		return FALSE;
		  	}
			else {
				myData.interfaceFound = 1; //Interface found
				if(strcmp(cOneInfopipe,"time") == 0) {
					//cd_debug("netspeed -> read END !\n");	
					break;
				}
				if(strcmp(cOneInfopipe,"stop") == 0) {
					cd_error("netspeed -> Error -> Wrong /tmp/netspeed file >< !\n");	
					break;
				}
				recup = g_strsplit(cOneInfopipe,">", -1);
				interface = recup[0];
				// On ne compte pas wifi0 qui nous dit des trucs qu'on ne comprend pas...
				if(strcmp(interface,"wifi0") != 0) {
					newNDown += atoi(recup[1]);
					newNUp += atoi(recup[2]);
				}
				//cd_debug("netspeed -> read : Interface %s\n", interface);
				/*CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pUnknown);*/
			}
		}
		
		if(time != 0)
		{
			
			newTime = atoll(cInfopipesList[i+1]);
			cd_debug("netspeed -> Time Diff -> %lld\n", (newTime - time));
			// On calcule le débit en octet par secondes
			if((newTime - time) != 0)
			{
				upRate = (unsigned long long) (((newNUp - nUp) * 1000000000) / (newTime - time));
				downRate = (unsigned long long) (((newNDown - nDown) * 1000000000) / (newTime - time));
				cd_debug("Up : %llu - Down : %llu", upRate, downRate);
			}
			sumRate = upRate + downRate;
			if(sumRate > maxRate)
			{
				maxRate = sumRate;
			}
			gchar upRateFormatted[11];
			gchar downRateFormatted[11];
			cd_netspeed_formatRate(upRate, &upRateFormatted);
			cd_netspeed_formatRate(downRate, &downRateFormatted);
			if(inDebug == 1) 
			{
			cairo_dock_show_temporary_dialog(
				"nOctets avant : ↑%llu ↓%llu \n maintenant : ↑%llu ↓%llu\n diff : ↑%llu ↓%llu \n \
temps precedent : %llu \n temps courant : %llu \n Diff %llu \n sumRate : %llu \n maxRate : %llu",
				myIcon, myContainer, myConfig.iCheckInterval,
				nUp, nDown, newNUp, newNDown, newNUp - nUp, newNDown - nDown,
				time, newTime, newTime - time, sumRate, maxRate);
			}
			else
			{
			cd_debug("nOctets avant : ↑%llu ↓%llu \n maintenant : ↑%llu ↓%llu\n diff : ↑%llu ↓%llu \n \
temps precedent : %llu \n temps courant : %llu \n Diff %llu \n sumRate : %llu \n maxRate : %llu",
				nUp, nDown, newNUp, newNDown, newNUp - nUp, newNDown - nDown,
				time, newTime, newTime - time, sumRate, maxRate);
			}
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("↑%s\n↓%s", upRateFormatted,downRateFormatted);
			if(maxRate != 0)
			{
				make_cd_Gauge(myDrawContext,myDock,myIcon,myData.pGauge,(double) sumRate / maxRate);
			}
			CD_APPLET_REDRAW_MY_ICON
			time = newTime;
			nUp = newNUp;
			nDown = newNDown;
		}
		else
		{
			time = atoll(cInfopipesList[i+1]);
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("Loading");
			nUp = newNUp;
			nDown = newNDown;
		}
		g_strfreev (cInfopipesList);

		
	}  
	return TRUE;
}

// Prend un debit en octet par seconde et le transforme en une chaine de la forme : xxx yB/s
void cd_netspeed_formatRate(unsigned long long rate, gchar* debit) {
	int smallRate;
	if(rate > 1000000000000000)
	{
		g_sprintf(debit, "999+ TB/s");
 	}
 	else if (rate > 1000000000000)
 	{
 		smallRate = (int) (rate / 1000000000000);
 		g_sprintf(debit, "%i %s/s", smallRate, D_("TB"));
 	}
 	else if (rate > 1000000000)
 	{
 		smallRate = (int) (rate / 1000000000); 
 		g_sprintf(debit, "%i %s/s", smallRate, D_("TB"));
 	}
 	else if (rate > 1000000)
 	{
 		smallRate = (int) (rate / 1000000);
  		g_sprintf(debit, "%i %s/s", smallRate, D_("MB"));		
 	}
 	else if (rate > 1000)
 	{
 		smallRate = (int) (rate / 1000);
  		g_sprintf(debit, "%i %s/s", smallRate, D_("KB"));	
 	}
 	else
 	{
 		smallRate = rate;
		g_sprintf(debit, "%i %s/s", smallRate, D_("B"));
 	}
}

