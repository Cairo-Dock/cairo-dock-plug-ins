
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gint iSpeed;
	gdouble fRotationSpeed;
	gchar *cDropIndicatorImageName;
	gchar *cHoverIndicatorImageName;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	cairo_surface_t *pDropIndicatorSurface;
	gdouble fDropIndicatorWidth, fDropIndicatorHeight;
	GLuint iDropIndicatorTexture;
	cairo_surface_t *pHoverIndicatorSurface;
	gdouble fHoverIndicatorWidth, fHoverIndicatorHeight;
	GLuint iHoverIndicatorTexture;
	GLuint iBilinearGradationTexture;
	} ;

typedef struct _CDDropIndicatorData {
	gint iDropIndicatorOffset;
	gint iDropIndicatorRotation;
	gdouble fAlpha;
	gdouble fAlphaHover;
	} CDDropIndicatorData;


#endif
