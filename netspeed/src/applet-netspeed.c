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

extern AppletConfig myConfig;
extern AppletData myData;

gboolean cd_netspeed(gchar *origine) {
  static gboolean bBusy = FALSE;
  
	if (bBusy)
		return TRUE;
	bBusy = TRUE;
  
  cd_debug("netspeed: Execution called from %s\n", origine);
  
  GError *erreur = NULL;
  g_spawn_command_line_async (g_strdup_printf("bash %s/netspeed", MY_APPLET_SHARE_DATA_DIR), &erreur);
  if (erreur != NULL) {
	  cd_warning ("Attention : when trying to catting /proc/net/dev", erreur->message);
    g_error_free (erreur);
	}
	
	myData.strengthTimer = g_timeout_add (500, (GSourceFunc) cd_netspeed_getRate, (gpointer) NULL); 
	
  bBusy = FALSE;
  if (myData.interfaceFound == 0) {
	  return FALSE;
	}
	else {
	  return TRUE;
	}
}

gboolean cd_netspeed_getRate(void) {

  static gboolean bBusy = FALSE;
  
	if (bBusy)
		return FALSE;
	bBusy = TRUE;
	
  gchar *cContent = NULL;
	gsize length=0;
	GError *tmp_erreur = NULL;
	g_file_get_contents("/tmp/netspeed", &cContent, &length, &tmp_erreur);
	if ((tmp_erreur != NULL) || (cContent == NULL)) {
		cd_message("Attention : %s\n", tmp_erreur->message);
		g_error_free(tmp_erreur);
		CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("N/A");
		CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pDefault);
	}
	else {
		gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
		g_free(cContent);
		gchar *cOneInfopipe;
		gchar **recup, *interface;
    		static unsigned long long nUp = 0, nDown = 0;
    		static unsigned long long time = 0;
    		unsigned long long newTime, newNUp, newNDown, upRate = 0, downRate = 0;
    		int prcnt, i;
		
		for (i = 0; cInfopipesList[i] != NULL; i ++) {
			cOneInfopipe = cInfopipesList[i];
			if ((i == 0) && (strcmp(cOneInfopipe,"netspeed") == 0)) {
				CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		    		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("N/A");
		    		CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pBad);
		    		cd_message("No interface found, timer stopped.\n");
				myData.interfaceFound = 0;
		    		bBusy = FALSE;
		    		return FALSE;
		  	}
			else {
				myData.interfaceFound = 1; //Interface found
				if(strcmp(cOneInfopipe,"time") == 0) {
					cd_debug("netspeed -> read END !\n");	
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
				cd_debug("netspeed -> read : Interface %s\n", interface);
				CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pUnknown);
			}
		}
		cd_debug("netspeed -> last time -> %lld\n",time);
		if(time != 0)
		{
			
			newTime = atoll(cInfopipesList[i+1]);
			cd_debug("netspeed -> Time Diff -> %lld\n", (newTime - time));
			// On calcule le débit en octet par secondes
			if((newTime - time) != 0)
			{
				upRate = (unsigned long long) (((newNUp - nUp) * 1000000000) / (newTime - time));
				downRate = (unsigned long long) (((newNDown - nDown) * 1000000000) / (newTime - time));
				cd_debug("Up : %lld - Down : %lld", upRate, downRate);
			}
			gchar upRateFormatted[11];
			gchar downRateFormatted[11];
			cd_netspeed_formatRate(upRate, &upRateFormatted);
			cd_netspeed_formatRate(downRate, &downRateFormatted);
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("Up : %s - Down : %s", upRateFormatted,downRateFormatted);

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
  }
  
  bBusy = FALSE;
	return FALSE;
}


void cd_netspeed_init(gchar *origine) {
  cd_debug("netspeed: Initialisation called from %s\n", origine);
	myData.interfaceFound = 1;
	cd_netspeed("netspeed_Init");
  myData.checkTimer = g_timeout_add (1000, (GSourceFunc) cd_netspeed, (gpointer) origine);
}

void cd_netspeed_wait(gchar *origine) {
  cd_debug("netspeed: Check called from %s\n", origine);
  CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("Check...");
	CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pDefault);
	myData.interfaceFound = 1;
  myData.checkTimer = g_timeout_add (1000, (GSourceFunc) cd_netspeed, (gpointer) origine);
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
 		g_sprintf(debit, "%i TB/s", smallRate);
 	}
 	else if (rate > 1000000000)
 	{
 		smallRate = (int) (rate / 1000000000); 
 		g_sprintf(debit, "%i GB/s", smallRate);
 	}
 	else if (rate > 1000000)
 	{
 		smallRate = (int) (rate / 1000000);
  		g_sprintf(debit, "%i MB/s", smallRate);		
 	}
 	else if (rate > 1000)
 	{
 		smallRate = (int) (rate / 1000);
  		g_sprintf(debit, "%i KB/s", smallRate);	
 	}
 	else
 	{
 		smallRate = rate;
		g_sprintf(debit, "%i B/s", smallRate);
 	}
}

	
