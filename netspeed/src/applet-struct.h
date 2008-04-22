
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>


typedef enum {
	NETSPEED_NO_INFO = 0,
	NETSPEED_INFO_ON_ICON,
	NETSPEED_INFO_ON_LABEL,
	NETSPEED_NB_INFO_DISPLAY
} CDNetSpeedInfoDisplay;

typedef struct {
	gchar *defaultTitle;
	gint iCheckInterval;
	gchar *cThemePath;
	gchar *cInterface;
	gint iStringLen;
	CDNetSpeedInfoDisplay iInfoDisplay;
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
