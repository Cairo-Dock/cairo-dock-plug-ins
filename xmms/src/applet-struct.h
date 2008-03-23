
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef enum {
	MY_APPLET_NOTHING = 0,
	MY_APPLET_TIME_ELAPSED,
	MY_APPLET_TIME_LEFT,
	MY_APPLET_TOTAL_TIME,
	MY_APPLET_TRACK,
	MY_APPLET_NB_QUICK_INFO_TYPE
} MyAppletQuickInfoType;

typedef enum {
	MY_XMMS = 0,
	MY_AUDACIOUS,
	MY_BANSHEE,
	MY_EXAILE,
} MyPlayerType;

typedef enum {
  sPlayerPlaying = 0,
  sPlayerPaused,
  sPlayerStopped,
  sPlayerBroken,
  sPlayerNone,
} MyPlayerStatus;

typedef struct {
	gboolean enableDialogs;
	gboolean extendedDesklet;
	gdouble timeDialogs;
	gboolean enableAnim;
	CairoDockAnimationType changeAnimation;
	MyAppletQuickInfoType quickInfoType;
	gchar *defaultTitle;
	gchar *cDefaultIcon;
	gchar *cPlayIcon;
	gchar *cPauseIcon;
	gchar *cStopIcon;
	gchar *cBrokenIcon;
	MyPlayerType cPlayer;
} AppletConfig;

typedef struct {
  cairo_surface_t *pSurface;
	cairo_surface_t *pPlaySurface;
	cairo_surface_t *pPauseSurface;
	cairo_surface_t *pStopSurface;
	cairo_surface_t *pCover;
	cairo_surface_t *pBrokenSurface;
  gchar *playingTitle;
  MyPlayerStatus playingStatus;
  gchar *lastQuickInfo;
  int pipeTimer;
} AppletData;

#endif
