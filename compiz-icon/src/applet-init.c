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
#include "applet-load-icon.h"
#include "applet-compiz.h"

#define CD_COMPIZ_CHECK_TIME 4

CD_APPLET_DEFINITION ("compiz-icon",
	1, 6, 2,
	CAIRO_DOCK_CATEGORY_DESKTOP,
	N_("This applet allows you to manage compiz and other windows manager\n"
	"The sub-dock gives you to acces to CCSM, Emerald and some basic Compiz actions.\n"
	"You can bind one of these actions with the middle-click.\n"
	"The configuration panel gives you some options to launch Compiz."),
	"ChAnGFu (Rémy Robertson) (thanks to Coz for his icons)")


CD_APPLET_INIT_BEGIN
	cd_compiz_build_icons ();
	
	if (myConfig.bAutoReloadDecorator || myConfig.bAutoReloadCompiz) {
		myData.bDecoratorRestarted = FALSE;
		myData.iCompizIcon = -1;  // force le dessin.
		if (! myConfig.forceConfig) // on fait comme si c'est nous qui l'avons mis dans l'etat actuel.
			myData.bCompizRestarted = TRUE;
		
		myData.pTask = cairo_dock_new_task (CD_COMPIZ_CHECK_TIME,
			(CairoDockGetDataAsyncFunc) cd_compiz_read_data,
			(CairoDockUpdateSyncFunc) cd_compiz_update_from_data,
			myApplet);
		cairo_dock_launch_task (myData.pTask);
	}
	else {
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cUserImage[COMPIZ_DEFAULT], "default.svg");
	}
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		
		//\________________ les icones ont pu changer, ainsi que l'inhibition.
		CD_APPLET_DELETE_MY_ICONS_LIST;
		
		if (cairo_dock_task_is_active (myData.pTask) && ! myConfig.bAutoReloadDecorator && ! myConfig.bAutoReloadCompiz) {
			cairo_dock_stop_task (myData.pTask);
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cUserImage[COMPIZ_DEFAULT], "default.svg");
		}
		else if (! cairo_dock_task_is_active (myData.pTask) && (myConfig.bAutoReloadDecorator || myConfig.bAutoReloadCompiz)) {
			myData.iCompizIcon = -1;
			myData.bDecoratorRestarted = FALSE;
			if (! myConfig.forceConfig) // on fait comme si c'est nous qui l'avons mis dans l'etat actuel.
				myData.bCompizRestarted = TRUE;
			
			cairo_dock_launch_task (myData.pTask);
		}
		else {
			if (cairo_dock_task_is_active (myData.pTask))
				myData.iCompizIcon = -1;
			else {
				CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cUserImage[COMPIZ_DEFAULT], "default.svg");
			}
		}
		cd_compiz_build_icons ();
		
	}
	/*else if (myDesklet != NULL) {
		gpointer pConfig[2] = {GINT_TO_POINTER (FALSE), GINT_TO_POINTER (FALSE)};
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", pConfig);
	}
	else {
		//Rien a faire
	}*/
CD_APPLET_RELOAD_END
