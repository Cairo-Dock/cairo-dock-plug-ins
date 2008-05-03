
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <glib/gi18n.h>
#include <glib/gstdio.h>


typedef enum {
	SLIDER_DEFAULT = 0,
	SLIDER_FADE,
	SLIDER_FADE_IN_OUT,
	SLIDER_SIDE_KICK,
	SLIDER_DIAPORAMA,
	SLIDER_GROW_UP,
	SLIDER_SHRINK_DOWN,
	SLIDER_NB_ANIMATION,
} SliderAnimation;

typedef struct {
	double fImgX;
	double fImgY;
	double fImgW;
	double fImgH;
} myImgLips;

//\___________ structure containing the applet's configuration parameters.
typedef struct {
	double dSlideTime;
	gchar *cDirectory;
	gboolean bSubDirs;
	gboolean bNoStrench;
	gboolean bEffect;
	gboolean bRandom;
	SliderAnimation pAnimation;
	gdouble pBackgroundColor[4];
} AppletConfig;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
typedef struct {
	GList *pList;
	GList *pElement;
	gboolean bPause;
	double fAnimAlpha;
	double fAnimCNT;
	int iAnimCNT;
	int iAnimTimerID;
	int iTimerID;
	int iImagesNumber;
	myImgLips pImgL;
	cairo_t* pCairoContext;
  cairo_surface_t* pCairoSurface;
} AppletData;


#endif
