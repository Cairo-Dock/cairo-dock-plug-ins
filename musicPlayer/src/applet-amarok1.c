/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey (fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib/gi18n.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-musicplayer.h"
#include "applet-draw.h"
#include "applet-amarok1.h"
#include "applet-dcop.h"
#include "applet-cover.h"


//Les Fonctions
void cd_amarok1_free_data (void) { //Permet de libéré la mémoire prise par notre controleur
	cd_debug ("");
}

void cd_amarok1_control (MyPlayerControl pControl, gchar *cFile) { //Permet d'effectuer les actions de bases sur le lecteur
	GError *erreur = NULL;
	
	if (pControl != PLAYER_JUMPBOX && pControl != PLAYER_SHUFFLE && pControl != PLAYER_REPEAT && pControl != PLAYER_ENQUEUE) {
		g_free (myData.cRawTitle);
		myData.cRawTitle = NULL; //Reset the title to detect it for sure ;)
	}
	gchar *cCommand = NULL;
	
	switch (pControl) {
		case PLAYER_PREVIOUS :
			cCommand = "dcop amarok player prev";
		break;
		case PLAYER_PLAY_PAUSE :
			cCommand = "dcop amarok player playPause";
		break;
		case PLAYER_STOP :
			cCommand = "dcop amarok player stop";
		break;
		case PLAYER_NEXT :
			cCommand = "dcop amarok player next";
		break;
		case PLAYER_SHUFFLE :
			cCommand = "dcop amarok playlist shufflePlaylist";
		break;
		case PLAYER_REPEAT :
			cCommand = g_strdup_printf("dcop amarok player enableRepeatPlaylist %s",
										cd_dcop_get_boolean("dcop amarok player repeatPlaylistStatu") ?
										"true" : "false");
			/* 
			 * recuperer le boolean "dcop amarok player repeatPlaylistStatus"
			 * puis lancer : "dcop amarok player enableRepeatPlaylist false/true"
			 */
		break;
		case PLAYER_ENQUEUE :
			if (cFile != NULL)
				cCommand = g_strdup_printf ("dcop amarok playlist addMediaList [ \"%s\" ]", cFile);
		break;
		default :
			return;
		break;
	}
	
	cd_debug ("Handeler Amarok 1.4: will use '%s'", cCommand);
	g_spawn_command_line_async (cCommand, &erreur);
	if (pControl == PLAYER_ENQUEUE)
		g_free (cCommand);
	
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute command : %s", erreur->message);
		g_error_free (erreur);
		CD_APPLET_MAKE_TEMPORARY_EMBLEM_CLASSIC (CAIRO_DOCK_EMBLEM_ERROR, CAIRO_DOCK_EMBLEM_UPPER_LEFT, 5000);
	}
}

/*Permet de renseigner l'applet des fonctions supportées par le lecteur*/
gboolean cd_amarok1_ask_control (MyPlayerControl pControl) {
	cd_debug ("");
	switch (pControl) {
		case PLAYER_PREVIOUS :
			return TRUE;
		break;
		
		case PLAYER_PLAY_PAUSE :
			return TRUE;		
		break;

		case PLAYER_NEXT :
			return TRUE;
		break;
		
		case PLAYER_STOP :
			return TRUE;
			
		case PLAYER_SHUFFLE :
			return TRUE;
			
		case PLAYER_ENQUEUE :
			return TRUE;
			
		case PLAYER_REPEAT :
			return TRUE;
			
		default :
			return FALSE;
		break;
	}
	
	return FALSE;
}

void cd_amarok1_getStatus (void)
{
	int status = cd_dcop_get_int("dcop amarok player status");
		
	cd_debug("MP : Status : %i",status);
	switch (status) 
	{
		case 0:
			myData.pPlayingStatus = PLAYER_STOPPED;
			break;
		case 1:
			myData.pPlayingStatus = PLAYER_PAUSED;
			break;
		case 2: 
			myData.pPlayingStatus = PLAYER_PLAYING;
			break;
		default:
			break;
	}
}

void cd_amarok1_acquisition (void) {
	system ("echo amarok 1.4 >> /dev/null");
	cd_amarok1_getStatus();
	if (myData.pPlayingStatus == PLAYER_PLAYING)
	{	
		cd_amarok1_read_data();
	}
}

//Fonction de lecture du tuyau.
void cd_amarok1_read_data (void) {
	myData.iPreviousCurrentTime = myData.iCurrentTime;
	myData.pPreviousPlayingStatus=myData.pPlayingStatus;
	myData.iPreviousTrackNumber=myData.iTrackNumber;
	myData.iPreviousCurrentTime=myData.iCurrentTime;
	
	myData.cAlbum = cd_dcop_get_string("dcop amarok player album");
	myData.cArtist = cd_dcop_get_string("dcop amarok player artist");
	myData.cTitle = cd_dcop_get_string("dcop amarok player title");
	myData.iTrackNumber = cd_dcop_get_int("dcop amarok player trackPlayCounter");
	myData.iSongLength = cd_dcop_get_int("dcop amarok player trackTotalTime");
	myData.iCurrentTime = cd_dcop_get_int("dcop amarok player trackCurrentTime");
	myData.cCoverPath = cd_dcop_get_string("dcop amarok player coverImage"); // On recupere l'URI et on verifie que l'image existe et n'est pas l'image par defaut d'amarok
	
	myData.cRawTitle = g_strdup_printf ("%s - %s", myData.cArtist, myData.cTitle);
}

void cd_musicplayer_register_amarok1_handeler (void) { //On enregistre notre lecteurs
	MusicPlayerHandeler *pAmarok1 = g_new0 (MusicPlayerHandeler, 1);
	pAmarok1->acquisition = cd_amarok1_acquisition;
	pAmarok1->read_data = cd_amarok1_read_data;
	pAmarok1->free_data = cd_amarok1_free_data;
	pAmarok1->configure = NULL; //Cette fonction permettera de préparé le controleur
	//Pour les lecteurs utilisants dbus, c'est elle qui connectera le dock aux services des lecteurs etc..
	pAmarok1->control = cd_amarok1_control;
	pAmarok1->ask_control = cd_amarok1_ask_control;
	pAmarok1->appclass = g_strdup("amarok"); //Toujours g_strdup sinon l'applet plante au free_handler
	pAmarok1->name = g_strdup("Amarok 1.4");
	pAmarok1->launch = NULL;
	pAmarok1->iPlayer = MP_AMAROK1;
	cd_musicplayer_register_my_handeler (pAmarok1, "Amarok 1.4");
}
