
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gint iSpeed;
	gint iRotationSpeed;
	gchar *cDropIndicatorImageName;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	cairo_surface_t *pDropIndicatorSurface;
	gdouble fDropIndicatorWidth, fDropIndicatorHeight;
	GLuint iDropIndicatorTexture;
	GLuint iBilinearGradationTexture;
	gint iInitialHeight, iInitialWidth;
	} ;

typedef struct _CDDropIndicatorData {
	gint iDropIndicatorOffset;
	gint iDropIndicatorRotation;
	gdouble fAlpha;
	} CDDropIndicatorData;


#endif
