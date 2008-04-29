
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>


typedef struct {
	gchar *defaultTitle;
	gint iCheckInterval;
	gchar *cThemePath;
	gchar *cInterface;
	gint iStringLen;
	CairoDockInfoDisplay iInfoDisplay;
} AppletConfig;

typedef struct {
	GTimer *pClock;
	gboolean bInitialized;
	gint iReceivedBytes, iTransmittedBytes;
	gint iDownloadSpeed, iUploadSpeed;
	gint iMaxUpRate, iMaxDownRate;
	gboolean bAcquisitionOK;
	CairoDockMeasure *pMeasureTimer;
	Gauge *pGauge;
	DBusGProxy *dbus_proxy_nm;
} AppletData;


#endif
