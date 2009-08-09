
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

//Canevas
typedef struct _MusicPlayerHandeler MusicPlayerHandeler;

// Players supportes
typedef enum {
	MP_AMAROK1 = 0,
	MP_AMAROK2,
	MP_RHYTHMBOX,
	MP_EXAILE,
	MP_LISTEN,
	MP_XMMS,
	MP_SONGBIRD,
	MP_QUODLIBET,
	MP_BANSHEE,
	MB_NB_PLAYERS
} MySupportedPlayers;

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
} MyPlayerControl;

typedef enum {
	PLAYER_BAD=0,  // aucune notification, il faut tout tester en permanence
	PLAYER_GOOD,  // notification de changement d'etat et de chanson, mais pas de temps.
	PLAYER_EXCELLENT,  // notification pour chaque evenement => aucune boucle n'est necessaire.
	PLAYER_NB_LEVELS
} MyLevel;  // niveau du lecteur.

typedef void (*MusicPlayerAcquireDataFunc) (void);  // obsolete
typedef void (*MusicPlayerReadDataFunc) (void);  // acquisition des donnees, threade.
typedef void (*MusicPlayerFreeDataFunc) (void);  // libere les ressources specifiques au backend (deconnexion des signaux, etc)
typedef void (*MusicPlayerConfigureFunc) (void);  // initialise le backend (connexion des signaux, etc)
typedef void (*MusicPlayerControlerFunc) (MyPlayerControl pControl, const gchar *cFile);  // controle du lecteur (play/pause/next/etc)
typedef void (*MusicPlayerGetCoverFunc) (void);  // pour les lecteurs buggues, recupere la couverture. Renseigner ce champ fera que si le lecteur n'a pas renvoye de couverture au changement de chanson, on retentera 2 secondes plus tard.

//A remplir lors du configure pour les players utilisant DBus.
typedef struct {
	gchar *service;
	gchar *path;
	gchar *interface;
	gchar *path2;
	gchar *interface2;
	gchar *play;
	gchar *pause;
	gchar *play_pause;
	gchar *stop;
	gchar *next;
	gchar *previous;
	gchar *get_status;
	gchar *get_title;
	gchar *get_artist;
	gchar *get_album;
	gchar *get_cover_path;
	gchar *duration;
	gchar *current_position;
} MusicPlayerDBus;

struct _MusicPlayerHandeler {
	MusicPlayerAcquireDataFunc 		acquisition;
	MusicPlayerReadDataFunc 		read_data;
	MusicPlayerFreeDataFunc 		free_data;
	MusicPlayerConfigureFunc		configure;
	MusicPlayerControlerFunc		control;
	MusicPlayerGetCoverFunc			get_cover;
	gchar *appclass;  // classe de l'appli.
	gchar *name;  // nom du backend.
	gchar *launch;  // commande lancant le lecteur.
	gchar *cCoverDir;  // repertoire utilisateur de l'appli, contenant les couvertures.
	gboolean bSeparateAcquisition;  // Sert a activer le thread ou pas (TRUE = activé; False = désactivé)
	MySupportedPlayers iPlayer;  // ID du backend.
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
};

struct _AppletData {
	//Pointeurs du canvas
	CairoDockTask *pTask;
	GList *pHandelers;
	MusicPlayerHandeler *pCurrentHandeler;
	
	//Informations essentielles
	DBusGProxy *dbus_proxy_player;
	DBusGProxy *dbus_proxy_shell;
	gchar *cRawTitle, *cPreviousRawTitle; 
	gchar *cTitle;
	gchar *cArtist;
	gchar *cAlbum;
	gchar* cPlayingUri;
	MyPlayerStatus pPlayingStatus, pPreviousPlayingStatus;
	gint iTrackNumber, iPreviousTrackNumber;
	gint iCurrentTime, iPreviousCurrentTime;
	gint iSongLength;
	gint iPreviousuValue;

	// Pour les lecteurs utilisant DBus
	MusicPlayerDBus DBus_commands;
	gboolean dbus_enable;
	gboolean dbus_enable_shell;
	gboolean bIsRunning;
	
	//Données de dessin
	cairo_surface_t *pSurfaces[PLAYER_NB_STATUS];
	cairo_surface_t *pCover;
	
	// Les pochettes
	gchar *cCoverPath, *cPreviousCoverPath;
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
