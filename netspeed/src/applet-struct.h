
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef struct {
	gchar *defaultTitle;
	gchar *cDefault;
	int quickInfoType;
} AppletConfig;

typedef struct {
	cairo_surface_t *pDefault;
	cairo_surface_t *pUnknown;	
	cairo_surface_t *pBad;	
	cairo_surface_t *pOk;	
	int checkTimer;
	int interfaceFound;	
	int strengthTimer;
} AppletData;


#endif
