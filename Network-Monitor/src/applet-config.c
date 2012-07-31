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
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-wifi.h"
#include "applet-netspeed.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.fSmoothFactor = CD_CONFIG_GET_DOUBLE ("Configuration", "smooth");
	myConfig.cInterface = CD_CONFIG_GET_STRING ("Configuration", "interface");
	if (myConfig.cInterface != NULL)
	{
		gchar *str = strrchr (myConfig.cInterface, ' ');  // on a rajoute (wifi) ou (ethernet).
		if (str)
			*str = '\0';
	}
	myConfig.iStringLen = (myConfig.cInterface ? strlen (myConfig.cInterface) : 0);
	myConfig.cWifiConfigCommand = CD_CONFIG_GET_STRING ("Configuration", "wifi command");
	myConfig.cSysMonitorCommand = CD_CONFIG_GET_STRING ("Configuration", "netspeed command");
	myConfig.cAnimation = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "conn animation", "rotate");
	
	myConfig.bModeWifi = (CD_CONFIG_GET_INTEGER ("Configuration", "mode") == 0);
	myConfig.iWifiCheckInterval = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Wifi", "wifi delay", 10);
	myConfig.iNetspeedCheckInterval = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Net Speed", "netspeed delay", 10);
	
	myConfig.wifiRenderer.iRenderType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Wifi", "renderer", 0);
	if (myConfig.wifiRenderer.iRenderType == CD_EFFECT_ICON)
	{
		GString *sKeyName = g_string_new ("");
		int i;
		for (i = 0; i < CONNECTION_NB_QUALITY; i ++)
		{
			g_string_printf (sKeyName, "icon_%d", i);
			myConfig.wifiRenderer.cUserImage[i] = CD_CONFIG_GET_STRING ("Wifi", sKeyName->str);
		}
		g_string_free (sKeyName, TRUE);
		myConfig.wifiRenderer.iEffect = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Wifi", "effect", 0);
	}
	else if (myConfig.wifiRenderer.iRenderType == CD_EFFECT_GAUGE)
	{
		myConfig.wifiRenderer.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Wifi", "theme");
	}
	else
	{
		myConfig.wifiRenderer.iGraphType = CD_CONFIG_GET_INTEGER ("Wifi", "graphic type");
		CD_CONFIG_GET_COLOR_RVB ("Wifi", "low color", myConfig.wifiRenderer.fLowColor);
		CD_CONFIG_GET_COLOR_RVB ("Wifi", "high color", myConfig.wifiRenderer.fHigholor);
		CD_CONFIG_GET_COLOR ("Wifi", "bg color", myConfig.wifiRenderer.fBgColor);
	}
	
	myConfig.netSpeedRenderer.iRenderType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Net Speed", "renderer", 0);
	if (myConfig.netSpeedRenderer.iRenderType == CD_EFFECT_GAUGE)
	{
		myConfig.netSpeedRenderer.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Net Speed", "theme");
	}
	else
	{
		myConfig.netSpeedRenderer.iGraphType = CD_CONFIG_GET_INTEGER ("Net Speed", "graphic type");
		CD_CONFIG_GET_COLOR_RVB ("Net Speed", "low color", myConfig.netSpeedRenderer.fLowColor);
		CD_CONFIG_GET_COLOR_RVB ("Net Speed", "high color", myConfig.netSpeedRenderer.fHigholor);
		CD_CONFIG_GET_COLOR ("Net Speed", "bg color", myConfig.netSpeedRenderer.fBgColor);
	}
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cWifiConfigCommand);
	g_free (myConfig.cInterface);
	
	g_free (myConfig.wifiRenderer.cGThemePath);
	g_free (myConfig.netSpeedRenderer.cGThemePath);
	int i;
	for (i = 0; i < CONNECTION_NB_QUALITY; i ++)
	{
		g_free (myConfig.wifiRenderer.cUserImage[i]);
	}
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cd_netmonitor_free_wifi_task (myApplet);
	cd_netmonitor_free_netspeed_task (myApplet);
	
	CD_APPLET_REMOVE_MY_DATA_RENDERER;
	
	int i;
	for (i = 0; i < CONNECTION_NB_QUALITY; i ++)
		cairo_surface_destroy (myData.pSurfaces[i]);
	
	g_free (myData.cESSID);
	g_free (myData.cInterface);
	g_free (myData.cAccessPoint);
	
CD_APPLET_RESET_DATA_END


void cd_netmonitor_load_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile* pKeyFile)
{
	cd_debug ("%s\n", __func__);
	//\____________ On recupere la combo.
	GtkWidget *pCombo = CD_APPLET_GET_CONFIG_PANEL_WIDGET ("Configuration", "interface");
	g_return_if_fail (pCombo != NULL);
	
	//\____________ On construit la liste interfaces disponibles.
	GList *pWirelessInterfaces = cd_netmonitor_get_wireless_interfaces ();
	GList *pList = cd_netmonitor_get_available_interfaces (pWirelessInterfaces);
	g_list_foreach (pWirelessInterfaces, (GFunc)g_free, NULL);
	g_list_free (pWirelessInterfaces);
	
	//\____________ On remplit la combo.
	cairo_dock_fill_combo_with_list (pCombo, pList, myConfig.cInterface);
	
	g_list_foreach (pList, (GFunc)g_free, NULL);
	g_list_free (pList);
}
