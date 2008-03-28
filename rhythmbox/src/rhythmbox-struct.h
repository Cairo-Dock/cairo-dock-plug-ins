#ifndef __RYTHMBOX_STRUCT__
#define  __RYTHMBOX_STRUCT__

#include <cairo-dock.h>

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
	MyAppletQuickInfoType quickInfoType;
	gchar *defaultTitle;
	gchar *cUserImage[PLAYER_NB_STATUS];
	gboolean bInhibateRhythmboxAppli;
	} AppletConfig;

typedef struct {
	cairo_surface_t *pSurfaces[PLAYER_NB_STATUS];
	cairo_surface_t *pCover;
	gboolean dbus_enable;
	gboolean opening;
	gboolean playing;
	gboolean cover_exist;
	int playing_duration;
	int playing_track;
	gchar *playing_uri;
	const gchar *playing_artist;
	const gchar *playing_album;
	const gchar *playing_title;
	gchar *playing_cover;
	guint iSidCheckCover;
	gboolean bAppliInhibitedByMe;
	} AppletData;


#endif
