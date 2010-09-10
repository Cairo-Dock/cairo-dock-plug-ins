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
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER ("Configuration", "delay");
	myConfig.fSmoothFactor = CD_CONFIG_GET_DOUBLE ("Configuration", "smooth");
	
	myConfig.bShowCpu = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "show cpu", TRUE);
	myConfig.bShowRam = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "show ram", TRUE);
	myConfig.bShowSwap = CD_CONFIG_GET_BOOLEAN ("Configuration", "show swap");
	myConfig.bShowNvidia = CD_CONFIG_GET_BOOLEAN ("Configuration", "show nvidia");
	myConfig.bShowFreeMemory = CD_CONFIG_GET_BOOLEAN ("Configuration", "show free");
	
	myConfig.iInfoDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "info display");
	myConfig.iDisplayType = CD_CONFIG_GET_INTEGER ("Configuration", "renderer");
	
	myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
	
	myConfig.iGraphType = CD_CONFIG_GET_INTEGER ("Configuration", "graphic type");
	myConfig.bMixGraph = CD_CONFIG_GET_BOOLEAN ("Configuration", "mix graph");
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "low color", myConfig.fLowColor);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "high color", myConfig.fHigholor);
	CD_CONFIG_GET_COLOR ("Configuration", "bg color", myConfig.fBgColor);
	
	myConfig.iLowerLimit = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "llt", 50);
	myConfig.iUpperLimit = MAX (myConfig.iLowerLimit+1, CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "ult", 110));
	myConfig.iAlertLimit = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "alt", 100);
	myConfig.bAlert = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "alert", TRUE);
	myConfig.bAlertSound = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "asound", TRUE);
	myConfig.cSoundPath = CD_CONFIG_GET_STRING ("Configuration", "sound path");
	
	myConfig.iNbDisplayedProcesses = CD_CONFIG_GET_INTEGER ("Configuration", "top");
	myConfig.iProcessCheckInterval = CD_CONFIG_GET_INTEGER ("Configuration", "top delay");
	
	myConfig.pTopTextDescription = cairo_dock_duplicate_label_description (&myDialogs.dialogTextDescription);
	g_free (myConfig.pTopTextDescription->cFont);
	myConfig.pTopTextDescription->cFont = g_strdup ("Mono");  // on prend une police a chasse fixe.
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "top color start", myConfig.pTopTextDescription->fColorStart);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "top color stop", myConfig.pTopTextDescription->fColorStop);
	myConfig.pTopTextDescription->bVerticalPattern = TRUE;
	myConfig.bTopInPercent = CD_CONFIG_GET_BOOLEAN ("Configuration", "top in percent");
	
	myConfig.cSystemMonitorCommand = CD_CONFIG_GET_STRING ("Configuration", "sys monitor");
	myConfig.bStealTaskBarIcon = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "inhibate appli", TRUE);
	if (myConfig.bStealTaskBarIcon)
	{
		myConfig.cSystemMonitorClass = CD_CONFIG_GET_STRING ("Configuration", "sys monitor class");
		if (myConfig.cSystemMonitorClass == NULL)
		{
			if (myConfig.cSystemMonitorCommand != NULL)
			{
				myConfig.cSystemMonitorClass = g_strdup (myConfig.cSystemMonitorCommand);
				gchar *str = strchr (myConfig.cSystemMonitorClass, ' ');  // on coupe au 1er espace.
				if (str)
					*str = '\0';
			}
			else if (g_iDesktopEnv == CAIRO_DOCK_GNOME)
				myConfig.cSystemMonitorClass = g_strdup ("gnome-system-monitor");
			else if (g_iDesktopEnv == CAIRO_DOCK_XFCE)
				myConfig.cSystemMonitorClass = g_strdup ("xfce-system-monitor");
			else if (g_iDesktopEnv == CAIRO_DOCK_KDE)
				myConfig.cSystemMonitorClass = g_strdup ("kde-system-monitor");
		}
	}
	myConfig.fUserHZ = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "HZ", 100);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cGThemePath);
	g_free (myConfig.defaultTitle);
	cairo_dock_free_label_description (myConfig.pTopTextDescription);
	g_free (myConfig.cSystemMonitorCommand);
	g_free (myConfig.cSystemMonitorClass);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_task (myData.pPeriodicTask);
	g_timer_destroy (myData.pClock);
	
	CD_APPLET_REMOVE_MY_DATA_RENDERER;
	
	cairo_dock_free_task (myData.pTopTask);
	if (myData.pTopClock != NULL)
		g_timer_destroy (myData.pTopClock);
	g_free (myData.pTopList);
	if (myData.pProcessTable != NULL)
		g_hash_table_destroy (myData.pProcessTable);
	cairo_surface_destroy (myData.pTopSurface);
	
	g_free (myData.cModelName);
	g_free (myData.cGPUName);
	g_free (myData.cDriverVersion);
	
CD_APPLET_RESET_DATA_END

