#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-infopipe.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;

//Fonction qui definie quel tuyau a emprunter pour récupérer les infos
//Ajout d'une condition pour que temps que le tuyau n'est pas en place il n'y ai pas lecture de l'information
gboolean cd_xmms_get_pipe() {
  static gboolean bBusy = FALSE;
  
	if (bBusy)
		return TRUE;
	bBusy = TRUE;
	
	if (myDesklet != NULL) {
		myIcon->fWidth = MAX (1, myDesklet->iWidth - g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius/2;
		myIcon->fDrawY = g_iDockRadius/2;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;
	}
	
	gchar *cInfopipeFilePath;
	if (myConfig.cPlayer == MY_XMMS) {
    cInfopipeFilePath = g_strdup_printf("/tmp/xmms-info_%s.0",g_getenv ("USER"));
  }
  else if (myConfig.cPlayer == MY_AUDACIOUS) {
    //Il faut émuler le pipe d'audacious par AUDTOOL
    cInfopipeFilePath = g_strdup_printf("/tmp/audacious-info_%s.0",g_getenv ("USER"));
    
    if (g_file_test (cInfopipeFilePath, G_FILE_TEST_EXISTS) == 0) {
      GString *sScriptPath = g_string_new ("");
      g_string_printf (sScriptPath, "bash %s/infoaudacious.sh", MY_APPLET_SHARE_DATA_DIR);
      GError *erreur = NULL;
      g_spawn_command_line_async (sScriptPath->str, &erreur);
      if (erreur != NULL) {
	      cd_warning ("Attention : when trying to execute 'infoaudacious.sh", erreur->message);
        g_error_free (erreur);
	    }
    }
    
  }
  
  //Le pipe est trop lent et cause des freezes...
  else if (myConfig.cPlayer == MY_BANSHEE) {
    //Il faut émuler le pipe de banshee par le script
    cInfopipeFilePath = g_strdup_printf("/tmp/banshee-info_%s.0",g_getenv ("USER"));
    
    if (g_file_test (cInfopipeFilePath, G_FILE_TEST_EXISTS) == 0) {
      GString *sScriptPath = g_string_new ("");
      g_string_printf (sScriptPath, "bash %s/infobanshee.sh", MY_APPLET_SHARE_DATA_DIR);
      GError *erreur = NULL;
      g_spawn_command_line_async (sScriptPath->str, &erreur);
      if (erreur != NULL) {
  	    cd_warning ("Attention : when trying to execute 'infobanshee.sh", erreur->message);
        g_error_free (erreur);
	    }
    }
  }
  
  //Le pipe est trop lent, récupération des infos une fois sur deux avec un pique du cpu lors de l'éxécution du script
  else if (myConfig.cPlayer == MY_EXAILE) {
    //Il faut émuler le pipe d'audacious par Exaile -q
    cInfopipeFilePath = g_strdup_printf("/tmp/exaile-info_%s.0",g_getenv ("USER"));
    
    if (g_file_test (cInfopipeFilePath, G_FILE_TEST_EXISTS) == 0) {
      GString *sScriptPath = g_string_new ("");
      g_string_printf (sScriptPath, "bash %s/infoexaile.sh", MY_APPLET_SHARE_DATA_DIR);
      GError *erreur = NULL;
      g_spawn_command_line_async (sScriptPath->str, &erreur);
      if (erreur != NULL) {
	      cd_warning ("Attention : when trying to execute 'infoexaile.sh", erreur->message);
        g_error_free (erreur);
	    }
    }
  }
  
  //On verifie si le pipe est mal défini (s'il y a une erreur dans la conf avec current-player ou pas de pipe), on sort.
  if (cInfopipeFilePath == NULL) {
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
  
  bBusy = FALSE;
	return TRUE;
}

//Fonction de lecture du tuyau et d'affichage des informations
gboolean cd_xmms_read_pipe(gchar *cInfopipeFilePath) {
  static gboolean bBusy = FALSE;
  
	if (bBusy)
		return FALSE;
	bBusy = TRUE;
	
	if (myDesklet != NULL) {
		myIcon->fWidth = MAX (1, myDesklet->iWidth - g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius/2;
		myIcon->fDrawY = g_iDockRadius/2;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;
	}
	
  gchar *cContent = NULL;
	gsize length=0;
	GError *tmp_erreur = NULL;
	g_file_get_contents(cInfopipeFilePath, &cContent, &length, &tmp_erreur);
	if (tmp_erreur != NULL) {
		cd_message("Attention : %s\n", tmp_erreur->message);
		g_error_free(tmp_erreur);
		CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pSurface);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_AND_REDRAW(" ");
	}
	else {
	  gchar *cQuickInfo;
		gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
		g_free(cContent);
		gchar *cOneInfopipe;
		gchar **tcnt;
		int **icnt;
		gchar *titre;
		int uSecPos, uSecTime, timeLeft;
		int i = 0;
		cQuickInfo = " ";
		for (i = 0; cInfopipesList[i] != NULL; i ++) {
			cOneInfopipe = cInfopipesList[i];
			if (i == 2) {
			  tcnt = g_strsplit(cOneInfopipe," ", -1);
			  if ((strcmp(tcnt[1],"Playing") == 0) || (strcmp(tcnt[1],"playing") == 0)) {
			    CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pPlaySurface);
			  }
			  else if ((strcmp(tcnt[1],"Paused") == 0) || (strcmp(tcnt[1],"paused") == 0)) {
			    CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pPauseSurface);
			  }
			  else if ((strcmp(tcnt[1],"Stopped") == 0) || (strcmp(tcnt[1],"stopped") == 0)) {
			    CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pStopSurface);
			  }
			  else {
			    CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pBrokenSurface);
			  }
			}
			else if ((i == 4) && (myConfig.quickInfoType == MY_APPLET_TRACK)) {
				tcnt = g_strsplit(cOneInfopipe,":", -1);
				cQuickInfo = g_strdup_printf ("%s", tcnt[1]);
			}
			else if ((i == 5) && (myConfig.quickInfoType == MY_APPLET_TIME_LEFT)) {
				tcnt = g_strsplit(cOneInfopipe," ", -1);
			  uSecPos = atoi(tcnt[1])/1000;
			}
			else if ((i == 6) && (myConfig.quickInfoType == MY_APPLET_TIME_ELAPSED)) {
				tcnt = g_strsplit(cOneInfopipe," ", -1);
			  cQuickInfo = g_strdup_printf ("%s", tcnt[1]);
			}
			else if ((i == 7) && (myConfig.quickInfoType == MY_APPLET_TIME_LEFT)) {
				tcnt = g_strsplit(cOneInfopipe," ", -1);
			  uSecTime = atoi(tcnt[1])/1000;
			  timeLeft = uSecTime - uSecPos;
			  int min = timeLeft / 60;
				int sec = timeLeft % 60;
				cQuickInfo = g_strdup_printf ("%i:%.02d", min,sec);
			}
		  else if ((i == 8) && (myConfig.quickInfoType == MY_APPLET_TOTAL_TIME)) {
			  tcnt = g_strsplit(cOneInfopipe," ", -1);
			  cQuickInfo = g_strdup_printf ("%s", tcnt[1]);
			}
			else if (i == 12) {
			  tcnt = g_strsplit(cOneInfopipe,"e: ", -1);
			  CD_APPLET_SET_NAME_FOR_MY_ICON(tcnt[1]);
			  
			  titre = tcnt[1];
			  if (strcmp(titre,myData.playingTitle) != 0) {
			    cd_message("On a changé de son! %s\n",titre);
			    myData.playingTitle = titre;
			   if (myConfig.enableAnim) {
			      cd_xmms_animat_icon(1);
			    }
			    if (myConfig.enableDialogs) {
			      cd_xmms_new_song_playing();
			    }
			  } 
			  
			}
		}
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_AND_REDRAW(cQuickInfo);
  }
  cd_remove_pipes();
  bBusy = FALSE;
	return FALSE;
}

void cd_xmms_update_title() {
  cd_message("On met a jour le titre et le status de l'applet\n");
  cd_xmms_get_pipe();
}

//Fonction qui supprime les tuyaux émulés pour eviter des pics CPU
int cd_remove_pipes() {
  if (myConfig.cPlayer == MY_XMMS) {
    return 0;
  }
  GString *sScriptPath = g_string_new ("");
  gchar *cInfopipeFilePath;
  if (myConfig.cPlayer == MY_AUDACIOUS) {
    cInfopipeFilePath = g_strdup_printf("/tmp/audacious-info_%s.0",g_getenv ("USER"));
  }
  else if (myConfig.cPlayer == MY_BANSHEE) {
    cInfopipeFilePath = g_strdup_printf("/tmp/banshee-info_%s.0",g_getenv ("USER"));
  }
  else if (myConfig.cPlayer == MY_EXAILE) {
    cInfopipeFilePath = g_strdup_printf("/tmp/exaile-info_%s.0",g_getenv ("USER"));
  }
  g_string_printf (sScriptPath, "rm %s", cInfopipeFilePath);
  GError *erreur = NULL;
  g_spawn_command_line_async (sScriptPath->str, &erreur);
  if (erreur != NULL) {
	  cd_warning ("Attention : when trying to execute 'infobanshee.sh", erreur->message);
    g_error_free (erreur);
	}
	return 0;
}

//Fonction qui affiche la bulle au changement de musique
void cd_xmms_new_song_playing(void) {
	cairo_dock_show_temporary_dialog ("%s",	myIcon, myDock, myConfig.timeDialogs, myData.playingTitle);
}
//Fonction qui anime l'icone au changement de musique
void cd_xmms_animat_icon(int animationLength) {
	CD_APPLET_ANIMATE_MY_ICON (myConfig.changeAnimation, animationLength)
}
