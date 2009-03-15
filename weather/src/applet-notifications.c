/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"
#include "applet-notifications.h"


CD_APPLET_ABOUT (D_("This is the weather applet\n made by Fabrice Rey for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	if (myDock != NULL && myIcon->pSubDock != NULL && pClickedContainer == CAIRO_CONTAINER (myIcon->pSubDock))  // on a clique sur une icone du sous-dock.
	{
		cd_debug (" clic sur %s", pClickedIcon->acName);
		cd_weather_show_forecast_dialog (myApplet, pClickedIcon);
	}
	else if (myDesklet != NULL && pClickedContainer == myContainer && pClickedIcon != NULL)  // on a clique sur une icone du desklet.
	{
		if (pClickedIcon == myIcon)
			cd_weather_show_current_conditions_dialog (myApplet);
		else
			cd_weather_show_forecast_dialog (myApplet, pClickedIcon);
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_CLICK_END


static void _cd_weather_reload (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cairo_dock_stop_measure_timer (myData.pMeasureTimer);
	
	cairo_dock_launch_measure (myData.pMeasureTimer);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_IN_MENU (D_("Reload now"), _cd_weather_reload, pSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (pClickedIcon == myIcon)
	{
		cd_weather_show_current_conditions_dialog (myApplet);
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_MIDDLE_CLICK_END


CairoDialog *cd_weather_show_forecast_dialog (CairoDockModuleInstance *myApplet, Icon *pIcon)
{
	if (myDock != NULL)
		g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_remove_dialog_if_any, NULL);
	else
		//g_list_foreach (myData.pDeskletIconList, (GFunc) cairo_dock_remove_dialog_if_any, NULL);
		cairo_dock_remove_dialog_if_any (myIcon);
	
	if (myData.bErrorRetrievingData)
	{
		cairo_dock_show_temporary_dialog_with_icon (_("No data were available\n is connection alive ?"), 
			(myDock ? pIcon : myIcon),
			(myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer),
			myConfig.cDialogDuration,
			pIcon->acFileName);
		return NULL;
	}
	
	int iNumDay = ((int) pIcon->fOrder) / 2, iPart = ((int) pIcon->fOrder) - 2 * iNumDay;
	g_return_val_if_fail (iNumDay < myConfig.iNbDays && iPart < 2, NULL);
	
	Day *day = &myData.days[iNumDay];
	DayPart *part = &day->part[iPart];
	cairo_dock_show_temporary_dialog_with_icon ("%s (%s) : %s\n %s : %s%s -> %s%s\n %s : %s%%\n %s : %s%s (%s)\n %s : %s\n %s : %s  %s %s",
		(myDock ? pIcon : myIcon), (myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer), myConfig.cDialogDuration, pIcon->acFileName,
		day->cName, day->cDate, part->cWeatherDescription,
		D_("Temperature"), _display (day->cTempMin), myData.units.cTemp, _display (day->cTempMax), myData.units.cTemp,
		D_("Precipitation Probability"), _display (part->cPrecipitationProba),
		D_("Wind"), _display (part->cWindSpeed), myData.units.cSpeed, _display (part->cWindDirection),
		D_("Humidity"), _display (part->cHumidity),  // unite ?...
		D_("SunRise"), _display (day->cSunRise), _("SunSet"), _display (day->cSunSet));
}

CairoDialog *cd_weather_show_current_conditions_dialog (CairoDockModuleInstance *myApplet)
{
	cairo_dock_remove_dialog_if_any (myIcon);
	
	if (myData.bErrorRetrievingData)
	{
		cairo_dock_show_temporary_dialog_with_icon (_("No data were available\n is connection alive ?"), 
			myIcon,
			myContainer,
			myConfig.cDialogDuration,
			myIcon->acFileName);
		return NULL;
	}
	
	CurrentContitions *cc = &myData.currentConditions;
	cairo_dock_show_temporary_dialog_with_icon ("%s (%s, %s)\n %s : %s%s (%s : %s%s)\n %s : %s%s (%s)\n %s : %s - %s : %s%s\n %s : %s  %s %s",
		myIcon, myContainer, myConfig.cDialogDuration, myIcon->acFileName,
		cc->cWeatherDescription, cc->cDataAcquisitionDate, cc->cObservatory,
		D_("Temperature"), _display (cc->cTemp), myData.units.cTemp, D_("feeled"), _display (cc->cFeeledTemp), myData.units.cTemp,
		D_("Wind"), _display (cc->cWindSpeed), myData.units.cSpeed, _display (cc->cWindDirection),
		D_("Humidity"), _display (cc->cHumidity), D_("Pressure"), _display (cc->cPressure), myData.units.cPressure,  // unite ?...
		D_("SunRise"), _display (cc->cSunRise), D_("SunSet"), _display (cc->cSunSet));
}
