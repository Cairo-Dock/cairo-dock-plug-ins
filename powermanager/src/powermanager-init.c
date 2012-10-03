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
#include <cairo-dock.h>

#include "powermanager-struct.h"
#include "powermanager-config.h"
#include "powermanager-common.h"
#include "powermanager-upower.h"
#include "powermanager-menu-functions.h"
#include "powermanager-draw.h"
#include "powermanager-init.h"


CD_APPLET_DEFINITION (N_("PowerManager"),
	2, 3, 0,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This applet displays the current state of your <b>laptop battery</b>: charge, time remaining, etc\n"
	"<b>Click</b> on the icon to have useful inforamtion,\n"
	"<b>Right-click</b> on the icon to hibernate or suspend the system."),
	"Necropotame (Adrien Pilleboue) and Fabounet")


CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
	}
	
	///_set_data_renderer (myApplet);
	
	cd_powermanager_start ();
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	cairo_dock_discard_task (myData.pTask);
	
	// stop UPower monitoring
	cd_upower_stop ();
	
	// stop ACPI check loop
	if (myData.checkLoop != 0)
	{
		g_source_remove (myData.checkLoop);
	}
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	CD_APPLET_REMOVE_OVERLAY_ON_MY_ICON (CAIRO_OVERLAY_MIDDLE);
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
			CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
		}
		
		// handle options that may have changed
		///_set_data_renderer (myApplet);
		
		cd_powermanager_change_loop_frequency (myConfig.iCheckInterval);
		
		if (myDock)
		{
			if (myConfig.bHideNotOnBattery && ! myData.bOnBattery)
			{ // hide the icon when not on battery and if needed
				cairo_dock_detach_icon_from_dock (myIcon, myDock);
				myData.bIsHidden = TRUE;
			}
			else if (myData.bIsHidden)
			{
				cairo_dock_insert_icon_in_dock (myIcon, myDock, CAIRO_DOCK_ANIMATE_ICON);
				///cairo_dock_redraw_container (CAIRO_CONTAINER (myDock));
				myData.bIsHidden = FALSE;
			}
		}
		
		// force the update of the icon
		myData.bPrevOnBattery = ! myData.bOnBattery;
		myData.iPrevPercentage = -1;
		myData.iPrevTime = -1;
		CD_APPLET_REMOVE_MY_DATA_RENDERER;
		update_icon();
	}
	else
	{
		if (myConfig.iDisplayType == CD_POWERMANAGER_GRAPH)
			CD_APPLET_SET_MY_DATA_RENDERER_HISTORY_TO_MAX;
		if (myData.bBatteryPresent && ! myData.bOnBattery)
			CD_APPLET_ADD_OVERLAY_ON_MY_ICON (myConfig.cEmblemIconName ? myConfig.cEmblemIconName : MY_APPLET_SHARE_DATA_DIR"/charge.svg", CAIRO_OVERLAY_MIDDLE);
	}
	
	//\_______________ On redessine notre icone.
	/**if (myData.cBatteryStateFilePath || myData.pBatteryDeviceList != NULL)
	{
		if (myConfig.iDisplayType == CD_POWERMANAGER_GAUGE || myConfig.iDisplayType == CD_POWERMANAGER_GRAPH)  // On recharge la jauge.
		{
			double fPercent = (double) myData.iPercentage / 100.;
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fPercent);
		}
		else if (myConfig.iDisplayType == CD_POWERMANAGER_ICONS)
		{
			cd_powermanager_draw_icon_with_effect (myData.bOnBattery);
		}
		
		// re-set the overlay if on sector
		if (! myData.bOnBattery)
			CD_APPLET_ADD_OVERLAY_ON_MY_ICON (myConfig.cEmblemIconName ? myConfig.cEmblemIconName : MY_APPLET_SHARE_DATA_DIR"/charge.svg", CAIRO_OVERLAY_MIDDLE);
		
		myData.iPrevPercentage = -1;
		myData.iPrevTime = -1;
		update_icon();
	}
	else  // sinon on signale par l'icone appropriee qu'aucune donnee n'est  accessible.
		CD_APPLET_SET_IMAGE_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/sector.svg");*/
CD_APPLET_RELOAD_END
