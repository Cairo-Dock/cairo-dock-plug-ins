
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>


struct _AppletConfig {
	gchar *defaultTitle;
	gint iCheckInterval;
	gchar *cThemePath;
	gchar *cInterface;
	gint iStringLen;
	CairoDockInfoDisplay iInfoDisplay;
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
	DBusGProxy *dbus_proxy_nm;
} ;


#endif
