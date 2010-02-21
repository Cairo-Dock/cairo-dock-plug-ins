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

// REPRIS DE SYSTEM-MONITOR:
#include "applet-top.h"
#include "applet-monitor.h"



CD_APPLET_DEFINITION (N_("Doncky"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet allows you to write texts and monitor your system\n"
	"with a \"text style desklet\".\n"
	"USAGE:\n"
	"  - NEED TO BE DETACHED FROM THE DOCK TO WORK !\n"
	"  - Middle-clic to reload values"),
	"Yann Dulieu (Nochka85)")


// REPRIS DE SYSTEM-MONITOR:
static gboolean _unthreaded_task (CairoDockModuleInstance *myApplet)
{
	cd_sysmonitor_get_data (myApplet);
	cd_sysmonitor_update_from_data (myApplet);
	return TRUE;
}







//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN

	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	else
	{
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // en mode dock l'image de l'icone est statique.
	}
	
	cd_doncky_free_item_list (myApplet);	
	cd_doncky_readxml (myApplet);
	
	
	
	
	// REPRIS DE SYSTEM-MONITOR:
	// Initialisation de la tache periodique de mesure.
	myData.pClock = g_timer_new ();
	if (myConfig.bShowNvidia || (myConfig.bShowCpu && myConfig.bShowRam))
		myData.pPeriodicTask = cairo_dock_new_task (myConfig.iCheckInterval,
			(CairoDockGetDataAsyncFunc) cd_sysmonitor_get_data,
			(CairoDockUpdateSyncFunc) cd_sysmonitor_update_from_data,  // _unthreaded_task
			myApplet);
	else
		myData.pPeriodicTask = cairo_dock_new_task (myConfig.iCheckInterval,
			(CairoDockGetDataAsyncFunc) NULL,
			(CairoDockUpdateSyncFunc) _unthreaded_task,
			myApplet);
	myData.bAcquisitionOK = TRUE;
	cairo_dock_launch_task (myData.pPeriodicTask);
	
	// On gere l'appli "moniteur systeme".
	if (myConfig.cSystemMonitorClass)
		CD_APPLET_MANAGE_APPLICATION (myConfig.cSystemMonitorClass);
	
	
	
	
	// en mode desklet on redessine l'icone avec le message d'attente.
	if (myDesklet)
		cd_applet_update_my_icon (myApplet);
		
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	
	//\_______________ On lance le timer.
    myData.iSidPeriodicRefresh = g_timeout_add_seconds (myConfig.iCheckInterval, (GSourceFunc) cd_doncky_periodic_refresh, (gpointer) myApplet);  // On raffraichit toutes les secondes au MAX
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	
	//\_______________ On stoppe le timer en cours.
	g_source_remove (myData.iSidPeriodicRefresh);
	myData.iSidPeriodicRefresh = 0;

	CD_APPLET_MANAGE_APPLICATION (NULL);
	
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		// REPRIS DE SYSTEM-MONITOR:
		myData.bAcquisitionOK = TRUE;
		myData.fPrevCpuPercent = 0;
		myData.fPrevRamPercent = 0;
		myData.fPrevSwapPercent = 0;
		myData.fPrevGpuTempPercent = 0;
		myData.iTimerCount = 0;
		cairo_dock_relaunch_task_immediately (myData.pPeriodicTask, myConfig.iCheckInterval);
		
		g_free (myData.pTopList);
		myData.pTopList = NULL;
		if (myData.pTopTask != NULL)
			cairo_dock_change_task_frequency (myData.pTopTask, myConfig.iCheckInterval);
		
		CD_APPLET_MANAGE_APPLICATION (myConfig.cSystemMonitorClass);
		
		//\_______________ On stoppe le timer en cours.
		g_source_remove (myData.iSidPeriodicRefresh);
		myData.iSidPeriodicRefresh = 0;
		//\_______________ On relance le timer.
	    myData.iSidPeriodicRefresh = g_timeout_add_seconds (myConfig.iCheckInterval, (GSourceFunc) cd_doncky_periodic_refresh, (gpointer) myApplet);  // On raffraichit toutes les secondes au MAX
		
		cd_doncky_free_item_list (myApplet);	
		cd_doncky_readxml (myApplet);
		
		// redessin.
		if (myDesklet)
			cd_applet_update_my_icon (myApplet);
		else
			CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;		
	}
	else 
	{
		// REPRIS DE SYSTEM-MONITOR:			
		if (! cairo_dock_task_is_running (myData.pPeriodicTask))
		{
			myData.fPrevCpuPercent = 0;
			myData.fPrevRamPercent = 0;
			myData.fPrevSwapPercent = 0;
			myData.fPrevGpuTempPercent = 0;
			cd_sysmonitor_update_from_data (myApplet);
		}
	}
	
	// en mode desklet on redessine l'icone aux nouvelles dimensions.
	if (myDesklet)
		cd_applet_update_my_icon (myApplet);
		
CD_APPLET_RELOAD_END
