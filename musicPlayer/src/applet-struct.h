
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

//Canevas
typedef struct _MusicPlayerHandeler MusicPlayerHandeler;

typedef enum {
	PLAYER_NONE = 0,
	PLAYER_PLAYING,
	PLAYER_PAUSED,
	PLAYER_STOPPED,
	PLAYER_BROKEN,
	PLAYER_NB_STATUS
} MyPlayerStatus;

typedef enum {
	PLAYER_PREVIOUS = 0,
	PLAYER_PLAY_PAUSE,
	PLAYER_STOP,
	PLAYER_NEXT,
	PLAYER_JUMPBOX,
	PLAYER_SHUFFLE,
	PLAYER_REPEAT,
	PLAYER_ENQUEUE,
	PLAYER_NB_CONTROL
} MyPlayerControl;

typedef void (*MusicPlayerAcquireDataFunc) (void);
typedef void (*MusicPlayerReadDataFunc) (void);
typedef void (*MusicPlayerFreeDataFunc) (void);
typedef void (*MusicPlayerConfigureFunc) (void);
typedef void (*MusicPlayerControlerFunc) (MyPlayerControl pControl, gchar *cFile);
typedef gboolean (*MusicPlayerAskControlerFunc) (MyPlayerControl pControl);

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
	MusicPlayerAcquireDataFunc 	acquisition;
	MusicPlayerReadDataFunc 		read_data;
	MusicPlayerFreeDataFunc 		free_data;
	MusicPlayerConfigureFunc		configure;
	MusicPlayerControlerFunc		control;
	MusicPlayerAskControlerFunc	ask_control;
	gchar *appclass;
	gchar *name; //Servira a repérer le lecteur dans la GList.
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

typedef enum {
	MY_DESKLET_SIMPLE = 0,
	MY_DESKLET_INFO,
	MY_DESKLET_INFO_AND_CONTROLER,
	MY_DESKLET_CAROUSSEL,
	MY_DESKLET_CONTROLER,
	MY_DESKLET_NB_MODE
} MyExtendedMode;

typedef enum {
	MY_APPLET_PERSONNAL = 0,
	MY_APPLET_EXTENDED,
	MY_APPLET_CD_BOX,
	MY_APPLET_FRAME_REFLECTS,
	MY_APPLET_SCOTCH,
	MY_APPLET_FRAME_SCOTCH,
	MY_APPLET_NB_DECORATIONS
} MyAppletDecoration;

struct _AppletConfig {
	gboolean bEnableDialogs;
	gdouble fTimeDialogs;
	gboolean bEnableCover;
	gboolean bEnableAnim;
	gchar cChangeAnimation;
	gchar *cMusicPlayer;
	MyAppletQuickInfoType pQuickInfoType;
	gchar *cDefaultTitle;
	gchar *cUserImage[PLAYER_NB_STATUS];
	gboolean bStealTaskBarIcon;
	gboolean bIconBubble;
	MyExtendedMode iExtendedMode;
	
	gboolean extendedDesklet;
	MyAppletDecoration iDecoration;
	int iLeftOffset, iTopOffset, iRightOffset, iBottomOffset;
	gchar *cFrameImage;
	gchar *cReflectImage;
	gdouble fFrameAlpha;
	gdouble fReflectAlpha;
};

struct _AppletData {
	//Pointeurs du Canevas
	CairoDockMeasure *pMeasureTimer;
	GList *pHandelers;
	MusicPlayerHandeler *pCurrentHandeler;
	
	//Informations essentielles
	gchar *cRawTitle, *cPreviousRawTitle;
	gchar *cTitle;
	gchar *cArtist;
	gchar *cAlbum;
	gchar *cCoverPath, *cPreviousCoverPath;
	MyPlayerStatus pPlayingStatus, pPreviousPlayingStatus;
	gint iTrackNumber, iPreviousTrackNumber;
	gint iCurrentTime, iPreviousCurrentTime;
	gint iSongLength;

	// Pour les lecteurs utilisant DBus
	MusicPlayerDBus DBus_commands;
	gboolean dbus_enable;
	gboolean opening;
	
	//Données de dessin
	cairo_surface_t *pSurfaces[PLAYER_NB_STATUS];
	cairo_surface_t *pCover;
	gchar *cQuickInfo, *cPreviousQuickInfo;
	
	guint iSidCheckCover;
};


#endif
