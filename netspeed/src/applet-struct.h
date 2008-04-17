
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef struct {
	gchar *defaultTitle;
/*	gchar *cDefault;
	gchar *cUnknown;
	gchar *cBad;
	gchar *cOk;*/
	gint iCheckInterval;
	gint dCheckInterval;
	gchar *cThemePath;
	gchar *cInterface;
	gint iStringLen;
} AppletConfig;

typedef struct {
/*	cairo_surface_t *pDefault;
	cairo_surface_t *pUnknown;	
	cairo_surface_t *pBad;	
	cairo_surface_t *pOk;	*/
	guint iSidTimer;
	gint checkTimer;
	gint interfaceFound;
	GTimer *pClock;
	gint iReceivedBytes, iTransmittedBytes;
	gboolean bAcquisitionOK;
	CairoDockMeasure *pMeasureTimer;
	
	Gauge *pGauge;
} AppletData;


#endif
