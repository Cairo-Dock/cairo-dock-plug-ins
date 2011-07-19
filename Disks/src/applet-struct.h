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

#define CD_DISKS_NB_MAX_VALUES 10

typedef enum _CDDisksDisplayType {
	CD_DISKS_GAUGE=0,
	CD_DISKS_GRAPH,
	CD_DISKS_NB_TYPES
	} CDDisksDisplayType; 


typedef struct {
	//~ char cName [16];
	gchar *cName;
	gboolean bInitialized;
	gboolean bAcquisitionOK;
	unsigned uMaxReadRate, uMaxWriteRate;
	unsigned uReadSpeed, uWriteSpeed;
	long long unsigned uReadBlocks, uWriteBlocks;
	//~ unsigned uID;
} CDDiskSpeedData;


struct _AppletConfig {
	gchar *defaultTitle;
	gchar *cGThemePath;
	gchar *cWatermarkImagePath;  // delete ?
	gdouble fAlpha;
	
	CDDisksDisplayType iDisplayType;
	CairoDockInfoDisplay iInfoDisplay;
	gint iCheckInterval;
	gdouble fSmoothFactor;

	CairoDockTypeGraph iGraphType;
	gboolean bMixGraph;
	gdouble fLowColor[3];  // Down
	gdouble fHigholor[3];
	gdouble fBgColor[4];
	//~ gdouble fLowColor2[3];  // Up
	//~ gdouble fHigholor2[3];
	
	gchar **cDisks;
	gsize iNumberDisks;
	
	gchar **cParts;
	gsize iNumberParts;
	
	gchar *cSystemMonitorCommand;
} ;

struct _AppletData {
	GTimer *pClock;
	// shared memory
	gsize iNumberDisks;
	GList *lDisks;
	GList *lParts;
	
	// end of shared memory
	CairoDockTask *pPeriodicTask;
	DBusGProxy *dbus_proxy_nm;
} ;


#endif
