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

#define WEATHER_NB_DAYS_MAX 8

#define WEATHER_RATIO_ICON_DESKLET .5

#define WEATHER_DEFAULT_NAME "weather"

// https://geocoding-api.open-meteo.com/v1/search should work to retrieve location data, but not implemented for now
#undef CD_WEATHER_HAS_CODE_LOCATION

#define _display(cValue) ((cValue) == NULL || *((gchar*)cValue) == 'N' ? "?" : (const gchar*)(cValue))

struct _AppletConfig {
	double lat;
	double lon;
	gboolean bISUnits;
	gboolean bCurrentConditions;
	gboolean bDisplayTemperature;
	gint iNbDays;
	gchar *cRenderer;
	gint cDialogDuration;
	gchar *cThemePath;
	gboolean bDesklet3D;
	gboolean bSetName;
	} ;

typedef struct {
	char *cTemp;
	char *cSpeed;
	char *cPressure;
	char *cHumidity;
	} Unit;

typedef struct {
	const char *cIconNumber;
	char *cDate;
	char *cName;
	char *cTempMax;
	char *cTempMin;
	char *cPrecipProb;
	char *cWeatherDescription;
	} Day;

typedef struct {
	char *cWindSpeed;
	char *cWindDirection;
	char *cPressure;
	char *cHumidity;
	char *cSunRise;
	char *cSunSet;
	int ttl;  // in mn;
	Day now;
	} CurrentContitions;

typedef struct {
	Unit units;
	CurrentContitions currentConditions;
	Day days[WEATHER_NB_DAYS_MAX];
	char *cCountry;
	char *cCity;
	char *cLink;
	} CDWeatherData;

typedef struct {
	double lat; // location
	double lon;
	gboolean bISUnits;
	gboolean bCurrentConditions;
	CDWeatherData wdata;
	gboolean bErrorInThread;
	GldiModuleInstance *pApplet;
	} CDSharedMemory;

struct _AppletData {
	CDWeatherData wdata;
	GldiTask *pTask;
	gboolean bErrorRetrievingData;
	GList *pLocationsList;
	gboolean bSetName;
	gint iClickedDay;
	GldiTask *pGetLocationTask;
	gboolean bBusy;
	GtkWidget *pCodeEntry;
	} ;


#endif
