
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef enum {
	WIFI_INFO_NONE = 0,
	WIFI_INFO_SIGNAL_STRENGTH_LEVEL,
	WIFI_INFO_SIGNAL_STRENGTH_PERCENT,
	WIFI_INFO_SIGNAL_STRENGTH_DB,
	WIFI_NB_INFO_TYPE
} CDWifiInfoType;

typedef enum {
	WIFI_QUALITY_NO_SIGNAL = 0,
	WIFI_QUALITY_VERY_LOW,
	WIFI_QUALITY_LOW,
	WIFI_QUALITY_MIDDLE,
	WIFI_QUALITY_GOOD,
	WIFI_QUALITY_EXCELLENT,
	WIFI_NB_QUALITY
} CDWifiQuality;

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
	gchar *cUserImage[WIFI_NB_QUALITY];
	gchar *cGThemePath;
	gchar *cUserCommand;
	gchar *cWatermarkImagePath;
	gdouble fAlpha;
	
	CDWifiInfoType quickInfoType;
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
	CDWifiQuality iQuality, iPreviousQuality;
	gint iPercent, iPrevPercent;
	gint iSignalLevel, iPrevSignalLevel;
	gint iPrevNoiseLevel, iNoiseLevel;
	gchar *cESSID;
	gchar *cInterface;
	gchar *cAccessPoint;
	// end of shared memory
	gboolean bWirelessExt;
	CairoDockTask *pTask;
	cairo_surface_t *pSurfaces[WIFI_NB_QUALITY];
};


#endif
