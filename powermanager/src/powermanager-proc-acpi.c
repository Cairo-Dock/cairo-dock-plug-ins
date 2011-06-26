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
#include "powermanager-proc-acpi.h"

#define CD_BATTERY_DIR "/proc/acpi/battery"
//#define CD_BATTERY_DIR "/home/fab/proc/acpi/battery"

/*present: yes
capacity state: ok
charging state: discharging
present rate: 15000 mW
remaining capacity: 47040 mWh
present voltage: 15000 mV*/

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
		cBatteryName = g_dir_read_name (dir);  // usually "BAT0".
		if (cBatteryName == NULL)
			break ;
		
		// check the battery info.
		g_string_printf (sBatteryInfoFilePath, "%s/%s/info", cBatteryPath, cBatteryName);
		length=0;
		cd_debug ("  examen de la batterie '%s' ...", sBatteryInfoFilePath->str);
		g_file_get_contents (sBatteryInfoFilePath->str, &cContent, &length, NULL);
		if (cContent != NULL)
		{
			gchar *str = strchr (cContent, '\n');  // first line: "present:    yes"
			if (str != NULL)
			{
				gchar *str2 = strchr (str+1, ':');
				if (str2 != NULL)
				{
					str2 ++;
					myData.iCapacity = atoi (str2);
					
					gchar *str3 = strchr (str2, ':');
					if (str3 != NULL)  // prefer the last full capacity if available.
					{
						str3 ++;
						myData.iCapacity = atoi (str3);
					}
					
					cd_debug ("Capacity : %d mWsh\n", myData.iCapacity);
					myData.cBatteryStateFilePath = g_strdup_printf ("%s/%s/state", cBatteryPath, cBatteryName);
					bBatteryFound = TRUE;
				}
			}
			g_free (cContent);
		}
	}
	while (! bBatteryFound);
	g_dir_close (dir);
	return bBatteryFound;
}
gboolean cd_find_battery_proc_acpi (void)
{
	gboolean bBatteryFound = _find_battery_in_dir (CD_BATTERY_DIR);
	return bBatteryFound;
}


#define go_to_next_line \
	cCurLine = strchr (cCurVal, '\n'); \
	g_return_val_if_fail (cCurLine != NULL, FALSE); \
	cCurLine ++; \
	cCurVal = cCurLine;

#define jump_to_value \
	cCurVal = strchr (cCurLine, ':'); \
	g_return_val_if_fail (cCurVal != NULL, FALSE); \
	cCurVal ++; \
	while (*cCurVal == ' ') \
		cCurVal ++;

gboolean cd_get_stats_from_proc_acpi (void)
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
	gchar *cCurLine = cContent, *cCurVal = cContent;
	
	//\_______________ check the battery presence.
	jump_to_value  // "present: yes"
	gboolean bBatteryPresent = (*cCurVal == 'y');
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
		cd_debug ("la batterie a ete connectee\n");
		myData.iPrevTime = 0;
		myData.iPrevPercentage = 0;
		for (k = 0; k < PM_NB_VALUES; k ++)
			myData.fRateHistory[k] = 0;
		myData.iCurrentIndex = 0;
		myData.iIndexMax = 0;
	}
	
	go_to_next_line  // -> "capacity state: ok"
	
	go_to_next_line  // -> "charging state: discharging"
	
	//\_______________ check 'on battery' state.
	jump_to_value
	gboolean bOnBattery = (*cCurVal == 'd');  // "discharging"
	if (bOnBattery != myData.bOnBattery)  // state changed
	{
		for (k = 0; k < PM_NB_VALUES; k ++)  // reset the history.
			myData.fRateHistory[k] = 0;
		myData.iCurrentIndex = 0;
		myData.iIndexMax = 0;
		myData.bOnBattery = bOnBattery;
	}
	
	go_to_next_line  // -> present rate: 15000 mW
	
	//\_______________ get the current charge and rate (this one can be 0 if not available).
	jump_to_value
	double fPresentRate = atoi (cCurVal);  // 15000 mW OU 1400 mA
	
	/*cCurVal ++;
	while (*cCurVal != ' ')
		cCurVal ++;
	while (*cCurVal == ' ')
		cCurVal ++;
	if (*cCurVal != 'm')
		cd_warning ("PowerManager : expecting mA or mW as the present rate unit");
	cCurVal ++;
	if (*cCurVal == 'W')
		bWatt = TRUE;
	else if (*cCurVal == 'A')
		bWatt = FALSE;
	else
		cd_warning ("PowerManager : expecting A or W as the present rate unit");*/
	
	go_to_next_line  // -> "remaining capacity: 47040 mWh"
	
	jump_to_value
	int iRemainingCapacity = atoi (cCurVal);  // 47040 mWh
	
	/**go_to_next_line  // -> "present voltage: 15000 mV"
	
	jump_to_value
	int iPresentVoltage = atoi (cCurVal);  // 15000 mV
	*/
	myData.iPercentage = 100. * iRemainingCapacity / myData.iCapacity;
	cd_debug ("myData.iPercentage : %.2f%% (%d / %d)", (double)myData.iPercentage, iRemainingCapacity, myData.iCapacity);
	if (myData.iPercentage > 100)
		myData.iPercentage = 100;
	if (myData.iPercentage < 0)
		myData.iPercentage = 0.;
	
	//\_______________ compute the variation rate if not available in the file.
	if (fPresentRate == 0)
		fPresentRate = cd_compute_current_rate ();
	else
		cd_message ("found fPresentRate = %.2f\n", fPresentRate);
	
	//\_______________ store this value in conf if it has changed too much.
	if (fPresentRate > 0)
	{
		cd_store_current_rate (fPresentRate);
	}
	else if (myData.bOnBattery || myData.iPercentage < 99.9)  // if we are on sector and fully charged, the rate is of course 0.
	{
		cd_debug ("no rate, using last know values : %.2f ; %.2f\n", myConfig.fLastDischargeMeanRate, myConfig.fLastChargeMeanRate);
		fPresentRate = (myData.bOnBattery ? myConfig.fLastDischargeMeanRate : myConfig.fLastChargeMeanRate);
	}
	
	//\_______________ now compute the time.
	myData.iTime = cd_compute_time (fPresentRate, iRemainingCapacity);
	
	//cd_message ("PowerManager : On Battery:%d ; iCapacity:%dmWh ; iRemainingCapacity:%dmWh ; fPresentRate:%.2fmW ; iPresentVoltage:%dmV", myData.bOnBattery, myData.iCapacity, iRemainingCapacity, fPresentRate, iPresentVoltage); 
	g_free (cContent);
	return (TRUE);
}
