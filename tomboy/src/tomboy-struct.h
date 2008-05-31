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
	gboolean bDrawContent;
	gchar *cDateFormat;
	} AppletConfig;

typedef struct {
	cairo_surface_t *pSurfaceDefault;
	cairo_surface_t *pSurfaceNote;
	gboolean dbus_enable;
	gboolean opening;
	guint iSidCheckNotes;
	GHashTable *hNoteTable;
	CairoDockMeasure *pMeasureTimer;
	} AppletData;

#endif
