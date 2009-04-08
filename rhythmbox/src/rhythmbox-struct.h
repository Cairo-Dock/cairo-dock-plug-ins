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


struct _AppletConfig {
	gboolean enableDialogs;
	gboolean enableCover;
	gdouble timeDialogs;
	gchar *changeAnimation;
	MyAppletQuickInfoType quickInfoType;
	gchar *defaultTitle;
	gchar *cUserImage[PLAYER_NB_STATUS];
	gboolean bStealTaskBarIcon;
	gboolean extendedDesklet;
	gint iDeskletWidth;
	gint iDeskletHeight;

	gchar *cThemePath;
	} ;

struct _AppletData {
	cairo_surface_t *pSurfaces[PLAYER_NB_STATUS];
	cairo_surface_t *pCover;
	gboolean dbus_enable;
	gboolean opening;
	gboolean playing;
	gboolean cover_exist;
	int playing_duration;
	int playing_track;
	gchar *playing_uri;
	gchar *playing_artist;
	gchar *playing_album;
	gchar *playing_title;
	gchar *playing_cover;
	guint iSidCheckCover;
	
	GLuint TextureName;
	GLuint TextureFrame;
	GLuint TextureCover;
	GLuint TextureReflect;
	
	GLuint TextureEmblemPause;
	
	gchar *cThemeFrame;
	gchar *cThemeReflect;
	gint itopleftX;
	gint itopleftY;
	gint ibottomleftX;
	gint ibottomleftY;
	gint ibottomrightX;
	gint ibottomrightY;
	gint itoprightX;
	gint itoprightY;
	
	gint numberButtons;
	gboolean osd;
	
	// A passer en structure :
	gint button1coordX;
	gint button1coordY;
	gint button1sizeX;
	gint button1sizeY;
	gchar *cThemeButton1;
	GLuint TextureButton1;
	gboolean mouseOnButton1;
	gchar *cOsdPlay;
	GLuint TextureOsdPlay;
	gchar *cOsdPause;
	GLuint TextureOsdPause;
	
	
	// A passer en structure :
	gint button2coordX;
	gint button2coordY;
	gint button2sizeX;
	gint button2sizeY;
	gchar *cThemeButton2;
	GLuint TextureButton2;
	gboolean mouseOnButton2;
	gchar *cOsdPrev;
	GLuint TextureOsdPrev;
	
	// A passer en structure :
	gint button3coordX;
	gint button3coordY;
	gint button3sizeX;
	gint button3sizeY;
	gchar *cThemeButton3;
	GLuint TextureButton3;
	gboolean mouseOnButton3;
	gchar *cOsdNext;
	GLuint TextureOsdNext;
	
	// A passer en structure :
	gint button4coordX;
	gint button4coordY;
	gint button4sizeX;
	gint button4sizeY;
	gchar *cThemeButton4;
	GLuint TextureButton4;
	gboolean mouseOnButton4;
	gchar *cOsdHome;
	GLuint TextureOsdHome;
	
	gint iMouseX;
	gint iMouseY;
	
	
	} ;


#endif
