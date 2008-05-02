#ifndef __TOMBOY_STRUCT__
#define  __TOMBOY_STRUCT__

#include <cairo-dock.h>
	
#define TOMBOY_DEFAULT_NAME "_tomboy_"

typedef struct {
	gchar *name;
	gchar *title;
	} TomBoyNote;
	
typedef struct {
	gchar *defaultTitle;
	gchar *cIconDefault;
	gchar *cIconClose;
	gchar *cIconBroken;
	gboolean bNoDeletedSignal;
	} AppletConfig;

typedef struct {
	cairo_surface_t *pSurfaceDefault;
	cairo_surface_t *pSurfaceClose;
	cairo_surface_t *pSurfaceBroken;
	gboolean dbus_enable;
	gboolean opening;
	GList *noteList;
	int countNotes;
	guint iSidCheckNotes;
	} AppletData;

#endif
