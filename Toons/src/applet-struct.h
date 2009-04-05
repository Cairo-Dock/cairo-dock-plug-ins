
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gchar *cThemePath;
	CairoDockLoadImageModifier iLoadingModifier;
	gint iWinkDelay;
	gint iWinkDuration;
	gboolean bFastCheck;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	// centre de la pupille, dans le referentiel de la surface cairo.
	gint iXeyes[2], iYeyes[2];
	gint iEyesWidth[2], iEyesHeight[2];
	gdouble fPrevXpupil[2], fPrevYpupil[2];
	gdouble fXpupil[2], fYpupil[2];
	// image du fond
	cairo_surface_t *pBgSurface;
	GLuint iBgTexture;
	gdouble iXbg, iYbg;
	gint iBgWidth, iBgHeight;
	// pupille
	cairo_surface_t *pPupilSurface[2];
	GLuint iPupilTexture[2];
	gint iPupilWidth[2], iPupilHeight[2];
	// paupiere
	cairo_surface_t *pEyelidSurface;
	GLuint iEyelidTexture;
	gdouble iXeyelid, iYeyelid;
	gint iEyelidWidth, iEyelidHeight;
	// masque
	cairo_surface_t *pToonSurface;
	GLuint iToonTexture;
	gint iToonWidth, iToonHeight;
	// clignement des yeux.
	gint iTimeCount;
	gboolean bWink;
	} ;


#endif
