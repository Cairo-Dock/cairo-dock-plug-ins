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

#define WEATHER_NB_DAYS_MAX 5

#define WEATHER_RATIO_ICON_DESKLET .5

#define WEATHER_DEFAULT_NAME "weather"

#define _display(cValue) ((cValue) == NULL || *((gchar*)cValue) == 'N' ? "?" : (const gchar*)(cValue))

struct _AppletConfig {
	gchar *cLocationCode;
	gboolean bISUnits;
	gboolean bCurrentConditions;
	gboolean bDisplayNights;
	gboolean bDisplayTemperature;
	gint iNbDays;
	gchar *cRenderer;
	gint cDialogDuration;
	guint iCheckInterval;
	gchar *cThemePath;
	gboolean bDesklet3D;
	gboolean bSetName;
	} ;

typedef struct {
	xmlChar *cTemp;
	xmlChar *cDistance;
	xmlChar *cSpeed;
	xmlChar *cPressure;
	} Unit;

typedef struct {
	xmlChar *cSunRise;
	xmlChar *cSunSet;
	xmlChar *cDataAcquisitionDate;
	xmlChar *cObservatory;
	xmlChar *cTemp;
	xmlChar *cFeltTemp;
	xmlChar *cWeatherDescription;
	xmlChar *cIconNumber;
	xmlChar *cWindSpeed;
	xmlChar *cWindDirection;
	xmlChar *cPressure;
	xmlChar *cHumidity;
	xmlChar *cMoonIconNumber;
	} CurrentContitions;

typedef struct {
	xmlChar *cIconNumber;
	xmlChar *cWeatherDescription;
	xmlChar *cWindSpeed;
	xmlChar *cWindDirection;
	xmlChar *cHumidity;
	xmlChar *cPrecipitationProba;
	} DayPart;

typedef struct {
	xmlChar *cName;
	xmlChar *cDate;
	xmlChar *cTempMax;
	xmlChar *cTempMin;
	xmlChar *cSunRise;
	xmlChar *cSunSet;
	DayPart part[2];
	} Day;

typedef struct {
	xmlChar *cLocation;
	/**xmlChar *cLon;
	xmlChar *cLat;*/
	Unit units;
	CurrentContitions currentConditions;
	Day days[WEATHER_NB_DAYS_MAX];
	} CDWeatherData;

typedef struct {
	gchar *cLocationCode;
	gboolean bISUnits;
	gboolean bCurrentConditions;
	gint iNbDays;
	CDWeatherData wdata;
	gboolean bErrorInThread;
	CairoDockModuleInstance *pApplet;
	} CDSharedMemory;

struct _AppletData {
	CDWeatherData wdata;
	CairoDockTask *pTask;
	gboolean bErrorRetrievingData;
	GList *pLocationsList;
	gboolean bSetName;
	gint iClickedDay;
	CairoDockTask *pGetLocationTask;
	} ;


#endif
