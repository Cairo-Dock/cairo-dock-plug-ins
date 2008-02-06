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


typedef struct {
	gboolean enableDialogs;
	gboolean enableCover;
	gdouble timeDialogs;
	CairoDockAnimationType changeAnimation;
	MyAppletQuickInfoType quickInfoType;
	gchar *defaultTitle;
	gchar *cDefaultIcon;
	gchar *cPlayIcon;
	gchar *cPauseIcon;
	gchar *cStopIcon;
	gchar *cBrokenIcon;
	} AppletConfig;

typedef struct {
	cairo_surface_t *pSurface;
	cairo_surface_t *pPlaySurface;
	cairo_surface_t *pPauseSurface;
	cairo_surface_t *pStopSurface;
	cairo_surface_t *pCover;
	cairo_surface_t *pBrokenSurface;
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
	} AppletData;


#endif
