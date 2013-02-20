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

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-indicator3.h"
#include "applet-launcher.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN (N_("Indicator-Generic"),
	3, 1, 99,
	CAIRO_DOCK_CATEGORY_APPLET_DESKTOP,
	N_("A plugin to hold all your 'indicators' applets.\n"
	"Simply enable it and it will load all your installed indicators.\n"
	"You can define a blacklist to not load some indicators."),
	"Matthieu Baerts (matttbe)")
	pVisitCard->bMultiInstance = FALSE; // don't load other instances, it will be done by the dock
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
CD_APPLET_DEFINE_END

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}

	cd_debug ("Init: %d [%s]", myApplet->pModule->pVisitCard->iContainerType, myConfig.cIndicatorName);

	// plugin <=> it's the first instance which should launch all indicators
	if (myApplet->pModule->pVisitCard->iContainerType == CAIRO_DOCK_MODULE_IS_PLUGIN)
	{
		// other instances will have an icon
		myApplet->pModule->pVisitCard->iContainerType = CAIRO_DOCK_MODULE_CAN_DOCK | CAIRO_DOCK_MODULE_CAN_DESKLET;
		myData.bIsLauncher = TRUE;

		// load all:
		cd_indicator_generic_load_all_indicators (myApplet);
	}
	else
	{
		// load => voir syncmenu, etc.
		cd_indicator_generic_load_one_indicator (myApplet);

		CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	}
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	if (! myData.bIsLauncher)
	{
		CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
		cd_indicator_generic_indicator_stop (myApplet);
	}
	
CD_APPLET_STOP_END

//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myData.bIsLauncher)
		{
			cd_indicator_generic_reload_all_indicators (myApplet); // if we have modified the blacklist
		}
		else
		{
			cd_indicator_generic_indicator_reload (myData.pIndicator, myData.pEntry, myApplet);
			if (! myData.pIndicator)
				CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
		}
	}
CD_APPLET_RELOAD_END
