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
#include "applet-impulse.h"

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.cIconAnimation = CD_CONFIG_GET_STRING ("Configuration", "animation");
	myConfig.fMinValueToAnim = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "sensitivity", 0.25) / 3;
	myConfig.iNbAnimations = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nb animations", 1);
	myConfig.iLoopTime = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "refresh", 250);
	myConfig.pDock = cairo_dock_search_dock_from_name (CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "dock", "_MainDock_"));
	myConfig.bStopAnimations = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "stop animations", FALSE);
	myConfig.bLaunchAtStartup = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "startup", FALSE);

	myConfig.cIconImpulseON = CD_CONFIG_GET_STRING ("Configuration", "icon on");
	myConfig.cIconImpulseOFF = CD_CONFIG_GET_STRING ("Configuration", "icon off");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cIconAnimation);
	g_free (myConfig.cIconImpulseON);
	g_free (myConfig.cIconImpulseOFF);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	// cairo_dock_discard_task (myData.pTask);
	cd_impulse_stop_animations ();
CD_APPLET_RESET_DATA_END
