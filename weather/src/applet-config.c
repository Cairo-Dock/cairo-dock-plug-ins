/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)
Fabrice Rey (fabounet@users.berlios.de)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.cLocationCode = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "location code", "FRXX0076");
	myConfig.bISUnits = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "IS units", TRUE);
	myConfig.bCurrentConditions = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display cc", TRUE);
	myConfig.bDisplayNights = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display nights", FALSE);
	myConfig.iNbDays = MIN (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nb days", WEATHER_NB_DAYS_MAX), WEATHER_NB_DAYS_MAX);
	myConfig.bDisplayTemperature = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display temperature", FALSE);
	myConfig.cDialogDuration = 1000 * CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "dialog duration", 5);
	myConfig.iCheckInterval = 60 * MAX (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "check interval", 15), 1);
	
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "basic");
	
	myConfig.bDesklet3D = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "3D desket", FALSE);
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
CD_APPLET_GET_CONFIG_END


static void _reset_units (Unit *pUnits)
{
	g_free (pUnits->cTemp);
	g_free (pUnits->cDistance);
	g_free (pUnits->cSpeed);
	g_free (pUnits->cPressure);
}

static void _reset_current_conditions (CurrentContitions *pCurrentContitions)
{
	g_free (pCurrentContitions->cSunRise);
	g_free (pCurrentContitions->cSunSet);
	g_free (pCurrentContitions->cDataAcquisitionDate);
	g_free (pCurrentContitions->cObservatory);
	g_free (pCurrentContitions->cTemp);
	g_free (pCurrentContitions->cFeeledTemp);
	g_free (pCurrentContitions->cWeatherDescription);
	g_free (pCurrentContitions->cIconNumber);
	g_free (pCurrentContitions->cWindSpeed);
	g_free (pCurrentContitions->cWindDirection);
	g_free (pCurrentContitions->cPressure);
	g_free (pCurrentContitions->cHumidity);
	g_free (pCurrentContitions->cMoonIconNumber);
}

static void _reset_current_one_day (Day *pDay)
{
	g_free (pDay->cName);
	g_free (pDay->cDate);
	g_free (pDay->cTempMax);
	g_free (pDay->cTempMin);
	g_free (pDay->cSunRise);
	g_free (pDay->cSunSet);
	int j;
	for (j = 0; j < 2; j ++)
	{
		g_free (pDay->part[j].cIconNumber);
		g_free (pDay->part[j].cWeatherDescription);
		g_free (pDay->part[j].cWindSpeed);
		g_free (pDay->part[j].cWindDirection);
		g_free (pDay->part[j].cHumidity);
		g_free (pDay->part[j].cPrecipitationProba);
	}
}

CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cLocationCode);
	g_free (myConfig.cRenderer);
	g_free (myConfig.cThemePath);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	
	g_free (myData.cLon);
	g_free (myData.cLat);
	_reset_units (&myData.units);
	_reset_current_conditions (&myData.currentConditions);
	int i;
	for (i = 0; i < myConfig.iNbDays; i ++)
	{
		_reset_current_one_day (&myData.days[i]);
	}
	
	if (myIcon->pSubDock != NULL)
	{
		CD_APPLET_DESTROY_MY_SUBDOCK
	}
CD_APPLET_RESET_DATA_END
