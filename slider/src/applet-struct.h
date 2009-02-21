
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

#include <glib/gstdio.h>

typedef enum {
	SLIDER_DEFAULT = 0,
	SLIDER_FADE,
	SLIDER_BLANK_FADE,
	SLIDER_FADE_IN_OUT,
	SLIDER_SIDE_KICK,
	SLIDER_DIAPORAMA,
	SLIDER_GROW_UP,
	SLIDER_SHRINK_DOWN,
	SLIDER_RANDOM,
	SLIDER_NB_ANIMATION
} SliderAnimation;

typedef enum {
	SLIDER_PAUSE = 0,
	SLIDER_OPEN_IMAGE,
	SLIDER_NB_CLICK_OPTION
} SliderClickOption;

typedef enum {
	SLIDER_ORIENTATION_NORMAL=1,
	SLIDER_ORIENTATION_HFLIP,
	SLIDER_ORIENTATION_ROT_180,
	SLIDER_ORIENTATION_VFLIP,
	SLIDER_ORIENTATION_ROT_90_HFLIP,
	SLIDER_ORIENTATION_ROT_90,
	SLIDER_ORIENTATION_ROT_90_VFLIP,
	SLIDER_ORIENTATION_ROT_270
} SliderOrientation;

typedef struct {
	double fImgX;
	double fImgY;
	double fImgW;
	double fImgH;
} myImgLips;

typedef enum {
	SLIDER_UNKNOWN_FORMAT = 0,
	SLIDER_PNG,
	SLIDER_JPG,
	SLIDER_SVG,
	SLIDER_GIF,
	SLIDER_XPM,
	SLIDER_NB_IMAGE_FORMAT
} SliderImageFormat;

typedef enum {
	SLIDER_PERSONNAL = 0,
	SLIDER_FRAME_REFLECTS,
	SLIDER_SCOTCH,
	SLIDER_FRAME_SCOTCH,
	SLIDER_NB_DECORATIONS
} SliderDecoration;


typedef struct {
	gchar *cPath;
	gint iSize;
	SliderImageFormat iFormat;
	gboolean iOrientation;
} SliderImage;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gint iSlideTime;
	gchar *cDirectory;
	gboolean bSubDirs;
	gboolean bNoStretch;
	gboolean bFillIcon;
	gboolean bRandom;
	gboolean bImageName;
	gdouble pBackgroundColor[4];
	SliderAnimation iAnimation;
	SliderClickOption iClickOption;
	gboolean bUseThread;
} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GList *pList;
	GList *pElement;
	gboolean bPause;
	gdouble fAnimAlpha;
	gdouble fAnimCNT;
	gint iAnimCNT;
	gint iAnimTimerID;
	gint iTimerID;
	myImgLips pImgL;
	myImgLips pPrevImgL;
	cairo_surface_t* pCairoSurface;
	cairo_surface_t* pPrevCairoSurface;
	gdouble fSurfaceWidth, fSurfaceHeight;
	SliderAnimation iAnimation;
	CairoDockMeasure *pMeasureDirectory;
	CairoDockMeasure *pMeasureImage;
} ;


#endif
