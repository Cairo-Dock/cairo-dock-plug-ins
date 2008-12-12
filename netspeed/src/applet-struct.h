
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>


struct _AppletConfig {
	gchar *defaultTitle;
	gint iCheckInterval;
	const gchar *cGThemePath;
	gchar *cWatermarkImagePath;
	gdouble fAlpha;
	
	gboolean bUseGraphic;
	CairoDockTypeGraph iGraphType;
	gboolean bMixGraph;
	gdouble fLowColor[3];  // Down
	gdouble fHigholor[3];
	gdouble fBgColor[4];
	gdouble fLowColor2[3];  // Up
	gdouble fHigholor2[3];
	
	gchar *cInterface;
	gint iStringLen;
	CairoDockInfoDisplay iInfoDisplay;
	
	gchar *cSystemMonitorCommand;
} ;

struct _AppletData {
	GTimer *pClock;
	gboolean bInitialized;
	long long int iReceivedBytes, iTransmittedBytes;
	gint iDownloadSpeed, iUploadSpeed;
	gint iMaxUpRate, iMaxDownRate;
	gboolean bAcquisitionOK;
	CairoDockMeasure *pMeasureTimer;
	Gauge *pGauge;
	CairoDockGraph *pGraph;
	DBusGProxy *dbus_proxy_nm;
} ;


#endif
