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

//\________________ Add your name in the copyright file (and / or modify your name here)

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-init.h"
#include "applet-xml.h"


CD_APPLET_DEFINITION ("Doncky",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This applet allows you to write texts and monitor your system\n"
	"with a \"text style desklet\".\n"
	"USAGE:\n"
	"  - NEED TO BE DETACHED FROM THE DOCK TO WORK !\n"
	"  - Middle-clic to reload values"),
	"Yann Dulieu (Nochka85)")



//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN

	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
	}
		
	cd_doncky_free_item_list (myApplet);	
	if (! cd_doncky_readxml (myApplet))
		cd_debug ("Doncky-debug : ---------------------->  Bad XML format !");
	
	// REPRIS DE SYSTEM-MONITOR:
	myData.pClock = g_timer_new ();
	
	if (myConfig.cSystemMonitorClass)
		CD_APPLET_MANAGE_APPLICATION (myConfig.cSystemMonitorClass);
	
	cd_applet_update_my_icon (myApplet);
	
	
	//~ CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	//~ CD_APPLET_REGISTER_FOR_DOUBLE_CLICK_EVENT;
	// CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	//~ CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	
	//\_______________ On lance le timer.   
    myData.pPeriodicRefreshTask = cairo_dock_new_task (myConfig.iCheckInterval,
			(CairoDockGetDataAsyncFunc) cd_launch_command,
			(CairoDockUpdateSyncFunc) cd_retrieve_command_result,
			myApplet);
	cairo_dock_launch_task (myData.pPeriodicRefreshTask);
	
	myData.bAcquisitionOK = TRUE;
	
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	// CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	// CD_APPLET_UNREGISTER_FOR_DOUBLE_CLICK_EVENT;
	// CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	// CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_MANAGE_APPLICATION (NULL);	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
			CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
		}
		
		// REPRIS DE SYSTEM-MONITOR:
		myData.bAcquisitionOK = TRUE;
		myData.fPrevCpuPercent = 0;
		myData.fPrevRamPercent = 0;
		myData.fPrevSwapPercent = 0;
		myData.fPrevGpuTempPercent = 0;
		myData.iTimerCount = 0;
		
		CD_APPLET_MANAGE_APPLICATION (myConfig.cSystemMonitorClass);
				
		cairo_dock_relaunch_task_immediately (myData.pPeriodicRefreshTask, myConfig.iCheckInterval);
				
		cd_doncky_free_item_list (myApplet);		
		if (! cd_doncky_readxml (myApplet))
			cd_debug ("Doncky-debug : ---------------------->  Bad XML format !");
	}
	else 
	{
		myData.fPrevCpuPercent = 0;
		myData.fPrevRamPercent = 0;
		myData.fPrevSwapPercent = 0;
		myData.fPrevGpuTempPercent = 0;
	}
	
	// redessin.
	cd_applet_update_my_icon (myApplet);
		
CD_APPLET_RELOAD_END
