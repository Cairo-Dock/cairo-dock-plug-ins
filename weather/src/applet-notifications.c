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
	if (myDock != NULL && myIcon->pSubDock != NULL && pClickedDock == myIcon->pSubDock)
	{
		cd_debug (" clic sur %s", pClickedIcon->acName);
		cd_weather_show_forecast_dialog (pClickedIcon);
	}
	else if (myDesklet != NULL && pClickedDock == myDesklet)
	{
		int iMouseX = - (int) myDesklet->diff_x;
		int iMouseY = - (int) myDesklet->diff_y;
		cd_debug (" clic en (%d;%d)", iMouseX, iMouseY);
		
		if (myIcon->fDrawX < iMouseX && myIcon->fDrawX + myIcon->fWidth > iMouseX && myIcon->fDrawY < iMouseY && myIcon->fDrawY + myIcon->fHeight > iMouseY)
		{
			cd_weather_show_current_conditions_dialog ();
		}
		else
		{
			GList* ic;
			Icon *icon;
			for (ic = myData.pDeskletIconList; ic != NULL; ic = ic->next)
			{
				icon = ic->data;
				if (icon->fDrawX < iMouseX && icon->fDrawX + icon->fWidth * icon->fScale > iMouseX && icon->fDrawY < iMouseY && icon->fDrawY + icon->fHeight * icon->fScale > iMouseY)
				{
					cd_weather_show_forecast_dialog (icon);
					break ;
				}
			}
		}
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
	if (myDock != NULL)
		g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_remove_dialog_if_any, NULL);
	else
		//g_list_foreach (myData.pDeskletIconList, (GFunc) cairo_dock_remove_dialog_if_any, NULL);
		cairo_dock_remove_dialog_if_any (myIcon);
	
	int iNumDay = ((int) pIcon->fOrder) / 2, iPart = ((int) pIcon->fOrder) - 2 * iNumDay;
	g_return_val_if_fail (iNumDay < myConfig.iNbDays && iPart < 2, NULL);
	
	Day *day = &myData.days[iNumDay];
	DayPart *part = &day->part[iPart];
	cairo_dock_show_temporary_dialog_with_icon ("%s (%s) : %s\n %s : %s%s -> %s%s\n %s : %s%s (%s)\n %s : %s\n %s : %s  %s %s",
		(myDock != NULL ? pIcon : myIcon), (myDock != NULL ? myIcon->pSubDock : myDesklet), myConfig.cDialogDuration, pIcon->acFileName,
		day->cName, day->cDate, part->cWeatherDescription,
		_D("Temperature"), _display (day->cTempMin), myData.units.cTemp, _display (day->cTempMax), myData.units.cTemp,
		_D("Wind"), _display (part->cWindSpeed), myData.units.cSpeed, _display (part->cWindDirection),
		_D("Humidity"), _display (part->cHumidity),  // unite ?...
		_D("SunRise"), _display (day->cSunRise), _("SunSet"), _display (day->cSunSet));
}

CairoDockDialog *cd_weather_show_current_conditions_dialog (void)
{
	cairo_dock_remove_dialog_if_any (myIcon);
	
	CurrentContitions *cc = &myData.currentConditions;
	cairo_dock_show_temporary_dialog_with_icon ("%s (%s, %s)\n %s : %s%s (%s : %s%s)\n %s : %s%s (%s)\n %s : %s - %s : %s%s\n %s : %s  %s %s",
		myIcon, myDesklet, myConfig.cDialogDuration, myIcon->acFileName,
		cc->cWeatherDescription, cc->cDataAcquisitionDate, cc->cObservatory,
		_D("Temperature"), _display (cc->cTemp), myData.units.cTemp, _D("feeled"), _display (cc->cFeeledTemp), myData.units.cTemp,
		_D("Wind"), _display (cc->cWindSpeed), myData.units.cSpeed, _display (cc->cWindDirection),
		_D("Humidity"), _display (cc->cHumidity), _D("Pressure"), _display (cc->cPressure), myData.units.cPressure,  // unite ?...
		_D("SunRise"), _display (cc->cSunRise), _D("SunSet"), _display (cc->cSunSet));
}

