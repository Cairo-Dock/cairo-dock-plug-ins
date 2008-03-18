#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-wifi.h"
#include "cairo-dock.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;

gboolean cd_wifi(gchar *origine) {
  static gboolean bBusy = FALSE;
  
	if (bBusy)
		return TRUE;
	bBusy = TRUE;
  
  cd_debug("Wifi: Execution called from %s\n", origine);
  
  GError *erreur = NULL;
  g_spawn_command_line_async (g_strdup_printf("bash %s/wifi", MY_APPLET_SHARE_DATA_DIR), &erreur);
  if (erreur != NULL) {
	  cd_warning ("Attention : when trying to execute 'iwconfig", erreur->message);
    g_error_free (erreur);
	}
	
	myData.strengthTimer = g_timeout_add (500, (GSourceFunc) cd_wifi_getStrength, (gpointer) NULL); 
	
  bBusy = FALSE;
  if (myData.isWirelessDevice == 0) {
	  return FALSE;
	}
	else {
	  return TRUE;
	}
}

gboolean cd_wifi_getStrength(void) {
  static gboolean bBusy = FALSE;
  
	if (bBusy)
		return FALSE;
	bBusy = TRUE;
	
  gchar *cContent = NULL;
	gsize length=0;
	GError *tmp_erreur = NULL;
	g_file_get_contents("/tmp/wifi", &cContent, &length, &tmp_erreur);
	if ((tmp_erreur != NULL) || (cContent == NULL)) {
		cd_message("Attention : %s\n", tmp_erreur->message);
		g_error_free(tmp_erreur);
		CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("N/A");
		CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pDefault);
	}
	else {
	  gchar *cQuickInfo;
		gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
		g_free(cContent);
		gchar *cOneInfopipe;
		gchar **tcnt,**bcnt,*levelName;
    int flink,mlink,i=0;
    int prcnt;
		cQuickInfo = " ";
		for (i = 0; cInfopipesList[i] != NULL; i ++) {
			cOneInfopipe = cInfopipesList[i];
			if ((i == 0) && (strcmp(cOneInfopipe,"Wifi") == 0)) {
			  CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		    CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("N/A");
		    CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pDefault);
		    cd_message("No wifi device found, timer stopped.\n");
        myData.isWirelessDevice = 0;
		    bBusy = FALSE;
		    return FALSE;
		  }
			else if (i == 5) {
			  myData.isWirelessDevice = 1; //Wireless Devices found
			  
			  tcnt = g_strsplit(cOneInfopipe," ", -1);
			  bcnt = g_strsplit(tcnt[11],"=", -1);
			  if (bcnt[1] == NULL) {
			    bcnt = g_strsplit(tcnt[11],":", -1);
			  }
			  tcnt = g_strsplit(bcnt[1],"/", -1);
			  flink = atoi(tcnt[0]);
			  mlink = atoi(tcnt[1]);
			  
			  //Thanks to Ahmad Baitalmal & Brian Elliott Finley for thoses values (extracted from wifi-radar phyton script)
			  cd_debug("Signal Quality: %d / %d\n", flink, mlink);
			  
			  prcnt = pourcent(flink,mlink);
			  
			  if ((prcnt > 0)  && (prcnt <= 20)) {
			    levelName=_D("Very Low");
			    CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.p2Surface);
			  }
			  else if ((prcnt > 20)  && (prcnt <= 40)) {
			    levelName=_D("Low");
			    CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.p4Surface);
			  }
			  else if ((prcnt > 40)  && (prcnt <= 60)) {
			    levelName=_D("Middle");
			    CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.p6Surface);
			  }
			  else if ((prcnt > 60)  && (prcnt <= 80)) {
			    levelName=_D("Good");
			    CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.p8Surface);
			  }
			  else if ((prcnt > 80)  && (prcnt <= 100)) {
			    levelName=_D("Exellent");
			    CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.p1Surface);
			  }
			  else {
			    levelName=_D("None");
			    CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pDefault);
			  }
			}
		}
		if (myConfig.enableSSQ) {
		  switch (myConfig.quickInfoType) {
		    case 0: CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(levelName); break;
		    case 1: cairo_dock_set_quick_info (myDrawContext, g_strdup_printf ("%d%s ",prcnt,"%"), myIcon, (myDock != NULL ? 1 + g_fAmplitude : 1)); break;
		    case 2: CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(g_strdup_printf ("%d/%d", flink, mlink)); break;
		  }
		}
		else {
		  CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(NULL);
		}
  }
  
  bBusy = FALSE;
	return FALSE;
}

float pourcent(float x, float y) {
  float p=0;
  if (x > y) {
    x = y;
  }
  else if (x < 0) {
   x = 0;
  }
  p = (x / y) *100;
  return p;
}

void cd_wifi_init(gchar *origine) {
  cd_debug("Wifi: Initialisation called from %s\n", origine);
	myData.isWirelessDevice = 1;
	cd_wifi("Wifi_Init");
  myData.checkTimer = g_timeout_add (10000, (GSourceFunc) cd_wifi, (gpointer) origine);
}

void cd_wifi_wait(gchar *origine) {
  cd_debug("Wifi: Check called from %s\n", origine);
  CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("Check...");
	CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pDefault);
	myData.isWirelessDevice = 1;
  myData.checkTimer = g_timeout_add (10000, (GSourceFunc) cd_wifi, (gpointer) origine);
}

