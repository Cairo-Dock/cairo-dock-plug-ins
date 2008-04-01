
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef enum {
	MY_APPLET_NOTHING = 0,
	MY_APPLET_TIME_ELAPSED,
	MY_APPLET_TIME_LEFT,
	MY_APPLET_TRACK,
	MY_APPLET_NB_QUICK_INFO_TYPE
} MyAppletQuickInfoType;

typedef enum {
	MY_XMMS = 0,
	MY_AUDACIOUS,
	MY_BANSHEE,
	MY_EXAILE,
	MY_NB_PLAYERS
} MyPlayerType;

typedef enum {
	PLAYER_NONE = 0,
	PLAYER_PLAYING,
	PLAYER_PAUSED,
	PLAYER_STOPPED,
	PLAYER_BROKEN,
	PLAYER_NB_STATUS
} MyPlayerStatus;

typedef struct {
	gchar *defaultTitle;
	gboolean enableDialogs;
	gboolean extendedDesklet;
	gdouble timeDialogs;
	gboolean enableAnim;
	CairoDockAnimationType changeAnimation;
	MyAppletQuickInfoType quickInfoType;
	gchar *cUserImage[PLAYER_NB_STATUS];
	MyPlayerType iPlayer;
	gboolean bStealTaskBarIcon;
} AppletConfig;

typedef struct {
	cairo_surface_t *pSurfaces[PLAYER_NB_STATUS];
	gchar *playingTitle, *previousPlayingTitle;
	MyPlayerStatus playingStatus, previousPlayingStatus;
	gint iTrackNumber, iPreviousTrackNumber;
	gint iCurrentTime, iPreviousCurrentTime;
	gint iSongLength;
	guint pipeTimer;
} AppletData;

#endif
