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

#define CD_SYSMONITOR_NB_MAX_VALUES 4

#define CD_SYSMONITOR_PROC_FS "/proc"

typedef enum _CDSysmonitorDisplayType {
	CD_SYSMONITOR_GAUGE=0,
	CD_SYSMONITOR_GRAPH,
	CD_SYSMONITOR_BAR,
	CD_SYSMONITOR_NB_TYPES
	} CDSysmonitorDisplayType; 

struct _AppletConfig {
	gchar *defaultTitle;
	gint iCheckInterval;
	gdouble fSmoothFactor;
	gboolean bShowCpu;
	gboolean bShowRam;
	gboolean bShowNvidia;
	gboolean bShowSwap;
	gboolean bShowFreeMemory;
	
	CairoDockInfoDisplay iInfoDisplay;
	gchar *cGThemePath;
	gchar *cWatermarkImagePath;
	gdouble fAlpha;
	
	CDSysmonitorDisplayType iDisplayType;
	CairoDockTypeGraph iGraphType;
	gdouble fLowColor[3];
	gdouble fHigholor[3];
	gdouble fBgColor[4];
	gboolean bMixGraph;
	
	gint iNbDisplayedProcesses;
	gint iProcessCheckInterval;
	gboolean bTopInPercent;
	CairoDockLabelDescription *pTopTextDescription;
	
	gchar *cSystemMonitorCommand;
	gchar *cSystemMonitorClass;
	gboolean bStealTaskBarIcon;
	gdouble fUserHZ;
	
	gchar *cSoundPath;
	gint iLowerLimit;
	gint iUpperLimit;
	gint iAlertLimit;
	gboolean bAlert;
	gboolean bAlertSound;
} ;

typedef struct {
	gint iPid;
	gchar *cName;
	gint iCpuTime;
	gdouble fCpuPercent;
	gdouble iMemAmount;
	gdouble fLastCheckTime;
	} CDProcess;

struct _AppletData {
	// infos, constantes.
	gint iNbCPU;
	gulong iMemPageSize;
	gint iFrequency;
	gchar *cModelName;
	gchar *cGPUName;
	gint iVideoRam;
	gchar *cDriverVersion;
	
	CairoDockTask *pPeriodicTask;
	// shared memory for the main thread.
	gboolean bInitialized;
	gboolean bAcquisitionOK;
	GTimer *pClock;
	long long int cpu_user, cpu_user_nice, cpu_system, cpu_idle;
	unsigned long long ramTotal, ramFree, ramUsed, ramBuffers, ramCached;
	unsigned long long swapTotal, swapFree, swapUsed;
	gint iGPUTemp;
	gdouble fCpuPercent;
	gdouble fPrevCpuPercent;
	gdouble fRamPercent,fSwapPercent;
	gdouble fPrevRamPercent, fPrevSwapPercent;
	gdouble fGpuTempPercent;
	gdouble fPrevGpuTempPercent;
	gboolean bNeedsUpdate;
	gint iTimerCount;
	// end of shared memory.
	gboolean bAlerted;
	gint iCount;  // pour sous-echantilloner les acquisitions de valeurs moins variables.
	
	gint iNbProcesses;
	CairoDialog *pTopDialog;
	cairo_surface_t *pTopSurface;
	CairoDockTask *pTopTask;
	// shared memory for the "top" thread.
	GHashTable *pProcessTable;
	CDProcess **pTopList;
	GTimer *pTopClock;
	gboolean bSortTopByRam;
	// end of shared memory.
} ;


#endif
