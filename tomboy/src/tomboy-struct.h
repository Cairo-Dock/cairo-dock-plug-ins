#ifndef __TOMBOY_STRUCT__
#define  __TOMBOY_STRUCT__

#include <cairo-dock.h>
	
#define TOMBOY_DEFAULT_NAME "_TomBoy_"

typedef struct {
	gchar *defaultTitle;
	gchar *cIconDefault;
	gchar *cIconClose;
	gchar *cIconBroken;
	gboolean bNoDeletedSignal;
	gchar *cRenderer;
	} AppletConfig;

typedef struct {
	cairo_surface_t *pSurfaceDefault;
	cairo_surface_t *pSurfaceClose;
	cairo_surface_t *pSurfaceBroken;
	gboolean dbus_enable;
	gboolean opening;
	guint iSidCheckNotes;
	GHashTable *hNoteTable;
	} AppletData;

#endif
