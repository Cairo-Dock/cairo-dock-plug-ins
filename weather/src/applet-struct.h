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

// yahoo weather has no code location API
// it's possible to get a code (woeid) in the 'link' of the response, but that means that you already got a valid location
// so it's pretty useless
#undef CD_WEATHER_HAS_CODE_LOCATION

#define _display(cValue) ((cValue) == NULL || *((gchar*)cValue) == 'N' ? "?" : (const gchar*)(cValue))

struct _AppletConfig {
	gchar *cLocationCode;
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
	xmlChar *cTemp;
	xmlChar *cSpeed;
	xmlChar *cPressure;
	} Unit;

typedef struct {
	xmlChar *cIconNumber;
	xmlChar *cDate;
	xmlChar *cName;
	xmlChar *cTempMax;
	xmlChar *cTempMin;
	xmlChar *cWeatherDescription;
	} Day;

typedef struct {
	xmlChar *cWindSpeed;
	xmlChar *cWindDirection;
	xmlChar *cPressure;
	xmlChar *cHumidity;
	xmlChar *cSunRise;
	xmlChar *cSunSet;
	int ttl;  // in mn;
	xmlChar *cDataAcquisitionDate;
	Day now;
	} CurrentContitions;

typedef struct {
	Unit units;
	CurrentContitions currentConditions;
	Day days[WEATHER_NB_DAYS_MAX];
	xmlChar *cCountry;
	xmlChar *cCity;
	xmlChar *cLink;
	} CDWeatherData;

typedef struct {
	gchar *cLocationCode;
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
