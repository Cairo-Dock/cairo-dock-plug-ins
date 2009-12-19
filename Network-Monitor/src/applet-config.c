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
#include "applet-netspeed.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	
	myConfig.iWifiCheckInterval = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "wifi delay", 10);
	myConfig.iNetspeedCheckInterval = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "netspeed delay", 10);
	myConfig.fSmoothFactor = CD_CONFIG_GET_DOUBLE ("Configuration", "smooth");
	myConfig.cInterface = CD_CONFIG_GET_STRING ("Configuration", "interface");
	myConfig.iStringLen = (myConfig.cInterface ? strlen (myConfig.cInterface) : 0);
	myConfig.cWifiConfigCommand = CD_CONFIG_GET_STRING ("Configuration", "wifi command");
	myConfig.cSysMonitorCommand = CD_CONFIG_GET_STRING ("Configuration", "netspeed command");
	
	myConfig.iRenderType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "renderer", 0);
	if (myConfig.iRenderType == CD_EFFECT_ICON)
	{
		GString *sKeyName = g_string_new ("");
		int i;
		for (i = 0; i < CONNECTION_NB_QUALITY; i ++)
		{
			g_string_printf (sKeyName, "icon_%d", i);
			myConfig.cUserImage[i] = CD_CONFIG_GET_STRING ("Configuration", sKeyName->str);
		}
		g_string_free (sKeyName, TRUE);
		myConfig.iEffect = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "effect", 0);
	}
	else if (myConfig.iRenderType == CD_EFFECT_GAUGE)
	{
		myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
	}
	else
	{
		myConfig.iGraphType = CD_CONFIG_GET_INTEGER ("Configuration", "graphic type");
		CD_CONFIG_GET_COLOR_RVB ("Configuration", "low color", myConfig.fLowColor);
		CD_CONFIG_GET_COLOR_RVB ("Configuration", "high color", myConfig.fHigholor);
		CD_CONFIG_GET_COLOR ("Configuration", "bg color", myConfig.fBgColor);
	}
	
	myConfig.bModeWifi = (CD_CONFIG_GET_INTEGER ("Configuration", "mode") == 0);
	myConfig.quickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "signal_type", CONNECTION_INFO_SIGNAL_STRENGTH_LEVEL);
	
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cGThemePath);
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cWifiConfigCommand);
	g_free (myConfig.cInterface);
	
	int i;
	for (i = 0; i < CONNECTION_NB_QUALITY; i ++)
		g_free (myConfig.cUserImage[i]);
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_task (myData.pTask);
	
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
	g_print ("%s (%s)\n", __func__, myIcon->cName);
	//\____________ On recupere la combo.
	GtkWidget *pCombo = cairo_dock_get_widget_from_name ("Configuration", "interface");
	g_return_if_fail (pCombo != NULL);
	
	//\____________ On construit la liste interfaces disponibles.
	GList *pList = cd_netmonitor_get_available_interfaces ();
	
	//\____________ On remplit la combo.
	cairo_dock_fill_combo_with_list (pCombo, pList, myConfig.cInterface);
	
	g_list_foreach (pList, g_free, NULL);
	g_list_free (pList);
}
