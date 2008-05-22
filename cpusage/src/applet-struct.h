
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef struct {
	gchar *defaultTitle;
	gint iCheckInterval;
	
	CairoDockInfoDisplay iInfoDisplay;
	gchar *cThemePath;
	gboolean gaugeIcon; 
} AppletConfig;

typedef struct {
	Gauge *pGauge;
	
	GTimer *pClock;
	guint cpu_user, cpu_user_nice, cpu_system, cpu_idle;
	gdouble cpu_usage;
	gint iNbCPU;
	gint iFrequency;
	gchar *cModelName;
	gboolean bInitialized;
	CairoDockMeasure *pMeasureTimer;
	gboolean bAcquisitionOK;
} AppletData;


#endif
