
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef struct {
	gchar *defaultTitle;
	gint iCheckInterval;
	
	CairoDockInfoDisplay iInfoDisplay;
	gchar *cThemePath;
	gboolean gaugeIcon;
	
	gint iNbDisplayedProcesses;
	gint iProcessCheckInterval;
	CairoDockLabelDescription *pTopTextDescription;
} AppletConfig;

typedef struct {
	gint iPid;
	gchar *cName;
	gint iCpuTime;
	gdouble fCpuPercent;
	gdouble fLastCheckTime;
	} CDProcess;

typedef struct {
	Gauge *pGauge;
	
	gint iNbCPU;
	gint iFrequency;
	gchar *cModelName;
	gboolean bInitialized;
	gboolean bAcquisitionOK;
	CairoDockMeasure *pMeasureTimer;
	GTimer *pClock;
	long long int cpu_user, cpu_user_nice, cpu_system, cpu_idle;
	gdouble cpu_usage;
	
	GHashTable *pProcessTable;
	gint iNbProcesses;
	CDProcess **pTopList;
	CairoDialog *pTopDialog;
	GTimer *pTopClock;
	cairo_surface_t *pTopSurface;
	CairoDockMeasure *pTopMeasureTimer;
} AppletData;


#endif
