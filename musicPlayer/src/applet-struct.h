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

#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

//Canvas
typedef struct _MusicPlayerHandler MusicPlayerHandler;

typedef enum {
	PLAYER_NONE = 0,
	PLAYER_PLAYING,
	PLAYER_PAUSED,
	PLAYER_STOPPED,
	PLAYER_BROKEN,
	PLAYER_NB_STATUS
} MyPlayerStatus;

typedef enum {
	PLAYER_PREVIOUS		= 1<<0,
	PLAYER_PLAY_PAUSE	= 1<<1,
	PLAYER_STOP			= 1<<2,
	PLAYER_NEXT			= 1<<3,
	PLAYER_JUMPBOX		= 1<<4,
	PLAYER_SHUFFLE		= 1<<5,
	PLAYER_REPEAT		= 1<<6,
	PLAYER_ENQUEUE		= 1<<7,
	PLAYER_RATE			= 1<<8,
} MyPlayerControl;

typedef enum {
	PLAYER_BAD=0,  // aucune notification, il faut tout tester en permanence.
	PLAYER_GOOD,  // notification de changement d'etat et de chanson, mais pas de temps => il faut une boucle seulement pour afficher le temps ecoule.
	PLAYER_EXCELLENT,  // notification pour chaque evenement => aucune boucle n'est necessaire.
	PLAYER_NB_LEVELS
} MyLevel;  // niveau du lecteur.


typedef void (*MusicPlayerGetDataFunc) (void);  // acquisition des donnees, threade.
typedef void (*MusicPlayerStopFunc) (void);  // libere les ressources specifiques au backend (deconnexion des signaux, etc)
typedef void (*MusicPlayerStartFunc) (void);  // initialise le backend (connexion des signaux, etc)
typedef void (*MusicPlayerControlerFunc) (MyPlayerControl pControl, const gchar *cFile);  // controle du lecteur (play/pause/next/etc)
typedef void (*MusicPlayerGetCoverFunc) (void);  // pour les lecteurs buggues, recupere la couverture. Renseigner ce champ fera que si le lecteur n'a pas renvoye de couverture au changement de chanson, on retentera 2 secondes plus tard avec cette fonction.

struct _MusicPlayerHandler {
	const gchar *name;  // nom du backend.
	MusicPlayerGetDataFunc 		get_data;
	MusicPlayerStopFunc 		stop;
	MusicPlayerStartFunc		start;
	MusicPlayerControlerFunc	control;
	MusicPlayerGetCoverFunc		get_cover;
	const gchar *cMprisService;  // old Dbus service name
	const gchar *path;  // Player object
	const gchar *interface;
	const gchar *path2;  // TrackList object.
	const gchar *interface2;
	const gchar *appclass;  // classe de l'appli.
	const gchar *launch;  // commande lancant le lecteur.
	gchar *cCoverDir;  // repertoire utilisateur de l'appli, contenant les couvertures.
	gboolean bSeparateAcquisition;  // Sert a activer le thread ou pas (TRUE = active; False = desactive)
	MyPlayerControl iPlayerControls;  // un masque "OU" de MyPlayerControl.
	MyLevel iLevel;
};

//Structures essentielles de l'applet
typedef enum {
	MY_APPLET_NOTHING = 0,
	MY_APPLET_TIME_ELAPSED,
	MY_APPLET_TIME_LEFT,
	MY_APPLET_PERCENTAGE,
	MY_APPLET_TRACK,
	MY_APPLET_NB_QUICK_INFO_TYPE
} MyAppletQuickInfoType;

#define NB_TRANSITION_STEP 8.

#define MP_DBUS_TYPE_SONG_METADATA (dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE))

#define CD_MPRIS2_SERVICE_BASE "org.mpris.MediaPlayer2"
#define CD_MPRIS2_OBJ /org/mpris/MediaPlayer2
#define CD_MPRIS2_MAIN_IFACE org.mpris.MediaPlayer2

struct _AppletConfig {
	gboolean bEnableDialogs;
	gint iDialogDuration;
	gboolean bEnableCover;
	gboolean bEnableAnim;
	gchar *cChangeAnimation;
	gchar *cMusicPlayer;
	MyAppletQuickInfoType iQuickInfoType;
	gchar *cDefaultTitle;
	gchar *cUserImage[PLAYER_NB_STATUS];
	gboolean bStealTaskBarIcon;
	
	gboolean bDownload;
	gint iTimeToWait;
	gchar *cThemePath;
	gboolean bOpenglThemes;
	
	gboolean bPauseOnClick;
};

struct _AppletData {
	// general
	CairoDockTask *pTask;
	GList *pHandlers;
	MusicPlayerHandler *pCurrentHandler;
	gchar *cMpris2Service;  // MPRIS2 service associated with the current handler.
	
	//Informations essentielles
	DBusGProxy *dbus_proxy_player;
	DBusGProxy *dbus_proxy_shell;
	gchar *cRawTitle, *cPreviousRawTitle; 
	gchar *cTitle;
	gchar *cArtist;
	gchar *cAlbum;
	gchar* cPlayingUri;
	gchar* cTrackID;
	MyPlayerStatus iPlayingStatus, pPreviousPlayingStatus;
	gint iTrackNumber, iPreviousTrackNumber;  // track number = position dans la play-list, et non pas numero de piste dans l'album (qui ne nous interesse pas).
	gint iCurrentTime, iPreviousCurrentTime, iGetTimeFailed;
	gint iSongLength;
	gint iRating;
	gint iTrackListLength;
	gint iTrackListIndex;
	
	// Pour les lecteurs utilisant DBus
	gboolean bIsRunning;
	DBusGProxyCall *pDetectPlayerCall;
	
	//Donnees de dessin
	cairo_surface_t *pSurfaces[PLAYER_NB_STATUS];
	cairo_surface_t *pCover;
	
	// Les pochettes
	gchar *cCoverPath, *cPreviousCoverPath, *cMissingCover;
	gint iSidGetCoverInfoTwice;
	guint iSidCheckCover;
	gint iNbCheckFile;
	guint iSidCheckXmlFile;
	gint iCurrentFileSize;
	gchar *cCurrentXmlFile;
	gboolean cover_exist;
	gboolean bCoverNeedsTest;
	
	// pochette 3D
	gint iCoverTransition;
	GLuint iPrevTextureCover;
	GLuint TextureFrame;
	GLuint TextureCover;
	GLuint TextureReflect;
	
	gdouble itopleftX;
	gdouble itopleftY;
	gdouble ibottomleftX;
	gdouble ibottomleftY;
	gdouble ibottomrightX;
	gdouble ibottomrightY;
	gdouble itoprightX;
	gdouble itoprightY;
	GLuint draw_cover;  // calllist
	
	gint numberButtons;
	gboolean osd;
	/// A passer en structure...
	gboolean mouseOnButton1;
	GLuint TextureButton1;
	gdouble button1coordX, button1coordY;
	gdouble button1sizeX, button1sizeY;
	gint iButton1Count;
	GLuint TextureOsdPlay;
	gdouble osdPlaycoordX, osdPlaycoordY;
	gdouble osdPlaysizeX, osdPlaysizeY;
	GLuint TextureOsdPause;
	gdouble osdPausecoordX, osdPausecoordY;
	gdouble osdPausesizeX, osdPausesizeY;
	
	gboolean mouseOnButton2;
	GLuint TextureButton2;
	gdouble button2coordX, button2coordY;
	gdouble button2sizeX, button2sizeY;
	gint iButton2Count;
	GLuint TextureOsdPrev;
	gdouble osdPrevcoordX, osdPrevcoordY;
	gdouble osdPrevsizeX, osdPrevsizeY;
	
	gboolean mouseOnButton3;
	GLuint TextureButton3;
	gdouble button3coordX, button3coordY;
	gdouble button3sizeX, button3sizeY;
	gint iButton3Count;
	GLuint TextureOsdNext;
	gdouble osdNextcoordX, osdNextcoordY;
	gdouble osdNextsizeX, osdNextsizeY;
	
	gboolean mouseOnButton4;
	GLuint TextureButton4;
	gdouble button4coordX, button4coordY;
	gdouble button4sizeX, button4sizeY;
	gint iButton4Count;
	GLuint TextureOsdHome;
	gdouble osdHomecoordX, osdHomecoordY;
	gdouble osdHomesizeX, osdHomesizeY;
	
	gint iMouseX;
	gint iMouseY;
	gint iButtonState;  // combinaison des etats des differents boutons.
};


#endif
