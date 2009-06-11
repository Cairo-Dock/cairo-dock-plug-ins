
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

#define CD_SYSMONITOR_NB_MAX_VALUES 4

#define CD_SYSMONITOR_PROC_FS "/proc"

typedef enum _CDSysmonitorDisplayType {
	CD_SYSMONITOR_GAUGE=0,
	CD_SYSMONITOR_GRAPH,
	CD_SYSMONITOR_BAR,
	CD_SYSMONITOR_NB_TYPES
	} CDSysmonitorDisplayType; 

struct _AppletConfig {
	gchar *defaultTitle;
	gint iCheckInterval;
	gboolean bShowCpu;
	gboolean bShowRam;
	gboolean bShowNvidia;
	gboolean bShowSwap;
	gboolean bShowFreeMemory;
	
	CairoDockInfoDisplay iInfoDisplay;
	const gchar *cGThemePath;
	gchar *cWatermarkImagePath;
	gdouble fAlpha;
	
	CDSysmonitorDisplayType iDisplayType;
	CairoDockTypeGraph iGraphType;
	gdouble fLowColor[3];
	gdouble fHigholor[3];
	gdouble fBgColor[4];
	
	gint iNbDisplayedProcesses;
	gint iProcessCheckInterval;
	gboolean bTopInPercent;
	CairoDockLabelDescription *pTopTextDescription;
	
	gchar *cSystemMonitorCommand;
	gchar *cSystemMonitorClass;
	gboolean bStealTaskBarIcon;
	gdouble fUserHZ;
} ;

typedef struct {
	gint iPid;
	gchar *cName;
	gint iCpuTime;
	gdouble fCpuPercent;
	gdouble iMemAmount;
	gdouble fLastCheckTime;
	} CDProcess;

struct _AppletData {
	Gauge *pGauge;
	CairoDockGraph *pGraph;
	
	gint iNbCPU;  // constante.
	gulong iMemPageSize;  // constante.
	gint iFrequency;  // constante (a voir si on l'actualise...)
	gchar *cModelName;  // constante
	
	CairoDockMeasure *pMeasureTimer;
	// memoire partagee pour le thread principal.
	gboolean bInitialized;
	gboolean bAcquisitionOK;
	GTimer *pClock;
	long long int cpu_user, cpu_user_nice, cpu_system, cpu_idle;
	gdouble cpu_usage;
	unsigned long long ramTotal, ramFree, ramUsed, ramBuffers, ramCached;
	unsigned long long swapTotal, swapFree, swapUsed;
	gdouble fPrevRamPercent, fPrevSwapPercent;
	// fin de la memoire partagee.
	
	gint iNbProcesses;
	CairoDialog *pTopDialog;
	cairo_surface_t *pTopSurface;
	CairoDockMeasure *pTopMeasureTimer;
	// memoire partagee pour le thread "top".
	GHashTable *pProcessTable;
	CDProcess **pTopList;
	GTimer *pTopClock;
	gboolean bSortTopByRam;
	// fin de la memoire partagee.
} ;


#endif
