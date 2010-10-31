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
	if (cairo_dock_task_is_running (myData.pTask))
	{
		cairo_dock_show_temporary_dialog_with_icon (D_("Data are being retrieved, please wait a moment."), 
			myIcon,
			myContainer,
			3000,
			"same icon");
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
	}
	if (pClickedIcon == myIcon)  // en mode dock, on peut recevoir le clic sur l'icone principale si elle n'a pas de sous-dock, autrement dit si la connexion s'est mal passe, ce qui nous permet d'afficher le message d'erreur.
	{
		cd_weather_show_current_conditions_dialog (myApplet);
	}
	else if (pClickedIcon != NULL)  // clic sur une des sous-icones.
	{
		cd_weather_show_forecast_dialog (myApplet, pClickedIcon);
	}
CD_APPLET_ON_CLICK_END

static int _get_num_day_from_icon (CairoDockModuleInstance *myApplet, Icon *pIcon)
{
	/// TODO: determiner le jour exact...
	return (pIcon == myIcon ? 0 : pIcon->fOrder/2);  // la 1ere icone est le plus souvent celle d'aujourd'hui, toutefois cela peut ne pas etre vrai, surtout la nuit autour du changement de jour.
}
static inline void _go_to_site (CairoDockModuleInstance *myApplet, int iNumDay)
{
	gchar *cURI;
	if (iNumDay == 0)
		cURI = g_strdup_printf ("http://www.weather.com/weather/today/%s", myConfig.cLocationCode);
	else
		cURI = g_strdup_printf ("http://www.weather.com/weather/wxdetail/%s?dayNum=%d", myConfig.cLocationCode, iNumDay);
	cairo_dock_fm_launch_uri (cURI);
	g_free (cURI);
}

static inline void _reload (CairoDockModuleInstance *myApplet)
{
	if (cairo_dock_task_is_running (myData.pTask))
	{
		cairo_dock_show_temporary_dialog_with_icon (D_("Data are being retrieved, please wait a moment."), 
			myIcon,
			myContainer,
			3000,
			"same icon");
	}
	else
	{
		cairo_dock_stop_task (myData.pTask);
		
		cairo_dock_launch_task (myData.pTask);
	}
}


static void _cd_weather_reload (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	_reload (myApplet);
	CD_APPLET_LEAVE ();
}
static void _cd_weather_show_site (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	_go_to_site (myApplet, myData.iClickedDay);
	CD_APPLET_LEAVE ();
}
static void _cd_weather_show_cc (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_weather_show_current_conditions_dialog (myApplet);
	CD_APPLET_LEAVE ();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	// Main Menu
	if (pClickedIcon == myIcon)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Show current conditions (middle-click)"), GTK_STOCK_DIALOG_INFO, _cd_weather_show_cc, CD_APPLET_MY_MENU);
	}
	if (pClickedIcon != NULL)
	{
		myData.iClickedDay = _get_num_day_from_icon (myApplet, pClickedIcon);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Open weather.com (double-click)"), GTK_STOCK_JUMP_TO, _cd_weather_show_site, CD_APPLET_MY_MENU);
	}
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Reload now"), GTK_STOCK_REFRESH, _cd_weather_reload, CD_APPLET_MY_MENU);
	// Sub-Menu
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
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
		cairo_dock_remove_dialog_if_any (pClickedIcon);
		int iNumDay = _get_num_day_from_icon (myApplet, pClickedIcon);
		_go_to_site (myApplet, iNumDay);
	}
CD_APPLET_ON_DOUBLE_CLICK_END


CairoDialog *cd_weather_show_forecast_dialog (CairoDockModuleInstance *myApplet, Icon *pIcon)
{
	if (myDock != NULL)
		g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_remove_dialog_if_any_full, GINT_TO_POINTER (TRUE));
	else
		cairo_dock_remove_dialog_if_any (myIcon);
	
	if (myData.bErrorRetrievingData)
	{
		cairo_dock_show_temporary_dialog_with_icon (D_("No data available\n is your connection alive?"), 
			(myDock ? pIcon : myIcon),
			(myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer),
			myConfig.cDialogDuration,
			"same icon");
		return NULL;
	}
	
	int iNumDay = ((int) pIcon->fOrder) / 2, iPart = ((int) pIcon->fOrder) - 2 * iNumDay;
	g_return_val_if_fail (iNumDay < myConfig.iNbDays && iPart < 2, NULL);
	
	Day *day = &myData.days[iNumDay];
	DayPart *part = &day->part[iPart];
	cairo_dock_show_temporary_dialog_with_icon_printf ("%s (%s) : %s\n %s : %s%s -> %s%s\n %s : %s%%\n %s : %s%s (%s)\n %s : %s%%\n %s : %s  %s %s",
		(myDock ? pIcon : myIcon),
		(myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer),
		myConfig.cDialogDuration,
		"same icon",
		day->cName, day->cDate, part->cWeatherDescription,
		D_("Temperature"), _display (day->cTempMin), myData.units.cTemp, _display (day->cTempMax), myData.units.cTemp,
		D_("Precipitation probability"), _display (part->cPrecipitationProba),
		D_("Wind"), _display (part->cWindSpeed), myData.units.cSpeed, _display (part->cWindDirection),
		D_("Humidity"), _display (part->cHumidity),
		D_("Sunrise"), _display (day->cSunRise), _("Sunset"), _display (day->cSunSet));
}

CairoDialog *cd_weather_show_current_conditions_dialog (CairoDockModuleInstance *myApplet)
{
	cairo_dock_remove_dialog_if_any (myIcon);
	if (cairo_dock_task_is_running (myData.pTask))
	{
		cairo_dock_show_temporary_dialog_with_icon (D_("Data are being fetched, please re-try in a few seconds."),
			myIcon,
			myContainer,
			3000,
			"same icon");
		
		return NULL;
	}
	if (myData.bErrorRetrievingData)
	{
		cairo_dock_show_temporary_dialog_with_icon (D_("No data available\nRetrying now..."),
			myIcon,
			myContainer,
			3000,
			myIcon->cFileName);
		_reload (myApplet);
		
		return NULL;
	}
	
	CurrentContitions *cc = &myData.currentConditions;
	cairo_dock_show_temporary_dialog_with_icon_printf ("%s (%s, %s)\n %s : %s%s (%s : %s%s)\n %s : %s%s (%s)\n %s : %s - %s : %s%s\n %s : %s  %s %s",
		myIcon, myContainer, myConfig.cDialogDuration, myIcon->cFileName,
		cc->cWeatherDescription, cc->cDataAcquisitionDate, cc->cObservatory,
		D_("Temperature"), _display (cc->cTemp), myData.units.cTemp, D_("Feels like"), _display (cc->cFeltTemp), myData.units.cTemp,
		D_("Wind"), _display (cc->cWindSpeed), myData.units.cSpeed, _display (cc->cWindDirection),
		D_("Humidity"), _display (cc->cHumidity), D_("Pressure"), _display (cc->cPressure), myData.units.cPressure,  // unite ?...
		D_("Sunrise"), _display (cc->cSunRise), D_("Sunset"), _display (cc->cSunSet));
}
