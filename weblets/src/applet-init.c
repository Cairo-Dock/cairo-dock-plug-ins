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
#include "applet-widget.h"
#include "applet-init.h"

CD_APPLET_DEFINITION ("weblets",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("The weblets applet allows you to show an interactive web page on your desktop.\n"
	"You can select your web page, set the desired scrolling,\n"
	"and hide the scrollbars to get a real 'crop' of the page."),
	"Tofe (Christophe Chapuis)")

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN

	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	
	if (myDesklet != NULL)  // on cree le weblet pour avoir qqch a afficher dans le desklet.
	{
		if( myData.pGtkMozEmbed == NULL )
		{
			weblet_build_and_show (myApplet);
		}
		CD_APPLET_SET_STATIC_DESKLET;
		
		// mise en place du timer
		myData.pRefreshTimer = cairo_dock_new_task (myConfig.iReloadTimeout,
			NULL,
			(CairoDockUpdateSyncFunc) cd_weblets_refresh_page,
			myApplet);
		cairo_dock_launch_task (myData.pRefreshTimer); // ceci lance au moins une fois le chargement de la page
	}
	else  // en desklet on n'affiche pas l'icone.
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;

CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if( myData.pRefreshTimer )
		{
			cairo_dock_free_task( myData.pRefreshTimer );
			myData.pRefreshTimer = NULL;
		}
		
		if (myDock)  // en desklet on n'affiche pas l'icone.
			CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
		
		if (myData.pGtkMozEmbed == NULL)
		{
			if (myDesklet != NULL)  // on cree le terminal pour avoir qqch a afficher dans le desklet.
				weblet_build_and_show (myApplet);
		}
		else if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED)
		{
			if (myDesklet != NULL)  // il faut passer du dialogue au desklet.
			{
				myData.pGtkMozEmbed = cairo_dock_steal_widget_from_its_container (myData.pGtkMozEmbed);
				cairo_dock_dialog_unreference (myData.dialog);
				myData.dialog = NULL;
				cairo_dock_add_interactive_widget_to_desklet (myData.pGtkMozEmbed, myDesklet);
				cairo_dock_set_desklet_renderer_by_name (myDesklet, NULL, NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);  // pou rempecher le clignotement du au double-buffer.
				CD_APPLET_SET_STATIC_DESKLET;
			}
			else  // il faut passer du desklet au dialogue
			{
				myData.dialog =  cd_weblets_build_dialog(myApplet);
				
				cairo_dock_hide_dialog (myData.dialog);
			}
		}
		else
		{
			cairo_dock_set_desklet_margin (myDesklet, myConfig.iRightMargin);
		}

		// on remet en place un timer tout frais
		myData.pRefreshTimer = cairo_dock_new_task (myConfig.iReloadTimeout,
			NULL,
			(CairoDockUpdateSyncFunc) cd_weblets_refresh_page,
			myApplet);
		cairo_dock_launch_task (myData.pRefreshTimer); // ceci lance au moins une fois le chargement de la page
	}
CD_APPLET_RELOAD_END
