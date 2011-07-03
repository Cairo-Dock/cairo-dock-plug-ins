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

#include <math.h>
#include <string.h>
#include <dirent.h>
#include <dbus/dbus-glib.h>

#include "powermanager-draw.h"
#include "powermanager-struct.h"
#include "powermanager-common.h"
#include "powermanager-sys-class.h"

#define CD_BATTERY_DIR "/sys/class/power_supply"

/*POWER_SUPPLY_NAME=BAT1
POWER_SUPPLY_STATUS=Discharging
POWER_SUPPLY_PRESENT=1
POWER_SUPPLY_TECHNOLOGY=Li-ion
POWER_SUPPLY_CYCLE_COUNT=0
POWER_SUPPLY_VOLTAGE_MIN_DESIGN=11100000
POWER_SUPPLY_VOLTAGE_NOW=11100000
POWER_SUPPLY_CURRENT_NOW=0
POWER_SUPPLY_CHARGE_FULL_DESIGN=2200000
POWER_SUPPLY_CHARGE_FULL=2200000
POWER_SUPPLY_CHARGE_NOW=1760000
POWER_SUPPLY_MODEL_NAME=
POWER_SUPPLY_MANUFACTURER=DELL
POWER_SUPPLY_SERIAL_NUMBER=11
*/

static gboolean _find_battery_in_dir (const gchar *cBatteryPath)
{
	// open the folder containing battery data.
	GDir *dir = g_dir_open (cBatteryPath, 0, NULL);
	if (dir == NULL)
	{
		cd_debug ("powermanager: no battery in %s",cBatteryPath );
		return FALSE;
	}
	
	// parse the folder and search the battery files.
	GString *sBatteryInfoFilePath = g_string_new ("");
	gchar *cContent = NULL, *cPresentLine;
	gsize length=0;
	const gchar *cBatteryName;
	gboolean bBatteryFound = FALSE;
	do
	{
		cBatteryName = g_dir_read_name (dir);  // usually "BAT0" or "BAT1".
		if (cBatteryName == NULL)
			break ;
		
		// check the battery type.
		g_string_printf (sBatteryInfoFilePath, "%s/%s/type", cBatteryPath, cBatteryName);
		length=0;
		cd_debug ("  examen de la batterie '%s' ...", sBatteryInfoFilePath->str);
		g_file_get_contents (sBatteryInfoFilePath->str, &cContent, &length, NULL);
		if (cContent != NULL && strncmp (cContent, "Battery", 7) == 0)  // there may be a \n at the end, so let's ignore it.
		{
			myData.cBatteryStateFilePath = g_strdup_printf ("%s/%s/uevent", cBatteryPath, cBatteryName);
			bBatteryFound = TRUE;  // get the capacity when we read the uevent for the first time.
			cd_debug ("  myData.cBatteryStateFilePath: %s", myData.cBatteryStateFilePath);
		}
		g_free (cContent);
	}
	while (! bBatteryFound);
	g_dir_close (dir);
	return bBatteryFound;
}
gboolean cd_find_battery_sys_class (void)
{
	gboolean bBatteryFound = _find_battery_in_dir (CD_BATTERY_DIR);
	return bBatteryFound;
}

#define _get_static_value(s, x) \
str = strstr (cContent, s);\
if (str) { \
	str += strlen(s)+1;\
	cr = strchr (str, '\n');\
	if (cr) x = g_strndup (str, cr - str);\
	else x = g_strdup (str); }

gboolean cd_get_stats_from_sys_class (void)
{
	//\_______________ get the content of the stats file.
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents (myData.cBatteryStateFilePath, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("powermanager : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		return FALSE;
	}
	g_return_val_if_fail (cContent != NULL, FALSE);
	
	int k;
	
	//\_______________ check 'on battery' state.
	gchar *str = strstr (cContent, "STATUS");
	g_return_val_if_fail (str != NULL, FALSE);
	str += 7;
	gboolean bOnBattery = (*str == 'D');  // "Discharging"
	if (bOnBattery != myData.bOnBattery)  // state changed
	{
		/**for (k = 0; k < PM_NB_VALUES; k ++)  // reset the history.
			myData.fRateHistory[k] = 0;
		myData.iCurrentIndex = 0;
		myData.iIndexMax = 0;*/
		myData.iStatPercentageBegin = 0;
		myData.iStatPercentage = 0;
		myData.bOnBattery = bOnBattery;
	}
	
	//\_______________ check the battery presence.
	str = strstr (cContent, "PRESENT");
	g_return_val_if_fail (str != NULL, FALSE);
	str += 8;
	gboolean bBatteryPresent = (*str == '1');
	if (bBatteryPresent != myData.bBatteryPresent)  // the battery has just been inserted/removed. 
	{
		myData.bBatteryPresent = bBatteryPresent;
		if (! bBatteryPresent)  // if the battery has been removed, we are obviously on the sector.
		{
			cd_debug ("la batterie a ete enlevee\n");
			myData.bOnBattery = FALSE;
			update_icon();
			g_free (cContent);
			return TRUE;
		}
		
		// reset the history.
		cd_debug ("la batterie a ete connectee");
		myData.iPrevTime = 0;
		myData.iPrevPercentage = 0;
		/**for (k = 0; k < PM_NB_VALUES; k ++)
			myData.fRateHistory[k] = 0;
		myData.iCurrentIndex = 0;
		myData.iIndexMax = 0;*/
		myData.iStatPercentageBegin = 0;
		myData.iStatPercentage = 0;
	}
	
	//\_______________ get the current charge.
	if (myData.iCapacity == 0)  // not yet got
	{
		str = strstr (cContent, "CHARGE_FULL=");
		g_return_val_if_fail (str != NULL, FALSE);
		str += 12;
		myData.iCapacity = atoi (str);
		g_return_val_if_fail (myData.iCapacity != 0, FALSE);
		// also get few other static params.
		gchar *cr;
		_get_static_value ("TECHNOLOGY", myData.cTechnology)
		_get_static_value ("MANUFACTURER", myData.cVendor)
		_get_static_value ("MODEL_NAME", myData.cModel)
		str = strstr (cContent, "FULL_DESIGN");
		if (str)
		{
			str += 12;
			int iMaxCapacity = atoi (str);
			if (iMaxCapacity != 0)
				myData.fMaxAvailableCapacity = 100. * myData.iCapacity / iMaxCapacity;
		}
	}
	
	str = strstr (cContent, "CHARGE_NOW");
	g_return_val_if_fail (str != NULL, FALSE);
	str += 11;
	int iRemainingCapacity = atoi (str);
	
	myData.iPercentage = 100. * iRemainingCapacity / myData.iCapacity;
	cd_debug ("myData.iPercentage : %.2f%% (%d / %d)", (double)myData.iPercentage, iRemainingCapacity, myData.iCapacity);
	if (myData.iPercentage > 100)
		myData.iPercentage = 100;
	if (myData.iPercentage < 0)
		myData.iPercentage = 0.;
	
	//\_______________ now compute the time.
	myData.iTime = cd_estimate_time ();
	
	g_free (cContent);
	return (TRUE);
}
