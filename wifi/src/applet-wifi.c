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

#define WIFI_TMP_FILE "/tmp/wifi"

static gchar *s_cLevelQualityName[WIFI_NB_QUALITY] = {N_("None"), N_("Very Low"), N_("Low"), N_("Middle"), N_("Good"), N_("Exellent")};

static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;


gboolean cd_wifi_timer (gpointer data) {
	cd_wifi_launch_measure();
	return TRUE;
}

gpointer cd_wifi_threaded_calculation (gpointer data) {
	cd_wifi_get_data();
	
	g_atomic_int_set (&s_iThreadIsRunning, 0);
	cd_message ("*** fin du thread wifi");
	return NULL;
}

void cd_wifi_get_data (void) {
	gchar *cCommand = g_strdup_printf("bash %s/wifi", MY_APPLET_SHARE_DATA_DIR);
	system (cCommand);
	g_free (cCommand);
}

static gboolean _cd_wifi_check_for_redraw (gpointer data) {
	int iThreadIsRunning = g_atomic_int_get (&s_iThreadIsRunning);
	cd_message ("%s (%d)", __func__, iThreadIsRunning);
	if (! iThreadIsRunning) {
		s_iSidTimerRedraw = 0;
		if (myIcon == NULL) {
			g_print ("annulation du chargement du wifi\n");
			return FALSE;
		}
		
		gboolean bResultOK = cd_wifi_getStrength ();
		
		//\_______________________ On lance le timer si necessaire.
		if (myData.iSidTimer == 0) {
			myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_wifi_timer, NULL);
		}
		return FALSE;
	}
	return TRUE;
}

void cd_wifi_launch_measure (void) {
	cd_message (" ");
	if (g_atomic_int_compare_and_exchange (&s_iThreadIsRunning, 0, 1)) {  //il etait egal a 0, on lui met 1 et on lance le thread.
		cd_message (" ==> lancement du thread de calcul");
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) cd_wifi_threaded_calculation, NULL, FALSE, &erreur);
		if (erreur != NULL) {
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
				
		if (s_iSidTimerRedraw == 0) {
			s_iSidTimerRedraw = g_timeout_add (333, (GSourceFunc) _cd_wifi_check_for_redraw, (gpointer) NULL);
		}
		
	}
}

gboolean cd_wifi(gpointer data) {
	static gboolean bBusy = FALSE;
	if (bBusy)
		return TRUE;
	bBusy = TRUE;
	
	GError *erreur = NULL;
	gchar *cCommand = g_strdup_printf("bash %s/wifi", MY_APPLET_SHARE_DATA_DIR);
	g_spawn_command_line_async (cCommand, &erreur);
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'iwconfig", erreur->message);
		g_error_free (erreur);
	}
	g_free (cCommand);
		
		
	myData.strengthTimer = g_timeout_add (500, (GSourceFunc) cd_wifi_getStrength, (gpointer) NULL);
		
	bBusy = FALSE;
	if (myData.isWirelessDevice == 0) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

static float pourcent(float x, float y) {
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

static gboolean _wifi_get_values_from_file (gchar *cContent, int *iFlink, int *iMlink, int *iPercentage, CDWifiQuality *iQuality) {
	gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
	g_free(cContent);
	gchar *cOneInfopipe;
	const gchar *levelName;
	int flink=0, mlink=0, i=0,prcnt=0;
	for (i = 0; cInfopipesList[i] != NULL; i ++) {
		cOneInfopipe = cInfopipesList[i];
		if ((i == 0) && (strcmp(cOneInfopipe,"Wifi") == 0)) {
		  myData.isWirelessDevice = 0;
			g_strfreev (cInfopipesList);
			return FALSE;
		}
		else if (i == 5) {
			myData.isWirelessDevice = 1; //Wireless Devices found
			
			int c = 0, iNbSpace = 0;
			gchar *cUtilInfo = NULL;
			while (cOneInfopipe[c] != '\0') 	{
				if (cOneInfopipe[c] == ' ') {
					iNbSpace ++;
					if (iNbSpace == 11) {
						cUtilInfo = &cOneInfopipe[c+1];
						break ;
					}
				}
				c ++;
			}
			
			if (cUtilInfo != NULL) {
				gchar *str = strchr (cUtilInfo, '=');
				if (str == NULL) {
					str = strchr (cUtilInfo, ':');
				}
				if (str != NULL) {
					cUtilInfo = str + 1;
					str = strchr (cUtilInfo, '/');
					if (str != NULL) {
						*str = '\0';
						flink = atoi (cUtilInfo);
						mlink = atoi (str+1);
						
						cd_debug("Signal Quality: %d/%d", flink, mlink);
						prcnt = pourcent(flink,mlink);
					}
				}
			}
			
		}
	}
	g_strfreev (cInfopipesList);
	
	*iFlink = flink;
	*iMlink = mlink;
	if (prcnt <= 0) {
		*iQuality = WIFI_QUALITY_NO_SIGNAL;
	}
	else if (prcnt < 20) {
		*iQuality = WIFI_QUALITY_VERY_LOW;
	}
	else if (prcnt < 40) {
		*iQuality = WIFI_QUALITY_LOW;
	}
	else if (prcnt < 60) {
		*iQuality = WIFI_QUALITY_MIDDLE;
	}
	else if (prcnt < 80) {
		*iQuality = WIFI_QUALITY_GOOD;
	}
	else {
		*iQuality = WIFI_QUALITY_EXCELLENT;
	}
	*iPercentage = prcnt;
	return TRUE;
}


void _wifi_draw_no_wireless_extension (void) {
	CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("N/A");
	cd_wifi_set_surface (WIFI_QUALITY_NO_SIGNAL);
	myData.checkedTime = myData.checkedTime+1;
	
	if (myData.checkedTime == 1) {
	  myConfig.iCheckInterval = 60000; //check 1min
	  if (myData.iSidTimer != 0) {
		  g_source_remove (myData.iSidTimer);
		  myData.iSidTimer = 0;
	  }
	  myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_wifi_timer, NULL);
	}
	else if (myData.checkedTime == 2) {
	  myConfig.iCheckInterval = 180000; //check 3min
	  if (myData.iSidTimer != 0) {
		  g_source_remove (myData.iSidTimer);
		  myData.iSidTimer = 0;
	  }
	  myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_wifi_timer, NULL);
	}
	else if (myData.checkedTime >= 3) {
	  myConfig.iCheckInterval = 900000; //check 5min
	  if (myData.iSidTimer != 0) {
		  g_source_remove (myData.iSidTimer);
		  myData.iSidTimer = 0;
	  }
	  myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_wifi_timer, NULL);
	}
	
	cd_message("No wifi device found, timer changed %d.",myConfig.iCheckInterval);
	myData.isWirelessDevice = 0; //No wireless device
	myData.iPreviousQuality = WIFI_QUALITY_NO_SIGNAL;
}

gboolean cd_wifi_getStrength(void) {
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(WIFI_TMP_FILE, &cContent, &length, &erreur);
	if (erreur != NULL)	{
		cd_warning("Attention : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		_wifi_draw_no_wireless_extension ();
		return FALSE;
	}
	else {
		int flink,mlink;
		int prcnt;
		CDWifiQuality iQuality;
		gboolean bAcquisitionOK = _wifi_get_values_from_file (cContent, &flink, &mlink, &prcnt, &iQuality);
		
		if (! bAcquisitionOK) {
			_wifi_draw_no_wireless_extension();
			return FALSE;
		}
		if (prcnt <= 0) {
		  _wifi_draw_no_wireless_extension();
			return FALSE;
		}
		
		if (myConfig.iCheckInterval != myConfig.dCheckInterval) {
	    myConfig.iCheckInterval = myConfig.dCheckInterval;
	    if (myData.iSidTimer != 0) {
		    g_source_remove (myData.iSidTimer);
		    myData.iSidTimer = 0;
	    }
	    myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_wifi_timer, NULL);
	  }
		
		myData.checkedTime = 0; // On remet a zero le compteur
	  switch (myConfig.quickInfoType) {
		  case WIFI_INFO_NONE :
		    	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(NULL);
		  break;
		  case WIFI_INFO_SIGNAL_STRENGTH_LEVEL :
		   	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(_D(s_cLevelQualityName[iQuality]));
		  break;
		  case WIFI_INFO_SIGNAL_STRENGTH_PERCENT :
			  CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d%%", prcnt);
		  break;
		  case WIFI_INFO_SIGNAL_STRENGTH_DB :
			  CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("%d/%d", flink, mlink);
		  break;
	  }
			
			cd_wifi_set_surface (iQuality);
			
	}
	return TRUE;
}

void cd_wifi_set_surface (CDWifiQuality iQuality) {
	g_return_if_fail (iQuality < WIFI_NB_QUALITY);
	
	cairo_surface_t *pSurface = myData.pSurfaces[iQuality];
	if (pSurface == NULL) {
		if (myConfig.cUserImage[iQuality] != NULL) {
			gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cUserImage[iQuality]);
			myData.pSurfaces[iQuality] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
			g_free (cUserImagePath);
		}
		else {
			gchar *cImagePath = g_strdup_printf ("%s/link-%d.svg", MY_APPLET_SHARE_DATA_DIR, iQuality);
			myData.pSurfaces[iQuality] = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
			g_free (cImagePath);
		}
		CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pSurfaces[iQuality]);
	}
	else {
		CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
	}
}
