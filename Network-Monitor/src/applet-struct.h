
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef enum {
	CONNECTION_INFO_NONE = 0,
	CONNECTION_INFO_SIGNAL_STRENGTH_LEVEL,
	CONNECTION_INFO_SIGNAL_STRENGTH_PERCENT,
	CONNECTION_INFO_SIGNAL_STRENGTH_DB,
	CONNECTION_NB_INFO_TYPE
} CDConnectionInfoType;

typedef enum {
	WIFI_QUALITY_NO_SIGNAL = 0,
	WIFI_QUALITY_VERY_LOW,
	WIFI_QUALITY_LOW,
	WIFI_QUALITY_MIDDLE,
	WIFI_QUALITY_GOOD,
	WIFI_QUALITY_EXCELLENT,
	WIRED_NO_CONNECTION,
	WIRED_CONNECTION,
	CONNECTION_NB_QUALITY
} CDConnectionQuality;

typedef enum {
	WIFI_EFFECT_NONE = 0,
	WIFI_EFFECT_ZOOM,
	WIFI_EFFECT_TRANSPARENCY,
	WIFI_EFFECT_BAR,
} CDWifiEffect;


typedef enum _CDWifiDisplayType {
	CD_WIFI_GAUGE=0,
	CD_WIFI_GRAPH,
	CD_WIFI_BAR,
	CD_WIFI_NB_TYPES
	} CDWifiDisplayType; 

struct _AppletConfig {
	gchar *defaultTitle;
	gchar *cUserImage[CONNECTION_NB_QUALITY];
	gchar *cGThemePath;
	gchar *cUserCommand;
	gchar *cWatermarkImagePath;
	gdouble fAlpha;
	
	CDConnectionInfoType quickInfoType;
	CDWifiEffect iEffect;
	CDWifiDisplayType iDisplayType;
	
	gint iCheckInterval;
	
	CairoDockTypeGraph iGraphType;
	gdouble fLowColor[3];
	gdouble fHigholor[3];
	gdouble fBgColor[4];
	gdouble fSmoothFactor;
	
	gboolean bESSID;
};

struct _AppletData {
	// shared memory
	CDConnectionQuality iQuality, iPreviousQuality;
	gint iPercent, iPrevPercent;
	gint iSignalLevel, iPrevSignalLevel;
	gint iPrevNoiseLevel, iNoiseLevel;
	gchar *cESSID;
	gchar *cInterface;
	gchar *cAccessPoint;
	gint iSpeed;
	// end of shared memory
	
	gboolean bDbusConnection;
	
	gboolean bWirelessExt;
	gboolean bWiredExt;
	
	CairoDockTask *pTask;
	cairo_surface_t *pSurfaces[CONNECTION_NB_QUALITY];
	
	DBusGProxy *dbus_proxy_ActiveConnection;
	DBusGProxy *dbus_proxy_Device;
	DBusGProxy *dbus_proxy_ActiveAccessPoint;
	
	gchar *cDevice;
	gchar *cActiveAccessPoint;
};



#endif
