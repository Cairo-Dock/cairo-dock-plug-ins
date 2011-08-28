/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib/gi18n.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-musicplayer.h"
#include "applet-draw.h"

#include "applet-xmms.h"


//Structure et données necessaires
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

static char  *s_cTmpFile = NULL;
static int s_pLineNumber[NB_INFO] = {2,4,5,6,7,8,12};

//Les Fonctions
static void cd_xmms_free_data (void) { //Permet de libéré la mémoire prise par notre controleur
	cd_debug ("");
	g_free (s_cTmpFile);
	s_cTmpFile = NULL;
}

static void cd_xmms_control (MyPlayerControl pControl, const gchar *cFile) { //Permet d'effectuer les actions de bases sur le lecteur
	GError *erreur = NULL;
	
	if (pControl != PLAYER_JUMPBOX && pControl != PLAYER_SHUFFLE && pControl != PLAYER_REPEAT && pControl != PLAYER_ENQUEUE) {
		g_free (myData.cRawTitle);
		myData.cRawTitle = NULL; //Reset the title to detect it for sure ;)
	}
	const gchar *cCommand = NULL;
	gchar *cCommand2 = NULL;
	
	switch (pControl) {
		case PLAYER_PREVIOUS :
			cCommand = "xmms -r";
		break;
		case PLAYER_PLAY_PAUSE :
			cCommand = "xmms -t";
		break;
		case PLAYER_STOP :
			cCommand = "xmms -s";
		break;
		case PLAYER_NEXT :
			cCommand = "xmms -f";
		break;
		case PLAYER_JUMPBOX :
			cCommand = "xmms -j";
		break;
		case PLAYER_SHUFFLE :
			cCommand = "xmms -S";
		break;
		case PLAYER_REPEAT :
			cCommand = "xmms -R";
		break;
		case PLAYER_ENQUEUE :
			if (cFile != NULL)
				cCommand2 = g_strdup_printf ("xmms -e %s", cFile);
		break;
	}
	
	cd_debug ("Handler XMMS: will use '%s'", cCommand?cCommand:cCommand2);
	g_spawn_command_line_async (cCommand?cCommand:cCommand2, &erreur);
	g_free (cCommand2);
	
	if (erreur != NULL) {
		cd_warning ("MP : when trying to execute command : %s", erreur->message);
		g_error_free (erreur);
		//CD_APPLET_MAKE_TEMPORARY_EMBLEM_CLASSIC (CAIRO_DOCK_EMBLEM_ERROR, CAIRO_DOCK_EMBLEM_UPPER_LEFT, 5000);
	}
}

//Fonction de lecture du tuyau.
static void cd_xmms_read_data (void) {
	s_cTmpFile = g_strdup_printf("/tmp/xmms-info_%s.0",g_getenv ("USER"));
		
	gchar *cContent = NULL;
	gchar *cQuickInfo = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents (s_cTmpFile, &cContent, &length, &erreur);
	if (erreur != NULL) {
		cd_warning ("MP : %s", erreur->message);
		g_error_free (erreur);
		myData.iPlayingStatus = PLAYER_NONE;
		//cd_musicplayer_player_none ();
	}
	else {
		gchar **cInfopipesList = g_strsplit(cContent, "\n", -1);
		g_free (cContent);
		gchar *cOneInfopipe;
		myData.iTrackNumber = -1;
		myData.iCurrentTime = -1;
		myData.iSongLength = -1;
		int i;
		for (i = 0; cInfopipesList[i] != NULL; i ++) {
			cOneInfopipe = cInfopipesList[i];
			if (i == s_pLineNumber[INFO_STATUS]) {
				gchar *str = strchr (cOneInfopipe, ' ');
				if (str != NULL) {
					str ++;
					while (*str == ' ')
						str ++;
					if ((strcmp (str, "Playing") == 0) || (strcmp (str, "playing") == 0))
						myData.iPlayingStatus = PLAYER_PLAYING;
					else if ((strcmp (str, "Paused") == 0) || (strcmp (str, "paused") == 0))
						myData.iPlayingStatus = PLAYER_PAUSED;
					else if ((strcmp (str, "Stopped") == 0) || (strcmp (str, "stopped") == 0))
						myData.iPlayingStatus = PLAYER_STOPPED;
					else
						myData.iPlayingStatus = PLAYER_BROKEN;
				}
				else
					myData.iPlayingStatus = PLAYER_BROKEN;
			}
			else if (i == s_pLineNumber[INFO_TRACK_IN_PLAYLIST]) {
				if (myConfig.iQuickInfoType == MY_APPLET_TRACK) {
					gchar *str = strchr (cOneInfopipe, ':');
					if (str != NULL) {
						str ++;
						while (*str == ' ')
							str ++;
						myData.iTrackNumber = atoi (str);
					}
				}
			}
			else if (i == s_pLineNumber[INFO_TIME_ELAPSED_IN_SEC]) {
				if (myConfig.iQuickInfoType == MY_APPLET_TIME_ELAPSED || myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT) {
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
			else if (i == s_pLineNumber[INFO_TIME_ELAPSED]) {
				if ((myConfig.iQuickInfoType == MY_APPLET_TIME_ELAPSED || myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT) && myData.iCurrentTime == -1) {
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
			else if (i == s_pLineNumber[INFO_TOTAL_TIME_IN_SEC]) {
				if (myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT) {
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
			else if (i == s_pLineNumber[INFO_TOTAL_TIME]) {
				if (myConfig.iQuickInfoType == MY_APPLET_TIME_LEFT && myData.iSongLength == -1) {
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
			else if (i == s_pLineNumber[INFO_NOW_TITLE]) {
				gchar *str = strchr (cOneInfopipe, ':');
				if (str != NULL) {
					str ++;
					while (*str == ' ')
						str ++;
					if ((strcmp(str, "(null)") != 0) && (myData.cRawTitle == NULL || strcmp(str, myData.cRawTitle) != 0)) {
						g_free (myData.cRawTitle);
						myData.cRawTitle = g_strdup (str);
						cd_message ("On a changé de son! (%s)", myData.cRawTitle);
						//cd_musicplayer_change_desklet_data();
					}
				}
			}
		}  // fin de parcours des lignes.
		g_strfreev (cInfopipesList);
	}
	
	g_free (s_cTmpFile);
	s_cTmpFile = NULL;
}

void cd_musicplayer_register_xmms_handler (void) { //On enregistre notre lecteurs
	MusicPlayerHandler *pXMMS = g_new0 (MusicPlayerHandler, 1);
	pXMMS->read_data = cd_xmms_read_data;
	pXMMS->free_data = cd_xmms_free_data;
	pXMMS->configure = NULL; //Cette fonction permettera de préparer le controleur
	//Pour les lecteurs utilisants dbus, c'est elle qui connectera le dock aux services des lecteurs etc..
	pXMMS->control = cd_xmms_control;
	pXMMS->appclass = "xmms";
	pXMMS->name = "XMMS";
	pXMMS->iPlayerControls = PLAYER_PREVIOUS | PLAYER_PLAY_PAUSE | PLAYER_NEXT | PLAYER_STOP | PLAYER_JUMPBOX | PLAYER_SHUFFLE | PLAYER_ENQUEUE | PLAYER_REPEAT;
	pXMMS->launch = "xmms";
	pXMMS->bSeparateAcquisition = TRUE;
	cd_musicplayer_register_my_handler (pXMMS);
}
