
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

struct _AppletConfig {
	gchar *defaultTitle;
	gint iCheckInterval;
	CairoDockInfoDisplay iInfoDisplay;
	gboolean bShowSwap;
	
	const gchar *cGThemePath;
	gchar *cFilligranImagePath;
	gdouble fAlpha;
	
	gint iNbDisplayedProcesses;
	gboolean bTopInPercent;
	CairoDockLabelDescription *pTopTextDescription;
} ;


typedef struct {
	gint iPid;
	gchar *cName;
	gdouble iMemAmount;
	} CDProcess;

struct _AppletData {
	CairoDockMeasure *pMeasureTimer;
	guint ramTotal, ramFree, ramUsed, ramBuffers, ramCached;
	guint swapTotal, swapFree, swapUsed;
	gdouble fPrevRamPercent, fPrevSwapPercent;
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
} ;


#endif
