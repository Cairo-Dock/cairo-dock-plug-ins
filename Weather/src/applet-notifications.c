/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_ABOUT (_D("This is the weather applet\n made by Fabrice Rey for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	if (myIcon->pSubDock != NULL && pClickedDock == myIcon->pSubDock)
	{
		g_print (" clic sur %s\n", pClickedIcon->acName);
		cd_weather_show_forecast_dialog (pClickedIcon);
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("weather", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (pClickedIcon == myIcon)
	{
		cd_weather_show_current_conditions_dialog ();
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_MIDDLE_CLICK_END


CairoDockDialog *cd_weather_show_forecast_dialog (Icon *pIcon)
{
	g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_remove_dialog_if_any, NULL);
	
	int iNumDay = ((int) pIcon->fOrder) / 2, iPart = ((int) pIcon->fOrder) - 2 * iNumDay;
	g_return_val_if_fail (iNumDay < myConfig.iNbDays && iPart < 2, NULL);
	
	Day *day = &myData.days[iNumDay];
	DayPart *part = &day->part[iPart];
	cairo_dock_show_temporary_dialog_with_icon ("%s (%s) : %s\n %s : %s%s -> %s%s\n %s : %s%s (%s)\n %s : %s\n %s : %s  %s %s",
		pIcon, myIcon->pSubDock, myConfig.cDialogDuration, pIcon->acFileName,
		day->cName, day->cDate, part->cWeatherDescription,
		_("Temperature"), day->cTempMin, myData.units.cTemp, day->cTempMax, myData.units.cTemp,
		_("Wind"), part->cWindSpeed, myData.units.cSpeed, part->cWindDirection,
		_("Humidity"), part->cHumidity,  // unite ?...
		_("SunRise"), day->cSunRise, _("SunSet"), day->cSunSet);
}

CairoDockDialog *cd_weather_show_current_conditions_dialog (void)
{
	cairo_dock_remove_dialog_if_any (myIcon);
	
	CurrentContitions *cc = &myData.currentConditions;
	cairo_dock_show_temporary_dialog_with_icon ("%s (%s, %s)\n %s : %s%s (%s : %s%s)\n %s : %s%s (%s)\n %s : %s - %s : %s%s\n %s : %s  %s %s",
		myIcon, myDock, myConfig.cDialogDuration, myIcon->acFileName,
		cc->cWeatherDescription, cc->cDataAcquisitionDate, cc->cObservatory,
		_("Temperature"), cc->cTemp, myData.units.cTemp, _("feeled"), cc->cFeeledTemp, myData.units.cTemp,
		_("Wind"), cc->cWindSpeed, myData.units.cSpeed, cc->cWindDirection,
		_("Humidity"), cc->cHumidity, _("Pressure"), cc->cPressure, myData.units.cPressure,
		_("SunRise"), cc->cSunRise, _("SunSet"), cc->cSunSet);
}

