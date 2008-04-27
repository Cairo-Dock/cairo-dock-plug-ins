
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef enum {
	CPUSAGE_NO_INFO = 0,
	CPUSAGE_INFO_ON_ICON,
	CPUSAGE_INFO_ON_LABEL,
	CPUSAGE_NB_INFO_DISPLAY
} CDCpusageInfoDisplay;

typedef struct {
	gchar *defaultTitle;
	gint iCheckInterval;
	
	CDCpusageInfoDisplay iInfoDisplay;
	gchar *cThemePath;
	gboolean gaugeIcon; 
} AppletConfig;

typedef struct {
	gint inDebug;
	Gauge *pGauge;
	
	GTimer *pClock;
	guint cpu_user, cpu_user_nice, cpu_system, cpu_idle;
	gdouble cpu_usage;
	gboolean bInitialized;
	CairoDockMeasure *pMeasureTimer;
	gboolean bAcquisitionOK;
} AppletData;


#endif
