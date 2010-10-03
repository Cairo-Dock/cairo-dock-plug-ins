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
*
* Adapted from the 'sensors' program.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>

#include <sensors/sensors.h>
#include <sensors/error.h>

#include "applet-struct.h"
#include "applet-sensors.h"

/**************************************************
it87-isa-0290
Adapter: ISA adapter
VCore 1: +1.57 V (min = +1.42 V, max = +1.57 V) ALARM
VCore 2: +2.66 V (min = +2.40 V, max = +2.61 V) ALARM
+3.3V: +6.59 V (min = +3.14 V, max = +3.46 V) ALARM
+5V: +5.11 V (min = +4.76 V, max = +5.24 V)
+12V: +11.78 V (min = +11.39 V, max = +12.61 V)
-12V: -19.14 V (min = -12.63 V, max = -11.41 V) ALARM
-5V: +0.77 V (min = -5.26 V, max = -4.77 V) ALARM
Stdby: +5.00 V (min = +4.76 V, max = +5.24 V)
VBat: +3.12 V
fan1: 3668 RPM (min = 0 RPM, div = 
fan2: 0 RPM (min = 664 RPM, div =  ALARM
fan3: 0 RPM (min = 2657 RPM, div = 2) ALARM
M/B Temp: +39°C (low = +15°C, high = +40°C) sensor = thermistor
CPU Temp: +36°C (low = +15°C, high = +45°C) sensor = thermistor
Temp3: +96°C (low = +15°C, high = +45°C) sensor = diode
**************************************************/

static double get_value (const sensors_chip_name *name, const sensors_subfeature *sub)
{
	double val;
	int err;

	err = sensors_get_value(name, sub->number, &val);
	if (err) {
		fprintf(stderr, "ERROR: Can't get value of subfeature %s: %s\n",
			sub->name, sensors_strerror(err));
		val = 0;
	}
	return val;
}

void cd_sysmonitor_get_sensors_data (CairoDockModuleInstance *myApplet)
{
	if (myData.iSensorsState == 0)
	{
		int err = sensors_init (NULL);
		if (err != 0)
		{
			myData.iSensorsState = -1;
			cd_warning ("couldn't initialize libsensors: %s\nTry running 'sensors-detect' as root in a terminal.", sensors_strerror (err));
			return;
		}
		myData.iSensorsState = 1;
	}
	
	if (myData.iSensorsState != 1)
		return;
	
	const sensors_chip_name *chip;
	const sensors_subfeature *sf;
	double val;
	int chip_nr;
	
	chip_nr = 0;
	double fCpuTempPercent, fCpuTempPercentMax = 0;
	myData.iFanSpeed = 0;
	myData.iCPUTemp = 0;
	myData.bCpuTempAlarm = FALSE;
	myData.bFanAlarm = FALSE;
	while ((chip = sensors_get_detected_chips (NULL, &chip_nr)))
	{
		const sensors_feature *feature;
		int i;
		
		i = 0;
		while ((feature = sensors_get_features (chip, &i)))
		{
			switch (feature->type)
			{
				case SENSORS_FEATURE_TEMP:  // une sonde de temperature
				{
					double limit1=0, limit2=100;
					
					sf = sensors_get_subfeature(chip, feature,
						SENSORS_SUBFEATURE_TEMP_FAULT);
					if (sf && get_value(chip, sf))  // fault
						break;
					
					// valeur
					sf = sensors_get_subfeature(chip, feature,
						SENSORS_SUBFEATURE_TEMP_INPUT);
					if (!sf)
						break;
					val = get_value(chip, sf);
					if (val == 0)
						break;
					
					// alarme
					sf = sensors_get_subfeature(chip, feature,
						SENSORS_SUBFEATURE_TEMP_ALARM);
					if (sf && get_value(chip, sf))
						myData.bCpuTempAlarm = TRUE;
					
					// min limit
					sf = sensors_get_subfeature(chip, feature,
						SENSORS_SUBFEATURE_TEMP_MIN);
					if (sf)
					{
						limit1 = get_value(chip, sf);
						
						sf = sensors_get_subfeature(chip, feature,
							SENSORS_SUBFEATURE_TEMP_MIN_ALARM);
						if (sf && get_value(chip, sf))
							myData.bCpuTempAlarm = TRUE;
					}
					
					// max limit
					sf = sensors_get_subfeature(chip, feature,
						SENSORS_SUBFEATURE_TEMP_MAX);
					if (sf)
					{
						limit2 = get_value(chip, sf);
						
						sf = sensors_get_subfeature(chip, feature,
							SENSORS_SUBFEATURE_TEMP_MAX_ALARM);
						if (sf && get_value(chip, sf))
							myData.bCpuTempAlarm = TRUE;
					}
					else  // pas de valeur max, on regarde si une valeur critique existe.
					{
						sf = sensors_get_subfeature(chip, feature,
							SENSORS_SUBFEATURE_TEMP_CRIT);
						if (sf)
						{
							limit2 = get_value(chip, sf);
							
							sf = sensors_get_subfeature(chip, feature,
								SENSORS_SUBFEATURE_TEMP_CRIT_ALARM);
							if (sf && get_value(chip, sf))
								myData.bCpuTempAlarm |= 1;
						}
					}
					if (limit2 <= limit1 + 1)
						limit2 = limit1 + 1;
					
					double fCpuTempPercent = 100. * (limit2 - val) / (limit2 - limit1);
					if (fCpuTempPercent > fCpuTempPercentMax)  // on ne va garder qu'une seule valeur : celle qui est la plus grande en valeur relative.
					{
						fCpuTempPercentMax = fCpuTempPercent;
						myData.fCpuTempPercent = fCpuTempPercent;
						myData.iCPUTemp = val;
						myData.iCPUTempMin = limit1;
						myData.iCPUTempMax = limit2;
					}
					//g_print ("CPU : %.2f %d(%d) %.2f\n", limit1, myData.iCPUTemp, myData.bCpuTempAlarm, limit2);
				}
				break;
				
				case SENSORS_FEATURE_FAN:  // un ventilo
					sf = sensors_get_subfeature (chip, feature,
						SENSORS_SUBFEATURE_FAN_FAULT);
					if (sf && get_value(chip, sf))  // fault
						break;
					
					// valeur
					sf = sensors_get_subfeature (chip, feature,
						SENSORS_SUBFEATURE_FAN_INPUT);
					if (!sf)
						break;
					val = get_value (chip, sf);  // rpm
					if (val == 0)
						return;
					
					// alarm
					sf = sensors_get_subfeature (chip, feature,
						SENSORS_SUBFEATURE_FAN_ALARM);
					if (sf && get_value(chip, sf))
						myData.bFanAlarm = TRUE;
					
					// max speed
					myData.fMaxFanSpeed = 8000.;  // pour l'instant on la laisse en dur a une valeur pas trop bete, car libsensors ne fournit pas de max pour les fans (elle fournit un min, mais sans le max ca a peu d'interet).
					if (val > myData.fMaxFanSpeed)
						val = myData.fMaxFanSpeed;
					
					myData.iFanSpeed = MAX (myData.iFanSpeed, val);  // on ne garde qu'une valeur : la plus grande.
					
					myData.fFanSpeedPercent = 100. * myData.iFanSpeed / myData.fMaxFanSpeed;
				break;
				
				case SENSORS_FEATURE_IN:
				case SENSORS_FEATURE_VID:
				case SENSORS_FEATURE_BEEP_ENABLE:
				case SENSORS_FEATURE_POWER:
				case SENSORS_FEATURE_ENERGY:
				case SENSORS_FEATURE_CURR:
				default:
				break;
			}
		}
	}
	
	if (myData.iFanSpeed == 0 && myData.iCPUTemp == 0) {
		cd_warning("couldn't acquire sensors data\nrun 'sensors-detect' as root in a terminal.");
		myData.bAcquisitionOK = FALSE;
	}
	
	if (fabs (myData.fCpuTempPercent - myData.fPrevCpuTempPercent) > 1)
	{
		myData.fPrevCpuTempPercent = myData.fCpuTempPercent;
		myData.bNeedsUpdate = TRUE;
	}
	if (fabs (myData.fFanSpeedPercent - myData.fPrevFanSpeedPercent) > 1)
	{
		myData.fPrevFanSpeedPercent = myData.fFanSpeedPercent;
		myData.bNeedsUpdate = TRUE;
	}
}


void cd_sysmonitor_get_sensors_info (CairoDockModuleInstance *myApplet)
{
	
}

void cd_cpu_alert (CairoDockModuleInstance *myApplet)
{
	if (myData.bCPUAlerted || ! myConfig.bAlert)
		return;
	
	cairo_dock_show_temporary_dialog_with_icon_printf (D_("CPU temperature has reached %d°C"), myIcon, myContainer, 4e3, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE, myData.iCPUTemp);
	
	if (myConfig.bAlertSound)
		cairo_dock_play_sound (myConfig.cSoundPath);
	
	myData.bCPUAlerted = TRUE;
}

void cd_fan_alert (CairoDockModuleInstance *myApplet)
{
	if (myData.bFanAlerted || ! myConfig.bAlert)
		return;
	
	cairo_dock_show_temporary_dialog_with_icon_printf (D_("Fan speed has reached %d rpm"), myIcon, myContainer, 4e3, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE, myData.iFanSpeed);
	
	if (myConfig.bAlertSound)
		cairo_dock_play_sound (myConfig.cSoundPath);
	
	myData.bFanAlerted = TRUE;
}
