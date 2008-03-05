#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-infopipe.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;

gboolean cd_xmms_read_pipe() {
  static gboolean bBusy = FALSE;
  
	if (bBusy)
		return TRUE;
	bBusy = TRUE;
	
	gchar *cInfopipeFilePath;
	//cd_message("P: %d",myConfig.cPlayer);
	if (myConfig.cPlayer == MY_XMMS) {
    cInfopipeFilePath = g_strdup_printf("/tmp/xmms-info_%s.0",g_getenv ("USER"));
  }
  else if (myConfig.cPlayer == MY_AUDACIOUS) {
    //Il faut émuler le pipe d'audacious par AUDTOOL
    GString *sScriptPath = g_string_new ("");
    g_string_printf (sScriptPath, "bash %s/infoaudacious.sh", MY_APPLET_SHARE_DATA_DIR);
    GError *erreur = NULL;
    g_spawn_command_line_async (sScriptPath->str, &erreur);
    if (erreur != NULL) {
	    cd_warning ("Attention : when trying to execute 'infoaudacious.sh", erreur->message);
      g_error_free (erreur);
	  }
    
    cInfopipeFilePath = g_strdup_printf("/tmp/audacious-info_%s.0",g_getenv ("USER"));
  }
  
  //On verifie si un pipe est défini (s'il y a une erreur dans la conf avec current-player > 1)
  if (cInfopipeFilePath == NULL) {
   	CD_APPLET_SET_NAME_FOR_MY_ICON(myConfig.defaultTitle);
		CD_APPLET_SET_SURFACE_ON_MY_ICON(myData.pBrokenSurface);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(" ");
		CD_APPLET_REDRAW_MY_ICON;
		cd_message("No Pipe to read");
		return TRUE;
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
			//cd_message("Reading: %s\n",cOneInfopipe);
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
			/* else if ((i == 5) && (myConfig.quickInfoType == MY_APPLET_TIME_LEFT)) {
				tcnt = g_strsplit(cOneInfopipe,":", -1);
			  uSecPos = tcnt[1];
			} */
			else if ((i == 6) && (myConfig.quickInfoType == MY_APPLET_TIME_ELAPSED)) {
				tcnt = g_strsplit(cOneInfopipe," ", -1);
			  cQuickInfo = g_strdup_printf ("%s", tcnt[1]);
			}
			/* else if ((i == 7) && (myConfig.quickInfoType == MY_APPLET_TIME_LEFT)) {
				tcnt = g_strsplit(cOneInfopipe,":", -1);
			  uSecTime = tcnt[1];
			  timeLeft = uSecTime - uSecPos;
			  int min = timeLeft / 60;
				int sec = timeLeft % 60;
				cQuickInfo = g_strdup_printf ("%i:%.02d", min,sec);
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON(cQuickInfo); 
			  g_free (cQuickInfo);
			} */
		  else if ((i == 8) && (myConfig.quickInfoType == MY_APPLET_TOTAL_TIME)) {
			  tcnt = g_strsplit(cOneInfopipe," ", -1);
			  cQuickInfo = g_strdup_printf ("%s", tcnt[1]);
			}
			else if (i == 12) {
			  tcnt = g_strsplit(cOneInfopipe,"e:", -1);
			  CD_APPLET_SET_NAME_FOR_MY_ICON(tcnt[1]);
			  
			  /* titre = tcnt[1];
			  if (strcmp(titre,myData.playingTitle) != 0) {
			    //On remet en variable le titre
			    myData.playingTitle = titre;
			    //On anime l'icon
			    //CD_APPLET_ANIMATE_MY_ICON (myConfig.changeAnimation,1)
			    //On affiche la bulle
			  } */
			  
			}
		}
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_AND_REDRAW(cQuickInfo);
		g_free (cQuickInfo);
		g_free(cInfopipesList);
  }
  
  bBusy = FALSE;
	return TRUE;
}

void cd_xmms_update_title() {
  cd_message("On met a jour le titre et le status de l'applet\n");
  cd_xmms_read_pipe();
}
