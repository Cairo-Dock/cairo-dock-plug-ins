/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

#define CD_NETSPEED_NB_MAX_VALUES 2

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


typedef enum _CDWifiRenderType {
	CD_EFFECT_GAUGE=0,
	CD_EFFECT_GRAPH,
	CD_EFFECT_ICON,
	CD_WIFI_NB_TYPES
	} CDRenderType;

struct _AppletConfig {
	gchar *defaultTitle;
	gchar *cUserImage[CONNECTION_NB_QUALITY];
	gchar *cWifiConfigCommand;
	
	CDConnectionInfoType quickInfoType;  // unite du signal wifi.
	
	CDRenderType iRenderType;
	// jauge.
	gchar *cGThemePath;
	// graphe
	CairoDockTypeGraph iGraphType;
	gdouble fLowColor[3];
	gdouble fHigholor[3];
	gdouble fBgColor[4];
	gdouble fSmoothFactor;
	// icone
	CDWifiEffect iEffect;
	
	// parametres
	gboolean bModeWifi;  // TRUE pour l'affichage Wifi, FALSE pour l'affichage Netspeed.
	gchar *cInterface;  // interface (eth0, etc) a surveiller
	gint iStringLen;
	gchar *cSysMonitorCommand;  // command pour ouvrir un moniteur systeme.
	
	// wifi
	gint iWifiCheckInterval;
	
	// netspeed
	gint iNetspeedCheckInterval;
	
};


typedef struct _CDNetSpeed {
	CairoDockTask *pTask;
	// shared memory
	gboolean _bInitialized;
	gboolean _bAcquisitionOK;
	long long int _iReceivedBytes, _iTransmittedBytes;
	gint _iDownloadSpeed, _iUploadSpeed;
	gint _iMaxUpRate, _iMaxDownRate;
	// end of shared memory
	gboolean bAcquisitionOK;
	long long int iReceivedBytes, iTransmittedBytes;
	CairoDataRenderer *pDataRenderer;
	} CDNetSpeed;

typedef struct _CDWifi {
	CairoDockTask *pTask;
	// shared memory
	CDConnectionQuality _iQuality, _iPreviousQuality;
	gint _iPercent, _iPrevPercent;
	gint _iSignalLevel, _iPrevSignalLevel;
	gint _iPrevNoiseLevel, _iNoiseLevel;
	gchar *_cESSID;
	gchar *_cInterface;
	gchar *_cAccessPoint;
	// end of shared memory
	gboolean bWirelessExt;
	CDConnectionQuality iQuality;
	gchar *cInterface;
	gchar *cAccessPoint;
	gchar *cESSID;
	CairoDataRenderer *pDataRenderer;
	} CDWifi;

typedef struct _CDNetworkManager {
	gint iPercent, iPrevPercent;
	gchar *cActiveAccessPoint;
	gchar *cAccessPointHwAdress;
	gchar *cESSID;
	gint iSpeed;  // max bit rate
	
	DBusGProxy *dbus_proxy_ActiveConnection;
	DBusGProxy *dbus_proxy_Device;
	DBusGProxy *dbus_proxy_ActiveAccessPoint;
	DBusGProxy *dbus_proxy_NM;
	DBusGProxy *dbus_proxy_WirelessDevice;
	DBusGProxy *dbus_proxy_WiredDevice;
	
	
	CairoDataRenderer *pDataRenderer;
	} CDNetworkManager;

struct _AppletData {
	CDConnectionQuality iQuality, iPreviousQuality;
	gint iPercent, iPrevPercent;
	gint iSignalLevel, iPrevSignalLevel;
	gint iPrevNoiseLevel, iNoiseLevel;
	gchar *cESSID;
	gchar *cInterface;
	gint iSpeed;  // max bit rate
	
	gboolean bDbusConnection;  // TRUE si on a trouve NM sur le bus.
	
	gboolean bWirelessExt;
	gboolean bWiredExt;
	
	cairo_surface_t *pSurfaces[CONNECTION_NB_QUALITY];
	
	DBusGProxy *dbus_proxy_ActiveConnection;
	DBusGProxy *dbus_proxy_Device;
	DBusGProxy *dbus_proxy_ActiveAccessPoint;
	DBusGProxy *dbus_proxy_NM;
	DBusGProxy *dbus_proxy_WirelessDevice;
	DBusGProxy *dbus_proxy_WiredDevice;
	
	gchar *cActiveConnection;
	gchar *cDevice;
	gchar *cServiceName;
	gchar *cAccessPoint;
	gchar *cAccessPointHwAdress;
	GPtrArray *pMenuAccessPoints;
	
	CDNetSpeed netSpeed;
	CDWifi wifi;
	CDNetworkManager nm;
};


#endif
