#ifndef __TOMBOY_STRUCT__
#define  __TOMBOY_STRUCT__

#include <cairo-dock.h>
#include <cairo-dock-applet-single-instance.h>
	
#define TOMBOY_DEFAULT_NAME "_TomBoy_"

struct _AppletConfig {
	gchar *defaultTitle;
	gchar *cIconDefault;
	gchar *cIconClose;
	gchar *cIconBroken;
	gboolean bNoDeletedSignal;
	gchar *cRenderer;
	gboolean bDrawContent;
	gboolean bPopupContent;
	gchar *cDateFormat;
	gboolean bAutoNaming;
	gboolean bAskBeforeDelete;
	gdouble fTextColor[3];
	} ;

struct _AppletData {
	cairo_surface_t *pSurfaceDefault;
	cairo_surface_t *pSurfaceNote;
	gboolean dbus_enable;
	gboolean opening;
	guint iSidCheckNotes;
	GHashTable *hNoteTable;
	CairoDockTask *pTask;
	gint iSidResetQuickInfo;
	} ;

#endif
