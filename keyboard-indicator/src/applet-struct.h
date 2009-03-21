
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bShowKbdIndicator;
	gchar *cBackgroundImage;
	CairoDockLabelDescription textDescription;
	gint iTransitionDuration;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	cairo_surface_t *pBackgroundSurface;
	cairo_surface_t *pOldSurface;
	cairo_surface_t *pCurrentSurface;
	gint iOldTextWidth, iOldTextHeight;
	gint iCurrentTextWidth, iCurrentTextHeight;
	GLuint iBackgroundTexture;
	GLuint iOldTexture;
	GLuint iCurrentTexture;
	gint iCurrentGroup, iCurrentIndic;
	} ;


#endif
