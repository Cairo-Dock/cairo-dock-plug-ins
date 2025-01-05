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

#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"
#include "applet-notifications.h"


CD_APPLET_ON_CLICK_BEGIN
	if (pClickedIcon == myIcon)  // en mode dock, on peut recevoir le clic sur l'icone principale si elle n'a pas de sous-dock, autrement dit si la connexion s'est mal passe, ce qui nous permet d'afficher le message d'erreur.
	{
		cd_weather_show_current_conditions_dialog (myApplet);
	}
	else if (pClickedIcon != NULL)  // clic sur une des sous-icones.
	{
		cd_weather_show_forecast_dialog (myApplet, pClickedIcon);
	}
CD_APPLET_ON_CLICK_END

static int _get_num_day_from_icon (GldiModuleInstance *myApplet, Icon *pIcon)
{
	/// TODO: determiner le jour exact...
	return (pIcon == myIcon ? -1 : pIcon->fOrder/2);  // la 1ere icone est le plus souvent celle d'aujourd'hui, toutefois cela peut ne pas etre vrai, surtout la nuit autour du changement de jour.
}
static inline void _go_to_site (GldiModuleInstance *myApplet, int iNumDay)
{
	cairo_dock_fm_launch_uri ("https://open-meteo.com/en/docs");
}

static inline void _reload (GldiModuleInstance *myApplet)
{
	if (gldi_task_is_running (myData.pTask))
	{
		gldi_dialog_show_temporary_with_icon (D_("Data are being retrieved, please wait a moment."), 
			myIcon,
			myContainer,
			3000,
			"same icon");
	}
	else
	{
		gldi_task_stop (myData.pTask);  // not blocking since the task is not running.
		
		myData.bBusy = TRUE;
		CD_APPLET_ANIMATE_MY_ICON ("busy", 999);
		cairo_dock_mark_icon_as_clicked (myIcon);  // prevent hovering the icon to overwrite the animation with another one.
		gldi_task_launch (myData.pTask);
	}
}


static void _cd_weather_reload (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	_reload (myApplet);
	CD_APPLET_LEAVE ();
}
static void _cd_weather_show_site (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	_go_to_site (myApplet, myData.iClickedDay);
	CD_APPLET_LEAVE ();
}
static void _cd_weather_show_cc (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_weather_show_current_conditions_dialog (myApplet);
	CD_APPLET_LEAVE ();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	if (pClickedIcon == myIcon)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Show current conditions (middle-click)"), GLDI_ICON_NAME_DIALOG_INFO, _cd_weather_show_cc, CD_APPLET_MY_MENU);
	}
	if (pClickedIcon != NULL)
	{
		myData.iClickedDay = _get_num_day_from_icon (myApplet, pClickedIcon);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Open open-meteo.com (double-click)"), GLDI_ICON_NAME_JUMP_TO, _cd_weather_show_site, CD_APPLET_MY_MENU);
	}
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Reload now"), GLDI_ICON_NAME_REFRESH, _cd_weather_reload, CD_APPLET_MY_MENU);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (pClickedIcon == myIcon)
	{
		cd_weather_show_current_conditions_dialog (myApplet);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_DOUBLE_CLICK_BEGIN
	if (pClickedIcon != NULL)
	{
		gldi_dialogs_remove_on_icon (pClickedIcon);
		int iNumDay = _get_num_day_from_icon (myApplet, pClickedIcon);
		_go_to_site (myApplet, iNumDay);
	}
CD_APPLET_ON_DOUBLE_CLICK_END


void cd_weather_show_forecast_dialog (GldiModuleInstance *myApplet, Icon *pIcon)
{
	// remove any other forecast dialog.
	if (myDock != NULL)
		g_list_foreach (myIcon->pSubDock->icons, (GFunc) gldi_dialogs_remove_on_icon, NULL);
	else
		gldi_dialogs_remove_on_icon (myIcon);
	
	// present the day's forecast.
	int iNumDay = ((int) pIcon->fOrder) / 2;
	g_return_if_fail (iNumDay < myConfig.iNbDays);
	
	Day *day = &myData.wdata.days[iNumDay];
	gldi_dialog_show_temporary_with_icon_printf ("%s (%s) : %s\n %s : %s%s -> %s%s\n %s : %s",
		(myDock ? pIcon : myIcon),
		(myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer),
		myConfig.cDialogDuration,
		"same icon",
		day->cName, day->cDate, day->cWeatherDescription,
		D_("Temperature"), _display (day->cTempMin), myData.wdata.units.cTemp, _display (day->cTempMax), myData.wdata.units.cTemp,
		D_("Rain or snow"), _display (day->cPrecipProb));
}

void cd_weather_show_current_conditions_dialog (GldiModuleInstance *myApplet)
{
	gldi_dialogs_remove_on_icon (myIcon);
	
	// if an error occured, the current conditions are no more valid.
	if (gldi_task_is_running (myData.pTask))  // current conditions are outdated.
	{
		gldi_dialog_show_temporary_with_icon (D_("Data are being fetched, please re-try in a few seconds."),
			myIcon,
			myContainer,
			3000,
			"same icon");
		return;
	}
	
	if (myData.bErrorRetrievingData)
	{
		gldi_dialog_show_temporary_with_icon (D_("No data available\nRetrying now..."),
			myIcon,
			myContainer,
			3000,
			myIcon->cFileName);
		_reload (myApplet);
		return ;
	}
	
	// show a dialog with the current conditions.
	CurrentContitions *cc = &myData.wdata.currentConditions;
	gldi_dialog_show_temporary_with_icon_printf ("%s:\n %s : %s%s\n %s : %s%s\n %s : %s%s \n %s : %s%s\n %s : %s  %s : %s",
		myIcon, myContainer, myConfig.cDialogDuration, myIcon->cFileName,
		cc->now.cDate,
		D_("Temperature"), _display (cc->now.cTempMax), myData.wdata.units.cTemp,
		D_("Wind"), _display (cc->cWindSpeed), myData.wdata.units.cSpeed,
		D_("Humidity"), _display (cc->cHumidity), myData.wdata.units.cHumidity,
		D_("Pressure"), _display (cc->cPressure), myData.wdata.units.cPressure,
		D_("Sunrise"), _display (cc->cSunRise), D_("Sunset"), _display (cc->cSunSet));
}
