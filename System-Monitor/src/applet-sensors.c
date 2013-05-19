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

static int s_iSensorsState = 0;  // on en fait une variable globale plutot qu'un parametre de myData, car la libsensors ne doit etre fermee qu'une seule fois (meme si l'applet est instancee plusieurs fois, le .so du plug-in n'est ouvert une seule fois).

void cd_sysmonitor_clean_sensors (void)
{
	if (s_iSensorsState == 1)
		sensors_cleanup();
	s_iSensorsState = 0;
}

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

static inline void _init_sensors (void)
{
	if (s_iSensorsState == 0)
	{
		int err = sensors_init (NULL);
		if (err != 0)
		{
			s_iSensorsState = -1;
			cd_warning ("couldn't initialize libsensors: %s\nTry running 'sensors-detect' as root in a terminal.", sensors_strerror (err));
			return;
		}
		s_iSensorsState = 1;
	}
}

void cd_sysmonitor_get_sensors_data (GldiModuleInstance *myApplet)
{
	_init_sensors ();
	if (s_iSensorsState != 1)
		return;
	
	const sensors_chip_name *chip;
	const sensors_subfeature *sf;
	double val;
	int chip_nr;
	
	chip_nr = 0;
	double fCpuTempPercentMax = 0;
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
					
					sf = sensors_get_subfeature (chip, feature,
						SENSORS_SUBFEATURE_TEMP_FAULT);
					if (sf && get_value (chip, sf))  // fault
						break;
					
					// valeur
					sf = sensors_get_subfeature (chip, feature,
						SENSORS_SUBFEATURE_TEMP_INPUT);
					if (!sf)
						break;
					val = get_value(chip, sf);
					if (val == 0)
						break;
					
					// alarme
					sf = sensors_get_subfeature (chip, feature,
						SENSORS_SUBFEATURE_TEMP_ALARM);
					if (sf && get_value (chip, sf))
						myData.bCpuTempAlarm = TRUE;
					
					// min limit
					sf = sensors_get_subfeature (chip, feature,
						SENSORS_SUBFEATURE_TEMP_MIN);
					if (sf)
					{
						limit1 = get_value(chip, sf);
						
						sf = sensors_get_subfeature (chip, feature,
							SENSORS_SUBFEATURE_TEMP_MIN_ALARM);
						if (sf && get_value (chip, sf))
							myData.bCpuTempAlarm = TRUE;
					}
					
					// max limit
					sf = sensors_get_subfeature(chip, feature,
						SENSORS_SUBFEATURE_TEMP_MAX);
					if (sf)
					{
						limit2 = get_value (chip, sf);
						
						sf = sensors_get_subfeature (chip, feature,
							SENSORS_SUBFEATURE_TEMP_MAX_ALARM);
						if (sf && get_value (chip, sf))
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
								myData.bCpuTempAlarm = TRUE;
						}
					}
					if (limit2 <= limit1 + 1)
						limit2 = limit1 + 1;
					
					double fCpuTempPercent = 100. * (val - limit1) / (limit2 - limit1);
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
				{
					double min = 0;
					
					sf = sensors_get_subfeature (chip, feature,
						SENSORS_SUBFEATURE_FAN_FAULT);
					if (sf && get_value (chip, sf))  // fault
						break;
					
					// valeur
					sf = sensors_get_subfeature (chip, feature,
						SENSORS_SUBFEATURE_FAN_INPUT);
					if (!sf)
						break;
					val = get_value (chip, sf);  // rpm
					if (val == 0)
						break;
					
					// alarm
					sf = sensors_get_subfeature (chip, feature,
						SENSORS_SUBFEATURE_FAN_MIN);
					if (sf)
						min = get_value(chip, sf);
					
					sf = sensors_get_subfeature (chip, feature,
						SENSORS_SUBFEATURE_FAN_ALARM);
					if (sf && get_value(chip, sf) && val > min)  // on elimine les cas ou le min a une valeur aberrante.
						myData.bFanAlarm = TRUE;
					
					// max speed
					myData.fMaxFanSpeed = 8000.;  // pour l'instant on la laisse en dur a une valeur pas trop bete, car libsensors ne fournit pas de max pour les fans (elle fournit un min, mais sans le max ca a peu d'interet).
					if (val > myData.fMaxFanSpeed)
						val = myData.fMaxFanSpeed;
					
					myData.iFanSpeed = MAX (myData.iFanSpeed, val);  // on ne garde qu'une valeur : la plus grande.
					
					myData.fFanSpeedPercent = 100. * myData.iFanSpeed / myData.fMaxFanSpeed;
				}
				break;
				
				default:
				break;
			}
		}
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


void cd_sysmonitor_get_sensors_info (GldiModuleInstance *myApplet, GString *pInfo)
{
	_init_sensors ();
	if (s_iSensorsState != 1)
		return;
	
	const sensors_chip_name *chip;
	const sensors_subfeature *sf;
	double val;
	int chip_nr;
	
	chip_nr = 0;
	char *label;
	gboolean alarm;
	while ((chip = sensors_get_detected_chips (NULL, &chip_nr)))
	{
		const sensors_feature *feature;
		int i;
		
		i = 0;
		while ((feature = sensors_get_features (chip, &i)))
		{
			label = NULL;
			alarm = 0;
			switch (feature->type)
			{
				case SENSORS_FEATURE_TEMP:  // une sonde de temperature
				{
					// name
					label = sensors_get_label(chip, feature);
					if (!label)
						break;
					
					double limit1=-100, limit2=-100;
					
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
						alarm = TRUE;
					
					// min limit
					sf = sensors_get_subfeature(chip, feature,
						SENSORS_SUBFEATURE_TEMP_MIN);
					if (sf)
					{
						limit1 = get_value(chip, sf);
						
						sf = sensors_get_subfeature(chip, feature,
							SENSORS_SUBFEATURE_TEMP_MIN_ALARM);
						if (sf && get_value(chip, sf))
							alarm = TRUE;
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
							alarm = TRUE;
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
								alarm = TRUE;
						}
					}
					
					//g_print ("CPU : %.2f %d(%d) %.2f\n", limit1, myData.iCPUTemp, myData.bCpuTempAlarm, limit2);
					g_string_append_printf (pInfo, "\n%s: %d°C", label, (int)val);
					if (limit1 > -99)
						g_string_append_printf (pInfo, ", %s: %d°C", D_("min"), (int)limit1);
					if (limit2 > -99)
						g_string_append_printf (pInfo, ", %s: %d°C", D_("max"), (int)limit2);
					
					if (alarm)
						g_string_append_printf (pInfo, "  (%s)", D_("alarm"));
					free(label);
				}
				break;
				
				case SENSORS_FEATURE_FAN:  // un ventilo
					// name
					label = sensors_get_label(chip, feature);
					if (!label)
						break;
					
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
						break;
					
					// alarm
					sf = sensors_get_subfeature (chip, feature,
						SENSORS_SUBFEATURE_FAN_ALARM);
					if (sf && get_value(chip, sf))
						alarm = TRUE;
					
					g_string_append_printf (pInfo, "\n%s: %d %s", label, (int)val, D_("rpm"));
					if (alarm)
						g_string_append_printf (pInfo, "  (%s)", D_("alarm"));
					free(label);
				break;
				
				default:  // les autres ne nous interessent pas.
				break;
			}
		}
	}
}

void cd_cpu_alert (GldiModuleInstance *myApplet)
{
	if (myData.bCPUAlerted || ! myConfig.bAlert)
		return;
	
	gldi_dialogs_remove_on_icon (myIcon);
	gldi_dialog_show_temporary_with_icon_printf (D_("CPU temperature has reached %d°C"), myIcon, myContainer, 4e3, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE, myData.iCPUTemp);
	
	if (myConfig.bAlertSound)
		cairo_dock_play_sound (myConfig.cSoundPath);
	
	myData.bCPUAlerted = TRUE;
}

void cd_fan_alert (GldiModuleInstance *myApplet)
{
	if (myData.bFanAlerted || ! myConfig.bAlert)
		return;
	
	gldi_dialogs_remove_on_icon (myIcon);
	gldi_dialog_show_temporary_with_icon_printf (D_("Fan speed has reached %d rpm"), myIcon, myContainer, 4e3, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE, myData.iFanSpeed);
	
	if (myConfig.bAlertSound)
		cairo_dock_play_sound (myConfig.cSoundPath);
	
	myData.bFanAlerted = TRUE;
}
