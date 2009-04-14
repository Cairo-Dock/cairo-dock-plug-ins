
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>


//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gdouble fFontSizeRatio;
	CairoDockLabelDescription labelDescription;
	gchar *cShortkey;
	gchar *cIconAnimation;
	gdouble pBackgroundColor[4];
	gint iAnimationDuration;
	gint iAppearanceDuration;
	gint iCloseDuration;
	gchar **pDirList;
	} ;


typedef struct _CDChar {
	gchar c;
	cairo_surface_t *pSurface;
	GLuint iTexture;
	gint iWidth, iHeight;
	gint iAnimationTime;
	gint iInitialX, iInitialY;
	gint iFinalX, iFinalY;
	gint iCurrentX, iCurrentY;
	} CDChar;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GString *sCurrentText;
	gint iStopCount;
	gint iNbValidCaracters;
	Window iPreviouslyActiveWindow;
	Icon *pCurrentIcon;
	CairoDock *pCurrentDock;
	gint iTextWidth, iTextHeight;
	gint iCloseTime;
	GList *pCharList;
	gboolean bIgnoreIconState;
	gint iAppearanceTime;
	
	GHashTable *dir_hash;
	GList *possible_executables;
	GList *completion_items;
	GCompletion *completion;
	gboolean completion_started;
	
	gint iPrevMouseX, iPrevMouseY;
	gint iMouseX, iMouseY;
	gint iMotionCount;
	} ;


#endif
