#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-rame.h"
#include "cairo-dock.h"

CD_APPLET_INCLUDE_MY_VARS


#define RAME_TMP_FILE "/tmp/rame"

static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;

gboolean cd_rame_timer (gpointer data) {
	cd_rame_launch_analyse();
	return TRUE;
}

gpointer cd_rame_threaded_calculation (gpointer data) {
	cd_rame_get_data();
	
	g_atomic_int_set (&s_iThreadIsRunning, 0);
	cd_message ("*** fin du thread rame");
	return NULL;
}

void cd_rame_get_data (void) {
	gchar *cCommand = g_strdup_printf("bash %s/rame", MY_APPLET_SHARE_DATA_DIR);
	system (cCommand);
	g_free (cCommand);
}

static gboolean _cd_rame_check_for_redraw (gpointer data) {
	int iThreadIsRunning = g_atomic_int_get (&s_iThreadIsRunning);
	cd_message ("%s (%d)", __func__, iThreadIsRunning);
	if (! iThreadIsRunning) {
		s_iSidTimerRedraw = 0;
		if (myIcon == NULL) {
			g_print ("annulation du chargement de rame (myIcon == NULL)\n");
			return FALSE;
		}
		
		gboolean bResultOK = cd_rame_getRate();
		
		//\_______________________ On lance le timer si necessaire.
		if (myData.iSidTimer == 0) {
			myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_rame_timer, NULL);
		}
		return FALSE;
	}
	return TRUE;
}

void cd_rame_launch_analyse (void) {
	cd_message (" ");
	if (g_atomic_int_compare_and_exchange (&s_iThreadIsRunning, 0, 1)) {  //il etait egal a 0, on lui met 1 et on lance le thread.
		cd_message (" ==> lancement du thread de calcul");
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) cd_rame_threaded_calculation, NULL, FALSE, &erreur);
		if (erreur != NULL) {
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
				
		if (s_iSidTimerRedraw == 0) {
			s_iSidTimerRedraw = g_timeout_add (333, (GSourceFunc) _cd_rame_check_for_redraw, (gpointer) NULL);
		}
		
	}
}


gboolean cd_rame_getRate(void) {
  	gchar *cContent = NULL;
	gsize length=0;
	GError *tmp_erreur = NULL;
	g_file_get_contents(RAME_TMP_FILE, &cContent, &length, &tmp_erreur);
	if (tmp_erreur != NULL) {
		cd_message("Attention : %s\n", tmp_erreur->message);
		g_error_free(tmp_erreur);
		CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("N/A");
		return FALSE;
	}
	else {
		gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
		g_free(cContent);
		gchar *cOneInfopipe;
		gchar **recup;
		GList *pList = NULL;
		double *pValue;
    		unsigned long long ramTotal = 0,  ramUsed = 0,  ramFree = 0,  ramShared = 0, ramBuffers = 0, ramCached = 0;
   		unsigned long long swapTotal = 0, swapUsed = 0, swapFree = 0;
    		unsigned int memPercent = 0;
    		unsigned int swapPercent = 0;
		// On recupere dans le fichier les donnees sur la rame
		cOneInfopipe = cInfopipesList[0];		
		if(strcmp(cOneInfopipe,"stop") == 0) {
			cd_error("rame -> Error -> Wrong /tmp/rame file >< !\n");	
			return FALSE;
		}
		recup = g_strsplit(cOneInfopipe,">", -1);
		ramTotal = atoll(recup[0]);
		ramUsed = atoll(recup[1]);
		ramFree = atoll(recup[2]);
		ramShared = atoll(recup[3]);
		ramBuffers = atoll(recup[4]);
		ramCached = atoll(recup[5]);
		
		// On recupere dans le fichier les donnees sur la swap		
		cOneInfopipe = cInfopipesList[1];		
		if(strcmp(cOneInfopipe,"stop") == 0) {
			cd_error("rame -> Error -> Wrong /tmp/rame file middle >< !\n");	
			return FALSE;
		}
		
		recup = g_strsplit(cOneInfopipe,">", -1);		
		swapTotal = atoll(recup[0]);
		swapUsed = atoll(recup[1]);
		swapFree  = atoll(recup[2]);
		
		cOneInfopipe = cInfopipesList[2];				
		if(strcmp(cOneInfopipe,"stop") != 0) {
			cd_error("rame -> Error -> Wrong /tmp/rame file end >< !\n");	
			return FALSE;
		}
		if(ramTotal != 0) 
		{
			memPercent = (unsigned int) (((ramUsed - ramCached) * 100) / ramTotal);
		}
		else
		{
			memPercent = 0;
		}
		if(swapTotal != 0)
		{
			swapPercent = (unsigned int) ((swapUsed * 100) / swapTotal);
		}
		else
		{
			swapPercent = 0;
		}
		if(inDebug == 1) 
		{
		cairo_dock_show_temporary_dialog(
				"ramTotal : %llu ,  ramUsed : %llu ,  ramFree : %llu ,  \n ramShared : %llu , ramBuffers : %llu , ramCached : %llu ,\n \
swapTotalatoll : %llu , swapUsedatoll : %llu , swapFreeatoll : %llu",
				myIcon, myContainer, myConfig.iCheckInterval,
				ramTotal,  ramUsed,  ramFree,  ramShared, ramBuffers, ramCached,
				swapTotal, swapUsed, swapFree);
		}
		else
		{
		cd_debug(
				"ramTotal : %llu ,  ramUsed : %llu ,  ramFree : %llu ,  \n ramShared : %llu , ramBuffers : %llu , ramCached : %llu ,\n \
swapTotalatoll : %llu , swapUsedatoll : %llu , swapFreeatoll : %llu",
				ramTotal,  ramUsed,  ramFree,  ramShared, ramBuffers, ramCached,
				swapTotal, swapUsed, swapFree);
		}
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON("M:%i%%\nS:%i%%", memPercent, swapPercent);
		pValue = g_new (double, 1);
		*pValue = (double) memPercent / 100;
		pList = g_list_append (pList, pValue);
		pValue = g_new (double, 1);
		*pValue = (double) swapPercent / 100;
		pList = g_list_append (pList, pValue);
		//make_cd_Gauge(myDrawContext,myDock,myIcon,myData.pGauge,(double) memPercent / 100);
		make_cd_Gauge_multiValue(myDrawContext,myDock,myIcon,myData.pGauge,pList);		
		CD_APPLET_REDRAW_MY_ICON
		g_strfreev (cInfopipesList);
	}  
	return TRUE;
}
