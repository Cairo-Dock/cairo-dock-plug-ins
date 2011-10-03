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
#include "applet-init.h"
#include "applet-impulse.h"


CD_APPLET_DEFINE_BEGIN ("Impulse",
	2, 4, 0,
	CAIRO_DOCK_CATEGORY_APPLET_FUN,
	N_("Did you know that your dock can dance? :)\n"
	"If you click on this icon, the dock will dance!\n"
	"In fact, you will have a graphical equalizer into the dock\n"
	"It will analyse the signal given by PulseAudio."),
	"Matthieu Baerts (matttbe)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	// CD_APPLET_REDEFINE_TITLE (N_("Impulse")); do we have to translate it?
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_CAN_DOCK);
CD_APPLET_DEFINE_END

void _init_shared_memory (void)
{
	myData.pSharedMemory = g_new0 (CDSharedMemory, 1);
	myData.pSharedMemory->pIconsList = NULL; // without separators
	myData.pSharedMemory->bIsUpdatingIconsList = TRUE; // without separators
	myData.pSharedMemory->cIconAnimation = g_strdup (myConfig.cIconAnimation);
	myData.pSharedMemory->iNbAnimations = myConfig.iNbAnimations;
	myData.pSharedMemory->fMinValueToAnim = myConfig.fMinValueToAnim;
	myData.pSharedMemory->bStopAnimations = myConfig.bStopAnimations;
	myData.pSharedMemory->bNeedRefreshIfNotAnimated = FALSE; // will be TRUE when animated for the first time
	myData.pSharedMemory->pDock = myConfig.pDock;
	// myData.pSharedMemory->pApplet = myApplet;
}

void _free_shared_memory (void)
{
	g_free (myData.pSharedMemory->cIconAnimation);
	g_list_free (myData.pSharedMemory->pIconsList);
	g_free (myData.pSharedMemory);
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}

	// CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cIconImpulseOFF, "impulse-stopped.svg");

	myData.iSidAnimate = 0;
	myData.bPulseLaunched = FALSE;

	_init_shared_memory ();

	cd_impulse_im_setSourceIndex (myConfig.iSourceIndex);

	if (myConfig.bLaunchAtStartup)
		cd_impulse_start_animating_with_delay ();

	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;

	if (myData.iSidAnimate != 0)
		cd_impulse_stop_animations ();

	_free_shared_memory ();
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		gboolean bWasLaunched;
		if (myData.iSidAnimate != 0)
		{
			cd_impulse_stop_animations();
			bWasLaunched = TRUE;
		}
		else
			bWasLaunched = FALSE;

		cd_impulse_draw_current_state (); // if the user has specified other icons.

		// Shared Memory (cleaning)
		_free_shared_memory ();
		_init_shared_memory ();

		cd_impulse_im_setSourceIndex (myConfig.iSourceIndex);

		// if the icon has to be destroyed
		if (myConfig.bLaunchAtStartup && myConfig.bFree)
		{
			cairo_dock_detach_icon_from_dock (myIcon, myDock);
			cairo_dock_update_dock_size (myDock);
		}
		else
			cairo_dock_insert_icon_in_dock (myIcon, myDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);

		if (bWasLaunched || (myConfig.bLaunchAtStartup && myConfig.bFree)) // maybe the time has changed... or if it's automatically launched
			cd_impulse_launch_task ();
	}
CD_APPLET_RELOAD_END
