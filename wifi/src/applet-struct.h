
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


typedef struct {
	gchar *defaultTitle;
	gchar *cUserImage[WIFI_NB_QUALITY];
	/*gchar *cDefault;
	gchar *c20Surface;
	gchar *c40Surface;
	gchar *c60Surface;
	gchar *c80Surface;
	gchar *c100Surface;*/
	CDWifiInfoType quickInfoType;
	gint iCheckInterval;
} AppletConfig;

typedef struct {
	cairo_surface_t *pSurfaces[WIFI_NB_QUALITY];
	guint iSidTimer;
	gint isWirelessDevice;
	CDWifiQuality iPreviousQuality;
	
	gint checkTimer;
	gint strengthTimer;
} AppletData;


#endif
