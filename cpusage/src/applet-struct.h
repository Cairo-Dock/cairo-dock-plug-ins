
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

struct _AppletConfig {
	gchar *defaultTitle;
	gint iCheckInterval;
	
	CairoDockInfoDisplay iInfoDisplay;
	const gchar *cGThemePath;
	gchar *cWatermarkImagePath;
	gdouble fAlpha;
	
	gboolean bUseGraphic;
	CairoDockTypeGraph iGraphType;
	gdouble fLowColor[3];
	gdouble fHigholor[3];
	gdouble fBgColor[4];
	
	gint iNbDisplayedProcesses;
	gint iProcessCheckInterval;
	CairoDockLabelDescription *pTopTextDescription;
	
	gchar *cSystemMonitorCommand;
	gdouble fUserHZ;
} ;

typedef struct {
	gint iPid;
	gchar *cName;
	gint iCpuTime;
	gdouble fCpuPercent;
	gdouble fLastCheckTime;
	} CDProcess;

struct _AppletData {
	gint iNbCPU;
	gint iFrequency;
	gchar *cModelName;
	gboolean bInitialized;
	gboolean bAcquisitionOK;
	CairoDockTask *pTask;
	GTimer *pClock;
	long long int cpu_user, cpu_user_nice, cpu_system, cpu_idle;
	gdouble cpu_usage;
	
	GHashTable *pProcessTable;
	gint iNbProcesses;
	CDProcess **pTopList;
	CairoDialog *pTopDialog;
	GTimer *pTopClock;
	cairo_surface_t *pTopSurface;
	CairoDockTask *pTopTask;
} ;


#endif
