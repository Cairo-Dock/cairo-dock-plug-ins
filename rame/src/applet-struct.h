
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
	gint iNbDisplayedProcesses;
	gboolean bTopInPercent;
	CairoDockLabelDescription *pTopTextDescription;
} AppletConfig;


typedef struct {
	gint iPid;
	gchar *cName;
	gdouble iMemAmount;
	} CDProcess;

typedef struct {
	CairoDockMeasure *pMeasureTimer;
	guint ramTotal, ramFree, ramUsed, ramBuffers, ramCached;
	guint swapTotal, swapFree, swapUsed;
	gboolean bAcquisitionOK;
	gboolean bInitialized;
	Gauge *pGauge;
	glong iMemPageSize;
	CDProcess **pTopList;
	CDProcess **pPreviousTopList;
	gint iNbDisplayedProcesses;
	cairo_surface_t *pTopSurface;
	CairoDialog *pTopDialog;
	CairoDockMeasure *pTopMeasureTimer;
} AppletData;


#endif
