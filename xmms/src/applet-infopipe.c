#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-infopipe.h"
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;

//Fonction qui definie quel tuyau a emprunter pour récupérer les infos
//Ajout d'une condition pour que tant que le tuyau n'est pas en place il n'y ait pas lecture de l'information
gboolean cd_xmms_get_pipe(gpointer data) {
	static gboolean bBusy = FALSE;
	if (bBusy)
		return TRUE;
	bBusy = TRUE;
	
	gchar *cInfopipeFilePath;
	switch (myConfig.iPlayer)
	{
		case MY_XMMS :
			cInfopipeFilePath = g_strdup_printf("/tmp/xmms-info_%s.0",g_getenv ("USER"));
		break ;
		case MY_AUDACIOUS :  //Il faut émuler le pipe d'audacious par AUDTOOL
			cInfopipeFilePath = g_strdup_printf("/tmp/audacious-info_%s.0",g_getenv ("USER"));
			
			if (! g_file_test (cInfopipeFilePath, G_FILE_TEST_EXISTS)) {
				gchar *cCommand = g_strdup_printf ("bash %s/infoaudacious.sh", MY_APPLET_SHARE_DATA_DIR);
				GError *erreur = NULL;
				g_spawn_command_line_async (cCommand, &erreur);
				if (erreur != NULL) {
					cd_warning ("Attention : when trying to execute 'infoaudacious.sh", erreur->message);
					g_error_free (erreur);
					g_free (cCommand);
				}
			}
		break ;
		case MY_BANSHEE :  //Le pipe est trop lent et cause des freezes... // Il faut émuler le pipe de banshee par le script
			cInfopipeFilePath = g_strdup_printf("/tmp/banshee-info_%s.0",g_getenv ("USER"));
			if (g_file_test (cInfopipeFilePath, G_FILE_TEST_EXISTS) == 0) {
				gchar *cCommand = g_strdup_printf ("bash %s/infobanshee.sh", MY_APPLET_SHARE_DATA_DIR);
				GError *erreur = NULL;
				g_spawn_command_line_async (cCommand, &erreur);
				if (erreur != NULL) {
					cd_warning ("Attention : when trying to execute 'infobanshee.sh", erreur->message);
					g_error_free (erreur);
				}
				g_free (cCommand);
			}
		break ;
		case MY_EXAILE :  //Le pipe est trop lent, récupération des infos une fois sur deux avec un pique du cpu lors de l'éxécution du script // Il faut émuler le pipe d'audacious par Exaile -q
			cInfopipeFilePath = g_strdup_printf("/tmp/exaile-info_%s.0",g_getenv ("USER"));
			if (g_file_test (cInfopipeFilePath, G_FILE_TEST_EXISTS) == 0) {
				gchar *cCommand = g_strdup_printf ("bash %s/infoexaile.sh", MY_APPLET_SHARE_DATA_DIR);
				GError *erreur = NULL;
				g_spawn_command_line_async (cCommand, &erreur);
				if (erreur != NULL) {
					cd_warning ("Attention : when trying to execute 'infobanshee.sh", erreur->message);
					g_error_free (erreur);
				}
				g_free (cCommand);
			}
		break ;
		default :  // ne devrait pas arriver.
			CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
			CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pBrokenSurface);
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(" ");
			CD_APPLET_REDRAW_MY_ICON;
			cd_message("No Pipe to read");
			bBusy = FALSE;
		return TRUE;
	}
	
	//Si le pipe n'existe pas, on sort. Evite les cascades d'éxécution du script et de lire des données fausses
	if (g_file_test (cInfopipeFilePath, G_FILE_TEST_EXISTS) == 0) {
		bBusy = FALSE;
		return TRUE;
	}
	
	cd_xmms_read_pipe(cInfopipeFilePath);
	g_free (cInfopipeFilePath);
	bBusy = FALSE;
	return TRUE;
}

//Fonction de lecture du tuyau et d'affichage des informations
gboolean cd_xmms_read_pipe(gchar *cInfopipeFilePath) {
	static gboolean bBusy = FALSE;
	if (bBusy)
		return FALSE;
	bBusy = TRUE;
	
	gchar *cContent = NULL;
	gchar *cQuickInfo = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(cInfopipeFilePath, &cContent, &length, &erreur);
	if (erreur != NULL) {
		cd_warning("Attention : %s", erreur->message);
		g_error_free(erreur);
		CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pSurface);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_AND_REDRAW(" ");
	}
	else {
		gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
		g_free(cContent);
		gchar *cOneInfopipe, *titre=NULL;  // **tcnt
		int uSecPos=0, uSecTime=0, timeLeft=0, i=0;
		cQuickInfo = " ";
		for (i = 0; cInfopipesList[i] != NULL; i ++) {
			cOneInfopipe = cInfopipesList[i];
			if (i == 2) {
				//tcnt = g_strsplit(cOneInfopipe," ", -1);
				gchar *str = strchr (cOneInfopipe, ' ');
				if (str != NULL)
				{
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
			else if ((i == 4) && (myConfig.quickInfoType == MY_APPLET_TRACK)) {
				//tcnt = g_strsplit(cOneInfopipe,":", -1);
				gchar *str = strchr (cOneInfopipe, ':');
				if (str != NULL)
				{
					cQuickInfo = g_strdup (str+1);
				}
			}
			else if ((i == 5) && (myConfig.quickInfoType == MY_APPLET_TIME_LEFT)) {
				//tcnt = g_strsplit(cOneInfopipe," ", -1);
				gchar *str = strchr (cOneInfopipe, ' ');
				if (str != NULL)
				{
					while (*str == ' ')
						str ++;
					uSecPos = atoi(str) * 1e-3;
				}
			}
			else if ((i == 6) && (myConfig.quickInfoType == MY_APPLET_TIME_ELAPSED)) {
				//tcnt = g_strsplit(cOneInfopipe," ", -1);
				gchar *str = strchr (cOneInfopipe, ' ');
				if (str != NULL)
				{
					while (*str == ' ')
						str ++;
					cQuickInfo = g_strdup (str);
				}
			}
			else if ((i == 7) && (myConfig.quickInfoType == MY_APPLET_TIME_LEFT)) {
				//tcnt = g_strsplit(cOneInfopipe," ", -1);
				gchar *str = strchr (cOneInfopipe, ' ');
				if (str != NULL)
				{
					while (*str == ' ')
						str ++;
					uSecTime = atoi(str) * 1e-3;
					timeLeft = uSecTime - uSecPos;
					int min = timeLeft / 60;
					int sec = timeLeft % 60;
					cQuickInfo = g_strdup_printf ("%d:%.02d", min,sec);
				}
			}
			else if ((i == 8) && (myConfig.quickInfoType == MY_APPLET_TOTAL_TIME)) {
				//tcnt = g_strsplit(cOneInfopipe," ", -1);
				gchar *str = strchr (cOneInfopipe, ' ');
				if (str != NULL)
				{
					while (*str == ' ')
						str ++;
					cQuickInfo = g_strdup (str);
				}
			}
			else if (i == 12) {
				//tcnt = g_strsplit(cOneInfopipe,"e: ", -1);
				//titre = tcnt[1];
				gchar *str = strchr (cOneInfopipe, 'e');
				if (str != NULL)
				{
					titre = str+1;
					if ((strcmp(titre,"(null)") != 0) && (myData.playingTitle == NULL || strcmp(titre, myData.playingTitle) != 0)) {
						myData.playingTitle = g_strdup (titre);
						cd_message("On a changé de son! %s",titre);
						if (myConfig.enableAnim) {
							cd_xmms_animate_icon(1);
						}
						if (myConfig.enableDialogs) {
							cd_xmms_new_song_playing();
						}
					}
				}
			}
		}
	}
	
	if (myDesklet != NULL) {
		if (myConfig.extendedDesklet)
			cd_xmms_draw_in_desklet(myDrawContext, cQuickInfo);
	}
	else {
		cd_xmms_draw_in_dock(cQuickInfo);
	}
	cd_remove_pipes();
	bBusy = FALSE;
	return FALSE;
}

void cd_xmms_update_title() {
	cd_message("On met a jour le titre et le status de l'applet");
	cd_xmms_get_pipe(NULL);
}

//Fonction qui supprime les tuyaux émulés pour eviter des pics CPU
void cd_remove_pipes() {
	gchar *cInfopipeFilePath = NULL;
	switch (myConfig.iPlayer)
	{
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

//Fonction qui affiche la bulle au changement de musique
void cd_xmms_new_song_playing(void) {
	cairo_dock_show_temporary_dialog ("%s", myIcon, myDock, myConfig.timeDialogs, myData.playingTitle);
}
//Fonction qui anime l'icone au changement de musique
void cd_xmms_animate_icon(int animationLength) {
	CD_APPLET_ANIMATE_MY_ICON (myConfig.changeAnimation, animationLength)
}
