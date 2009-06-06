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

typedef enum {
	MY_APPLET_PERSONNAL = 0,
	MY_APPLET_EXTENDED,
	MY_APPLET_CD_BOX,
	MY_APPLET_FRAME_REFLECTS,
	MY_APPLET_SCOTCH,
	MY_APPLET_FRAME_SCOTCH,
	MY_APPLET_NB_DECORATIONS
} MyAppletDecoration;

#define NB_TRANSITION_STEP 8.


struct _AppletConfig {
	gboolean enableDialogs;
	gboolean enableCover;
	gdouble timeDialogs;
	gchar *changeAnimation;
	MyAppletQuickInfoType quickInfoType;
	gchar *defaultTitle;
	gchar *cUserImage[PLAYER_NB_STATUS];
	gboolean bStealTaskBarIcon;
	//gboolean extendedDesklet;
	
	gboolean bOpenglThemes;
	gchar *cThemePath;
	} ;

struct _AppletData {
	cairo_surface_t *pSurfaces[PLAYER_NB_STATUS];
	cairo_surface_t *pCover;
	gboolean dbus_enable;
	gboolean bIsRunning;
	gboolean playing;
	gboolean cover_exist;
	gboolean b3dThemesDebugMode;
	int playing_duration;
	int playing_track;
	gchar *playing_uri;
	gchar *playing_artist;
	gchar *playing_album;
	gchar *playing_title;
	gchar *playing_cover;
	guint iSidCheckCover;
	
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
	
	// A passer en structure...
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
	gint iState;  // combinaison des etats des differents boutons.
	
	gboolean CoverWasDistant;  // a degager en testant sur la taille ...
	gint iAnimationCount;
	} ;


#endif
