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

#ifndef __POWERMANAGER_STRUCT__
#define  __POWERMANAGER_STRUCT__

#include <cairo-dock.h>
#ifdef CD_UPOWER_AVAILABLE
#include <upower.h>
#endif

typedef enum _CDPowermanagerDisplayType {
	CD_POWERMANAGER_GAUGE=0,
	CD_POWERMANAGER_GRAPH,
	CD_POWERMANAGER_ICONS,
	CD_POWERMANAGER_NB_TYPES
	} CDPowermanagerDisplayType; 

typedef enum {
	POWER_MANAGER_NOTHING = 0,
	POWER_MANAGER_CHARGE,
	POWER_MANAGER_TIME,
	POWER_MANAGER_NB_QUICK_INFO_TYPE
  } MyAppletQuickInfoType;


typedef enum {
	POWER_MANAGER_CHARGE_CRITICAL = 0,
	POWER_MANAGER_CHARGE_LOW,
	POWER_MANAGER_CHARGE_FULL,
	POWER_MANAGER_NB_CHARGE_LEVEL,
	} MyAppletCharge;

	
struct _AppletConfig {
	gchar *defaultTitle;
	MyAppletQuickInfoType quickInfoType;
	gint iCheckInterval;
	
	CDPowermanagerDisplayType iDisplayType;
	CairoDockTypeGraph iGraphType;
	gdouble fLowColor[3];
	gdouble fHigholor[3];
	gdouble fBgColor[4];
	
	gint iNotificationType;
	gchar *cNotificationAnimation;
	gint iNotificationDuration;
	gboolean highBatteryWitness;
	gboolean lowBatteryWitness;
	gboolean criticalBatteryWitness;
	gint lowBatteryValue;
	gchar *cGThemePath;
	gchar *cSoundPath[POWER_MANAGER_NB_CHARGE_LEVEL];
	
	gdouble fLastDischargeMeanRate;
	gdouble fLastChargeMeanRate;
	
	gchar *cUserBatteryIconName;
	gchar *cUserChargeIconName;
	gchar *cEmblemIconName;
	gboolean bHideNotOnBattery;
} ;


#ifdef CD_UPOWER_AVAILABLE
typedef struct {
	UpClient *pUPowerClient;
	GList *pBatteryDeviceList;
	} CDSharedMemory;
#endif

struct _AppletData {
	CairoDockTask *pTask;  // async task to find the available backend (launched on startup)
	
	// UPower
	#ifdef CD_UPOWER_AVAILABLE
	UpClient *pUPowerClient;
	GList *pSignalIDList;  // SID for the "battery properties changed" signal of the UPower battery device
	gint iSignalIDAdded;
	gint iSignalIDRemoved;
	#endif
	GList *pBatteryDeviceList;
	
	// ACPI
	gchar *cBatteryStateFilePath;  // path to the ACPI file
	gboolean bProcAcpiFound;  // TRUE if in /proc/acpi (old format)
	gboolean bSysClassFound;  // TRUE if in /sys/class (new format)
	gint checkLoop;  // SID of the check loop is no UPower
	
	// Properties
	gchar *cTechnology;
	gchar *cVendor;
	gchar *cModel;
	gdouble fMaxAvailableCapacity;
	
	gint iTime;
	gint iPercentage;
	gboolean bOnBattery;
	gboolean bBatteryPresent;
	gint iPrevTime;
	gint iPrevPercentage;
	gboolean bPrevOnBattery;
	gboolean bIsHidden;
	
	gint iCapacity;
	
	gdouble fChargeMeanRate;
	gint iNbChargeMeasures;
	gdouble fDischargeMeanRate;
	gint iNbDischargeMeasures;
	
	gint iStatPercentage;
	gint iStatPercentageBegin;
	gint iStatTime;
	gint iStatTimeCount;
	
	gint iOnBatteryImage;  // -1 = no image yet, 0 = charging, 1 = on battery
	} ;


#endif
