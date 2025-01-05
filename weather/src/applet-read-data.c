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

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>  // rand

#include <json.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"


static const char *_cd_weather_get_icon_number (int	code, gboolean bNight)
{
	switch (code)
	{
		case 0:
			// Clear sky
			return bNight ? "31" : "32";
		case 1:
			// Mainly clear
			return bNight ? "29" : "30";
		case 2:
			// Partly cloudy
			return bNight ? "27" : "28";
		case 3:
			// overcast
			return "26";
		case 45:
			// fog, "sky invisible"
			return "20";
		case 48:
			// depositing rime fog, "sky visible"
			return bNight ? "33" : "34";
		case 51:
			// light drizzle
			return "9";
		case 53:
			// moderate drizzle
			return "11";
		case 55:
			// dense drizzle
			return "12";
		case 56:
			// light freezing drizzle
			return "8";
		case 57:
			// dense freezing drizzle
			return "10";
		case 61:
			// slight rain
			return "5";
		case 63:
			// moderate rain
			return "11";
		case 65:
			// heavy rain
			return "40";
		case 66:
			// light freezing rain
			return "8";
		case 67:
			// heavy freezing rain
			return "10";
		case 71:
			// slight snow
			return "13";
		case 73:
			// moderate snow
			return "14";
		case 75:
			// heavy snow
			return "16";
		case 77:
			// snow grains
			return "15";
		case 80:
		case 81:
		case 82:
			// Rain showers: Slight, moderate, and violent
			return bNight ? "45" : "39";
		case 85:
		case 86:
			// Snow showers slight and heavy
			return bNight ? "46" : "42";
		case 95:
			// Thunderstorm: Slight or moderate
			return bNight ? "47" : "37";
		case 96:
		case 99:
			// Thunderstorm with slight and heavy hail
			return "35";
		default:
			cd_warning ("Unknown weather code: %d", code);
			return "na";
	}
}


static void _cd_weather_parse_data (CDSharedMemory *pSharedMemory, const gchar *cData, gboolean bParseHeader, GError **erreur)
{
	struct json_object *obj = json_tokener_parse (cData);
	GTimeZone *tz = NULL;
	
	if (!obj) goto json_error;
	
	{
		struct json_object *tmp = json_object_object_get (obj, "timezone"); if (!tmp) goto json_error;
		const char *tmp2 = json_object_get_string (tmp); if (!tmp2) goto json_error;
		tz = g_time_zone_new_identifier (tmp2);
	}
	
	struct json_object *units = json_object_object_get (obj, "current_units");
	if (!units) goto json_error;
	
	{
		struct json_object *tmp = json_object_object_get (units, "temperature_2m");
		if (!tmp) goto json_error;
		pSharedMemory->wdata.units.cTemp = g_strdup (json_object_get_string (tmp));
		tmp = json_object_object_get (units, "wind_speed_10m"); if (!tmp) goto json_error;
		pSharedMemory->wdata.units.cSpeed = g_strdup (json_object_get_string (tmp));
		tmp = json_object_object_get (units, "surface_pressure"); if (!tmp) goto json_error;
		pSharedMemory->wdata.units.cPressure = g_strdup (json_object_get_string (tmp));
		tmp = json_object_object_get (units, "relative_humidity_2m"); if (!tmp) goto json_error;
		pSharedMemory->wdata.units.cHumidity = g_strdup (json_object_get_string (tmp));
	}
	
	struct json_object *current = json_object_object_get (obj, "current");
	if (!current) goto json_error;
	
	int current_code = -1;
	{
		struct json_object *tmp = json_object_object_get (current, "temperature_2m");
		if (!tmp || !(json_object_is_type (tmp, json_type_double) || json_object_is_type (tmp, json_type_int))) goto json_error;
		pSharedMemory->wdata.currentConditions.now.cTempMax = g_strdup_printf ("%.0f", json_object_get_double (tmp));
		tmp = json_object_object_get (current, "wind_direction_10m");
		if (!tmp || !(json_object_is_type (tmp, json_type_double) || json_object_is_type (tmp, json_type_int))) goto json_error;
		pSharedMemory->wdata.currentConditions.cWindDirection = g_strdup_printf ("%.0f", json_object_get_double (tmp));
		tmp = json_object_object_get (current, "wind_speed_10m");
		if (!tmp || !(json_object_is_type (tmp, json_type_double) || json_object_is_type (tmp, json_type_int))) goto json_error;
		pSharedMemory->wdata.currentConditions.cWindSpeed = g_strdup_printf ("%.0f", json_object_get_double (tmp));
		tmp = json_object_object_get (current, "relative_humidity_2m");
		if (!tmp || !(json_object_is_type (tmp, json_type_double) || json_object_is_type (tmp, json_type_int))) goto json_error;
		pSharedMemory->wdata.currentConditions.cHumidity = g_strdup_printf ("%.0f", json_object_get_double (tmp));
		tmp = json_object_object_get (current, "surface_pressure");
		if (!tmp || !(json_object_is_type (tmp, json_type_double) || json_object_is_type (tmp, json_type_int))) goto json_error;
		pSharedMemory->wdata.currentConditions.cPressure = g_strdup_printf ("%.0f", json_object_get_double (tmp));
		tmp = json_object_object_get (current, "weather_code");
		if (!tmp || !(json_object_is_type (tmp, json_type_int))) goto json_error;
		current_code = json_object_get_int (tmp); // will be processed later (once we know if it is night or day)
		
		tmp = json_object_object_get (current, "time"); if (!tmp) goto json_error;
		const char *datetime = json_object_get_string (tmp); if (!datetime) goto json_error;
		char *str2 = g_strdup_printf ("%s:00", datetime); // GDateTime needs the seconds part which is not provided here
		GDateTime *dt = g_date_time_new_from_iso8601 (str2, tz);
		g_free (str2);
		if (dt)
		{
			pSharedMemory->wdata.currentConditions.now.cDate = g_date_time_format (dt, "%c");
			g_date_time_unref (dt);
		}
		if (!pSharedMemory->wdata.currentConditions.now.cDate) pSharedMemory->wdata.currentConditions.now.cDate = g_strdup (datetime);
	}
	
	struct json_object *daily = json_object_object_get (obj, "daily");
	if (!daily) goto json_error;
	gboolean bIsCurrentNight = FALSE;
	
	{
		struct json_object *time     = json_object_object_get (daily, "time");
		struct json_object *temp_min = json_object_object_get (daily, "temperature_2m_min");
		struct json_object *temp_max = json_object_object_get (daily, "temperature_2m_max");
		struct json_object *code     = json_object_object_get (daily, "weather_code");
		struct json_object *precip   = json_object_object_get (daily, "precipitation_probability_mean");
		if (! (time && temp_min && temp_max && code && precip) ) goto json_error;
		if (!json_object_is_type (time,     json_type_array) ||
			!json_object_is_type (temp_min, json_type_array) ||
			!json_object_is_type (temp_max, json_type_array) ||
			!json_object_is_type (code,     json_type_array) ||
			!json_object_is_type (precip,   json_type_array))
				goto json_error;
		
		size_t len = json_object_array_length (time);
		if (len != json_object_array_length (temp_min) || len != json_object_array_length (temp_max) ||
			len != json_object_array_length (code)     || len != json_object_array_length (precip))
				goto json_error;
		
		if (len > WEATHER_NB_DAYS_MAX) len = WEATHER_NB_DAYS_MAX;
		size_t i;
		for (i = 0; i < len; i++)
		{
			struct json_object *tmp = json_object_array_get_idx (time, i);
			const char *day = json_object_get_string (tmp); if (!day) goto json_error;
			char *str2 = g_strdup_printf ("%sT00:00:00", day); // GDateTime needs the time of day as well
			GDateTime *dt = g_date_time_new_from_iso8601 (str2, tz);
			g_free (str2);
			if (dt)
			{
				pSharedMemory->wdata.days[i].cDate = g_date_time_format (dt, "%x");
				pSharedMemory->wdata.days[i].cName = g_date_time_format (dt, "%A");
				g_date_time_unref (dt);
			}
			if (!pSharedMemory->wdata.days[i].cDate) pSharedMemory->wdata.days[i].cDate = g_strdup (day);
			
			tmp = json_object_array_get_idx (temp_min, i);
			if (! (json_object_is_type (tmp, json_type_double) || json_object_is_type (tmp, json_type_int))) goto json_error;
			pSharedMemory->wdata.days[i].cTempMin = g_strdup_printf ("%.0f", json_object_get_double (tmp));
			tmp = json_object_array_get_idx (temp_max, i);
			if (! (json_object_is_type (tmp, json_type_double) || json_object_is_type (tmp, json_type_int))) goto json_error;
			pSharedMemory->wdata.days[i].cTempMax = g_strdup_printf ("%.0f", json_object_get_double (tmp));
			tmp = json_object_array_get_idx (precip, i);
			if (! (json_object_is_type (tmp, json_type_double) || json_object_is_type (tmp, json_type_int))) goto json_error;
			pSharedMemory->wdata.days[i].cPrecipProb = g_strdup_printf ("%.0f %%", json_object_get_double (tmp));
			
			tmp = json_object_array_get_idx (code, i);
			if (!json_object_is_type (tmp, json_type_int)) goto json_error;
			pSharedMemory->wdata.days[i].cIconNumber = _cd_weather_get_icon_number (json_object_get_int (tmp), FALSE); // always use the day version
		}
		
		// sunrise and sunset, only for the first day
		const char *sunrise = NULL;
		const char *sunset = NULL;
		struct json_object *tmp = json_object_object_get (daily, "sunrise");
		if (!tmp || !json_object_is_type (tmp, json_type_array)) goto json_error;
		tmp = json_object_array_get_idx (tmp, 0); if (!tmp) goto json_error;
		sunrise = json_object_get_string (tmp);
		tmp = json_object_object_get (daily, "sunset");
		if (!tmp || !json_object_is_type (tmp, json_type_array)) goto json_error;
		tmp = json_object_array_get_idx (tmp, 0); if (!tmp) goto json_error;
		sunset = json_object_get_string (tmp);
		
		if (!(sunrise && sunset)) goto json_error;
		
		// try to check if it is night currently
		{
			char *str2 = g_strdup_printf ("%s:00", sunrise); // GDateTime needs the seconds part which is not provided here
			GDateTime *dt_sunrise = g_date_time_new_from_iso8601 (str2, tz);
			g_free (str2);
			str2 = g_strdup_printf ("%s:00", sunset);
			GDateTime *dt_sunset = g_date_time_new_from_iso8601 (str2, tz);
			g_free (str2);
			
			if (dt_sunrise && dt_sunset)
			{
				GDateTime *dt_now = g_date_time_new_now (tz);
				if (dt_now)
				{
					if (g_date_time_compare (dt_now, dt_sunrise) < 0 || g_date_time_compare (dt_now, dt_sunset) > 0) bIsCurrentNight = TRUE;
					g_date_time_unref (dt_now);
				}
			}
			if (dt_sunrise) g_date_time_unref (dt_sunrise);
			if (dt_sunset) g_date_time_unref (dt_sunset);
		}
		
		sunrise = strchr (sunrise, 'T');
		if (!sunrise || !sunrise[1]) goto json_error;
		pSharedMemory->wdata.currentConditions.cSunRise = g_strdup (sunrise + 1); // only the time part, should be in local time
		sunset = strchr (sunset, 'T');
		if (!sunset || !sunset[1]) goto json_error;
		pSharedMemory->wdata.currentConditions.cSunSet = g_strdup (sunset + 1); // only the time part, should be in local time
	}
	
	pSharedMemory->wdata.currentConditions.now.cIconNumber = _cd_weather_get_icon_number (current_code, bIsCurrentNight);
	
	json_object_put (obj);
	if (tz) g_time_zone_unref (tz);
	return;
	
json_error:
	cd_warning ("Error parsing weather data!");
	if (obj) json_object_put (obj);
	if (tz) g_time_zone_unref (tz);
}

const gchar *cBaseUrl = "https://api.open-meteo.com/v1/forecast";
	
static void cd_weather_get_distant_data (CDSharedMemory *pSharedMemory)
{
	pSharedMemory->bErrorInThread = FALSE;
	
	if (isnan (pSharedMemory->lat) || isnan (pSharedMemory->lon))
	{
		cd_warning ("No location");
		pSharedMemory->bErrorInThread = TRUE;
		return;
	}
	
	
	//\____________________ get the weather data
	GError *erreur = NULL;
	gchar *cURL = g_strdup_printf("%s?latitude=%f&longitude=%f"
		"&temperature_unit=%s&wind_speed_unit=%s&precipitation_unit%s&timezone=auto"
		"&current=temperature_2m,relative_humidity_2m,wind_speed_10m,wind_direction_10m,surface_pressure,weather_code"
		"&daily=temperature_2m_min,temperature_2m_max,weather_code,sunrise,sunset,precipitation_probability_mean"
		"&forecast_days=8",
		cBaseUrl, pSharedMemory->lat, pSharedMemory->lon,
		pSharedMemory->bISUnits ? "celsius" : "fahrenheit",
		pSharedMemory->bISUnits ? "kmh" : "mph",
		pSharedMemory->bISUnits ? "mm" : "inch");
	
	gchar *cCCData = cairo_dock_get_url_data (cURL, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("while downloading current conditions data:\n%s -> %s", cURL, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	g_free (cURL);
	
	if (cCCData == NULL)
	{
		pSharedMemory->bErrorInThread = TRUE;
		return;
	}
	
	//\____________________ On extrait les donnees des conditions courantes.
	_cd_weather_parse_data (pSharedMemory, cCCData, TRUE, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("weather : %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		pSharedMemory->bErrorInThread = TRUE;
	}
	g_free (cCCData);
}



static void _reset_units (Unit *pUnits)
{
	g_free (pUnits->cTemp);
	g_free (pUnits->cSpeed);
	g_free (pUnits->cPressure);
	g_free (pUnits->cHumidity);
}

static void _reset_current_conditions (CurrentContitions *pCurrentContitions)
{
	g_free (pCurrentContitions->cSunRise);
	g_free (pCurrentContitions->cSunSet);
	g_free (pCurrentContitions->cWindSpeed);
	g_free (pCurrentContitions->cWindDirection);
	g_free (pCurrentContitions->cPressure);
	g_free (pCurrentContitions->cHumidity);
}

static void _reset_one_day (Day *pDay)
{
	g_free (pDay->cName);
	g_free (pDay->cDate);
	g_free (pDay->cTempMax);
	g_free (pDay->cTempMin);
	// g_free (pDay->cIconNumber); -- icon number is statically allocated
}

void cd_weather_reset_weather_data (CDWeatherData *pData)
{
	g_free (pData->cCity);
	g_free (pData->cCountry);
	_reset_units (&pData->units);
	_reset_current_conditions (&pData->currentConditions);
	int i;
	for (i = 0; i < WEATHER_NB_DAYS_MAX; i ++)
	{
		_reset_one_day (&pData->days[i]);
	}
}

void cd_weather_reset_data (GldiModuleInstance *myApplet)
{
	cd_weather_reset_weather_data (&myData.wdata);
}

static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	cd_weather_reset_weather_data (&pSharedMemory->wdata);
	g_free (pSharedMemory);
}
void cd_weather_launch_periodic_task (GldiModuleInstance *myApplet)
{
	if (myData.pTask != NULL)
	{
		gldi_task_discard (myData.pTask);
		myData.pTask = NULL;
	}
	
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	pSharedMemory->bISUnits = myConfig.bISUnits;
	pSharedMemory->bCurrentConditions = myConfig.bCurrentConditions;
	pSharedMemory->pApplet = myApplet;
	pSharedMemory->lat = myConfig.lat;
	pSharedMemory->lon = myConfig.lon;
	
	myData.pTask = gldi_task_new_full (3600,  // start with 1h period, it will be adjusted according to the TTL
		(GldiGetDataAsyncFunc) cd_weather_get_distant_data,
		(GldiUpdateSyncFunc) cd_weather_update_from_data,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	gldi_task_launch (myData.pTask);
}

