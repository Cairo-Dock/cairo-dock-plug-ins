
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

struct _AppletConfig {
	gchar *defaultTitle;
	gint iCheckInterval;
	CairoDockInfoDisplay iInfoDisplay;
	gboolean bShowSwap;
	
	const gchar *cGThemePath;
	gchar *cWatermarkImagePath;
	gdouble fAlpha;
	
	gboolean bUseGraphic;
	CairoDockTypeGraph iGraphType;
	gboolean bMixGraph;
	gdouble fLowColor[3];  // RAM
	gdouble fHigholor[3];
	gdouble fBgColor[4];
	gdouble fLowColor2[3];  // SWAP
	gdouble fHigholor2[3];
	
	gint iNbDisplayedProcesses;
	gboolean bTopInPercent;
	CairoDockLabelDescription *pTopTextDescription;
	
	gchar *cSystemMonitorCommand;
} ;


typedef struct {
	gint iPid;
	gchar *cName;
	gdouble iMemAmount;
	} CDProcess;

struct _AppletData {
	CairoDockMeasure *pMeasureTimer;
	unsigned long long ramTotal, ramFree, ramUsed, ramBuffers, ramCached;
	unsigned long long swapTotal, swapFree, swapUsed;
	gdouble fPrevRamPercent, fPrevSwapPercent;
	gboolean bAcquisitionOK;
	gboolean bInitialized;
	Gauge *pGauge;
	CairoDockGraph *pGraph;
	gulong iMemPageSize;
	CDProcess **pTopList;
	CDProcess **pPreviousTopList;
	gint iNbDisplayedProcesses;
	cairo_surface_t *pTopSurface;
	CairoDialog *pTopDialog;
	CairoDockMeasure *pTopMeasureTimer;
} ;


#endif
