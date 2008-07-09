
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef enum {
	MY_EXAILE = 0,
	MY_AUDACIOUS,
	MY_BANSHEE,
	MY_QUOD_LIBET,
	MY_LISTEN,
	MY_RHYTHMBOX,
	MY_NB_PLAYERS
} MyPlayerType;

typedef enum {
	MY_APPLET_NOTHING = 0,
	MY_APPLET_TIME_ELAPSED,
	MY_APPLET_TIME_LEFT,
	MY_APPLET_PERCENTAGE,
	MY_APPLET_TRACK,
	MY_APPLET_NB_QUICK_INFO_TYPE
	} MyAppletQuickInfoType;

typedef enum {
	PLAYER_NONE = 0,
	PLAYER_PLAYING,
	PLAYER_PAUSED,
	PLAYER_STOPPED,
	PLAYER_BROKEN,
	PLAYER_NB_STATUS
} MyAppletPlayerStatus;

typedef struct {
	gboolean enableDialogs;
	gboolean enableCover;
	gdouble timeDialogs;
	CairoDockAnimationType changeAnimation;
	MyPlayerType iPlayer;
	MyAppletQuickInfoType quickInfoType;
	gchar *defaultTitle;
	gchar *cUserImage[PLAYER_NB_STATUS];
	gboolean bStealTaskBarIcon;
	} AppletConfig;
	
typedef struct {
	gchar *service;
	gchar *path;
	gchar *interface;
	gchar *path2;
	gchar *interface2;
	gchar *play;
	gchar *pause;
	gchar *stop;
	gchar *next;
	gchar *previous;
	gchar *get_status;
	gchar *get_title;
	gchar *get_artist;
	gchar *get_album;
	gchar *get_cover_path;
	gchar *get_full_data;
	gchar *toggle;
	} AppletDBus;

typedef struct {
	cairo_surface_t *pSurfaces[PLAYER_NB_STATUS];
	cairo_surface_t *pCover;
	gchar *run_program;
	gboolean dbus_enable;
	gboolean opening;
	gboolean playing;
	gboolean paused;
	gboolean stopped;
	gint status;
	gint status_integer;
	gboolean data_have_changed;
	gboolean cover_exist;
	int percentage;
	gchar* total_length;
	int playing_track;
	int playing_duration;
	gchar *playing_uri;
	gchar *playing_artist;
	gchar *playing_album;
	gchar *playing_title;
	gchar *playing_cover;
	gchar *full_data;
	guint iSidCheckCover;
	AppletDBus DBus_commands;
	} AppletData;
	



#endif

