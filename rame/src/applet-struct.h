
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef struct {
	gchar *defaultTitle;
	gint iCheckInterval;
	CairoDockInfoDisplay iInfoDisplay;
	gchar *cThemePath;
	gboolean gaugeIcon;
	gboolean bShowSwap;
} AppletConfig;

typedef struct {
	GTimer *pClock;
	guint ramTotal, ramFree, ramUsed, ramBuffers, ramCached;
	guint swapTotal, swapFree, swapUsed;
	gboolean bAcquisitionOK;
	CairoDockMeasure *pMeasureTimer;
	gboolean bShowSwap;
	Gauge *pGauge;
} AppletData;


#endif
