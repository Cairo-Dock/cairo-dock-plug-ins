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
	RendererRotateTheme iRotateTheme;
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
	GldiTask *pPeriodicTask;
	DBusGProxy *dbus_proxy_nm;
} ;


#endif
