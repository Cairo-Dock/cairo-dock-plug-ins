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
#include "applet-init.h"
#include "applet-draw.h"

CD_APPLET_DEFINITION (N_("RSSreader"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet is a RSS feed reader\n"
	"(you can add as many instances you want)\n"
	" USAGE:\n"
	"  - Left-clic : Display the feed lines in a dialog (optional in desklet).\n"
	"  - Middle-clic : Refresh the feed.\n"
	"  - Add/Modify the RSS feed to display with either Drag'n Drop\n"
	"    a valid RSS feed Url,\"Paste a new RSS Url\" from menu\n"
	"    or with the Configuration Panel.\n"
	"  - Use \"Open in you web browser\" from menu to display\n"
	"    the RSS feed in your browser."),
	"Yann Dulieu (Nochka85)")


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	
	myData.bUpdateIsManual = FALSE;
	myData.cLastFirstFeedLine = NULL;
	myData.cLastSecondFeedLine = NULL;
	myData.cAllFeedLines = g_strdup_printf ("%s\n", D_("Please wait ..."));
	myData.cSingleFeedLine = g_strsplit (myData.cAllFeedLines,"\n",0);
	
	
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
		myData.bLastWasDocked = FALSE;
	}
	else
	{			
			CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cName != NULL ? myConfig.cName : "RSSreader");
			cd_rssreader_upload_feeds_TASK (myApplet);
			myData.bLastWasDocked = TRUE;
	}
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	if (myDesklet)
	{
		cd_applet_update_my_icon (myApplet, myIcon, myContainer);
		cd_rssreader_upload_feeds_TASK (myApplet);
	}
		
	CD_APPLET_REDRAW_MY_ICON; // On force un refresh
	
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}	
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		myData.bUpdateIsManual = FALSE;
		myData.cLastFirstFeedLine = NULL;
		myData.cLastSecondFeedLine = NULL;
		myData.cAllFeedLines = g_strdup_printf ("%s\n", D_("Please wait ..."));
		myData.cSingleFeedLine = g_strsplit (myData.cAllFeedLines,"\n",0);
		
		if (! myDesklet)
		{
				CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
				CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cName != NULL ? myConfig.cName : "RSSreader");
				cd_rssreader_upload_feeds_TASK (myApplet);
				myData.bLastWasDocked = TRUE;
		}
	}	
	
	if (myDesklet)
	{
		cd_applet_update_my_icon (myApplet, myIcon, myContainer);		
		
		// Si je ne fais pas ce qui suit, j'ai un plantage lorsqu'on détache l'applet :/
		if (cairo_dock_task_is_active (myData.pTask))
		{			
			cairo_dock_stop_task (myData.pTask);
			if (! myData.bLastWasDocked)
				cd_rssreader_upload_feeds_TASK (myApplet);
	    }
	    else
	   		cd_rssreader_upload_feeds_TASK (myApplet);
	   		
	   	myData.bLastWasDocked = FALSE;
	}
	
CD_APPLET_RELOAD_END
