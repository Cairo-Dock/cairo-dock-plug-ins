#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-infopipe.h"
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS


static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;
static GStaticMutex mutexData = G_STATIC_MUTEX_INIT;

enum {
	INFO_STATUS = 0,
	INFO_TRACK_IN_PLAYLIST,
	INFO_TIME_ELAPSED_IN_SEC,
	INFO_TIME_ELAPSED,
	INFO_TOTAL_TIME_IN_SEC,
	INFO_TOTAL_TIME,
	INFO_NOW_TITLE,
	NB_INFO
} AppletInfoEnum;

static int s_pLineNumber[MY_NB_PLAYERS][NB_INFO] = {
	{2,4,5,6,7,8,12} ,
	{0,1,2,3,4,5,6} ,
	{0,1,2,3,4,5,6} ,
	{0,1,2,3,4,5,6} ,
};


gboolean cd_xmms_timer (gpointer data) {
	cd_xmms_launch_measure ();
	return TRUE;
}

gpointer cd_xmms_threaded_calculation (gpointer data) {
	GError *erreur = NULL;
	
	gchar *cInfopipeFilePath = cd_xmms_get_pipe ();
	
	if (cInfopipeFilePath == NULL || ! g_file_test (cInfopipeFilePath, G_FILE_TEST_EXISTS)) {
		myData.playingStatus = PLAYER_NONE;
	}
	else {
		g_static_mutex_lock (&mutexData);
		cd_xmms_read_pipe(cInfopipeFilePath);
		g_static_mutex_unlock (&mutexData);
	}
	g_free (cInfopipeFilePath);
	
	g_atomic_int_set (&s_iThreadIsRunning, 0);
	cd_message ("*** fin du thread xmms");
	return NULL;
}

static gboolean _cd_xmms_check_for_redraw (gpointer data) {
	int iThreadIsRunning = g_atomic_int_get (&s_iThreadIsRunning);
	cd_message ("%s (%d)", __func__, iThreadIsRunning);
	if (! iThreadIsRunning) {
		s_iSidTimerRedraw = 0;
		if (myIcon == NULL) {
			g_print ("annulation du chargement de la meteo\n");
			return FALSE;
		}
		
		//\_______________________ On recharge l'icone principale.
		g_static_mutex_lock (&mutexData);
		cd_xmms_draw_icon ();  // lance le redraw de l'icone.
		g_static_mutex_unlock (&mutexData);
		
		//\_______________________ On lance le timer si necessaire.
		if (myData.pipeTimer == 0)
			myData.pipeTimer = g_timeout_add (1000, (GSourceFunc) cd_xmms_timer, NULL);
		return FALSE;
	}
	return TRUE;
}
void cd_xmms_launch_measure (void) {
	cd_message ("");
	if (g_atomic_int_compare_and_exchange (&s_iThreadIsRunning, 0, 1)) { // il etait egal a 0, on lui met 1 et on lance le thread.
		cd_message (" ==> lancement du thread de calcul");
		
		if (s_iSidTimerRedraw == 0)
			s_iSidTimerRedraw = g_timeout_add (250, (GSourceFunc) _cd_xmms_check_for_redraw, (gpointer) NULL);
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) cd_xmms_threaded_calculation,
			NULL,
			FALSE,
			&erreur);
		if (erreur != NULL) {
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
	}
}


//Fonction qui definie quel tuyau a emprunter pour récupérer les infos
gchar *cd_xmms_get_pipe(void) {
	gchar *cInfopipeFilePath = NULL;
	gchar *cCommand = NULL;
	switch (myConfig.iPlayer) {
		case MY_XMMS :
			cInfopipeFilePath = g_strdup_printf("/tmp/xmms-info_%s.0",g_getenv ("USER"));
		break ;
		case MY_AUDACIOUS :  //Il faut émuler le pipe d'audacious par AUDTOOL
			cInfopipeFilePath = g_strdup_printf("/tmp/audacious-info_%s.0",g_getenv ("USER"));
			if (! g_file_test (cInfopipeFilePath, G_FILE_TEST_EXISTS)) {
				cCommand = g_strdup_printf ("bash %s/infoaudacious.sh", MY_APPLET_SHARE_DATA_DIR);
				system (cCommand);
			}
		break ;
		case MY_BANSHEE :  //Le pipe est trop lent et cause des freezes... // Il faut émuler le pipe de banshee par le script
			cInfopipeFilePath = g_strdup_printf("/tmp/banshee-info_%s.0",g_getenv ("USER"));
			if (g_file_test (cInfopipeFilePath, G_FILE_TEST_EXISTS) == 0) {
				cCommand = g_strdup_printf ("bash %s/infobanshee.sh", MY_APPLET_SHARE_DATA_DIR);
				system (cCommand);
			}
		break ;
		case MY_EXAILE :  //Le pipe est trop lent, récupération des infos une fois sur deux avec un pique du cpu lors de l'éxécution du script // Il faut émuler le pipe d'audacious par Exaile -q
			cInfopipeFilePath = g_strdup_printf("/tmp/exaile-info_%s.0",g_getenv ("USER"));
			if (g_file_test (cInfopipeFilePath, G_FILE_TEST_EXISTS) == 0) {
				cCommand = g_strdup_printf ("bash %s/infoexaile.sh", MY_APPLET_SHARE_DATA_DIR);
				system (cCommand);
			}
		break ;
		default :
		break ;
	}
	g_free (cCommand);
	
	return cInfopipeFilePath;
}

//Fonction de lecture du tuyau.
void cd_xmms_read_pipe(gchar *cInfopipeFilePath) {
	gchar *cContent = NULL;
	gchar *cQuickInfo = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(cInfopipeFilePath, &cContent, &length, &erreur);
	if (erreur != NULL) {
		cd_warning("Attention : %s", erreur->message);
		g_error_free(erreur);
		myData.playingStatus = PLAYER_NONE;
	}
	else {
		gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
		g_free(cContent);
		gchar *cOneInfopipe;
		myData.iTrackNumber = -1;
		myData.iCurrentTime = -1;
		myData.iSongLength = -1;
		int *pLineNumber = s_pLineNumber[myConfig.iPlayer];
		int i;
		for (i = 0; cInfopipesList[i] != NULL; i ++) {
			cOneInfopipe = cInfopipesList[i];
			if (i == pLineNumber[INFO_STATUS]) {
				gchar *str = strchr (cOneInfopipe, ' ');
				if (str != NULL) {
					str ++;
					while (*str == ' ')
						str ++;
					if ((strcmp (str, "Playing") == 0) || (strcmp (str, "playing") == 0))
						myData.playingStatus = PLAYER_PLAYING;
					else if ((strcmp (str, "Paused") == 0) || (strcmp (str, "paused") == 0))
						myData.playingStatus = PLAYER_PAUSED;
					else if ((strcmp (str, "Stopped") == 0) || (strcmp (str, "stopped") == 0))
						myData.playingStatus = PLAYER_STOPPED;
					else
						myData.playingStatus = PLAYER_BROKEN;
				}
				else
					myData.playingStatus = PLAYER_BROKEN;
			}
			else if (i == pLineNumber[INFO_TRACK_IN_PLAYLIST]) {
				if (myConfig.quickInfoType == MY_APPLET_TRACK) {
					gchar *str = strchr (cOneInfopipe, ':');
					if (str != NULL) {
						str ++;
						while (*str == ' ')
							str ++;
						myData.iTrackNumber = atoi (str);
					}
				}
			}
			else if (i == pLineNumber[INFO_TIME_ELAPSED_IN_SEC]) {
				if (myConfig.quickInfoType == MY_APPLET_TIME_ELAPSED || myConfig.quickInfoType == MY_APPLET_TIME_LEFT) {
					gchar *str = strchr (cOneInfopipe, ' ');
					if (str != NULL) {
						str ++;
						while (*str == ' ')
							str ++;
						if (*str != 'N')
							myData.iCurrentTime = atoi(str) * 1e-3;
					}
				}
			}
			else if (i == pLineNumber[INFO_TIME_ELAPSED]) {
				if ((myConfig.quickInfoType == MY_APPLET_TIME_ELAPSED || myConfig.quickInfoType == MY_APPLET_TIME_LEFT) && myData.iCurrentTime == -1) {
					gchar *str = strchr (cOneInfopipe, ' ');
					if (str != NULL) {
						str ++;
						while (*str == ' ')
							str ++;
						gchar *str2 = strchr (str, ':');
						if (str2 == NULL) { // pas de minutes.
							myData.iCurrentTime = atoi(str);
						}
						else {
							*str2 = '\0';
							myData.iCurrentTime = atoi(str2+1) + 60*atoi (str);  // prions pour qu'ils n'ecrivent jamais les heures ... xD
						}
					}
				}
			}
			else if (i == pLineNumber[INFO_TOTAL_TIME_IN_SEC]) {
				if (myConfig.quickInfoType == MY_APPLET_TIME_LEFT) {
					gchar *str = strchr (cOneInfopipe, ' ');
					if (str != NULL) {
						str ++;
						while (*str == ' ')
							str ++;
						if (*str != 'N')
							myData.iSongLength = atoi(str) * 1e-3;
					}
				}
			}
			else if (i == pLineNumber[INFO_TOTAL_TIME]) {
				if (myConfig.quickInfoType == MY_APPLET_TIME_LEFT && myData.iSongLength == -1) {
					gchar *str = strchr (cOneInfopipe, ' ');
					if (str != NULL) {
						str ++;
						while (*str == ' ')
							str ++;
						gchar *str2 = strchr (str, ':');
						if (str2 == NULL) { // pas de minutes.
							myData.iSongLength = atoi(str);
						}
						else {
							*str2 = '\0';
							myData.iSongLength = atoi(str2+1) + 60*atoi (str);  // prions pour qu'ils n'ecrivent jamais les heures ...
						}
					}
				}
			}
			else if (i == pLineNumber[INFO_NOW_TITLE]) {
				gchar *str = strchr (cOneInfopipe, ':');
				if (str != NULL) {
					str ++;
					while (*str == ' ')
						str ++;
					if ((strcmp(str," (null)") != 0) && (myData.playingTitle == NULL || strcmp(str, myData.playingTitle) != 0)) {
						g_free (myData.playingTitle);
						myData.playingTitle = g_strdup (str);
						cd_message("On a changé de son! (%s)", myData.playingTitle);
					}
				}
			}
		}  // fin de parcours des lignes.
		g_strfreev (cInfopipesList);
	}
	cd_remove_pipes ();
}


//Fonction qui supprime les tuyaux émulés pour eviter des pics CPU
void cd_remove_pipes(void) {
	gchar *cInfopipeFilePath = NULL;
	switch (myConfig.iPlayer) {
		case MY_AUDACIOUS :
			cInfopipeFilePath = g_strdup_printf("/tmp/audacious-info_%s.0",g_getenv ("USER"));
		break;
		case MY_BANSHEE :
			cInfopipeFilePath = g_strdup_printf("/tmp/banshee-info_%s.0",g_getenv ("USER"));
		break;
		case MY_EXAILE :
			cInfopipeFilePath = g_strdup_printf("/tmp/exaile-info_%s.0",g_getenv ("USER"));
		break;
		default :  // xmms n'en a pas.
		return ;
	}
	g_remove (cInfopipeFilePath);
	g_free (cInfopipeFilePath);
	return ;
}
