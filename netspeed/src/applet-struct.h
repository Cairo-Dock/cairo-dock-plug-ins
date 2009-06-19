
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>


typedef enum _CDNetspeedDisplayType {
	CD_NETSPEED_GAUGE=0,
	CD_NETSPEED_GRAPH,
	CD_NETSPEED_BAR,
	CD_NETSPEED_NB_TYPES
	} CDNetspeedDisplayType; 

#define CD_NETSPEED_NB_MAX_VALUES 2

struct _AppletConfig {
	gchar *defaultTitle;
	gint iCheckInterval;
	gchar *cGThemePath;
	gchar *cWatermarkImagePath;
	gdouble fAlpha;
	
	CDNetspeedDisplayType iDisplayType;
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
	gdouble fSmoothFactor;
} ;

struct _AppletData {
	GTimer *pClock;
	// shared memory
	gboolean bInitialized;
	gboolean bAcquisitionOK;
	long long int iReceivedBytes, iTransmittedBytes;
	gint iDownloadSpeed, iUploadSpeed;
	gint iMaxUpRate, iMaxDownRate;
	// end of shared memory
	CairoDockTask *pPeriodicTask;
	DBusGProxy *dbus_proxy_nm;
} ;


#endif
