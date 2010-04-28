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
#include <stdlib.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-digital.h" //Digital html like renderer
#include "applet-config.h"
#include "applet-theme.h"
#include "applet-calendar.h"
#include "applet-backend-default.h"
#include "applet-notifications.h"
#include "applet-init.h"


CD_APPLET_PRE_INIT_BEGIN (N_("clock"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet displays time, date and a calandar.\n"
	"2 view are available : <b>numeric</b> and <b>analogic</b>.\n"
	" Analogic view is compatible with the Cairo-Clock's themes, and you can detach the applet to be a perfect clone of Cairo-Clock.\n"
	"It displays a <b>calendar</b> on left-click, which lets you <b>manage tasks</b>.\n"
	"It also supports alarms, and allows you to setup time and date.\n"
	"Left-click to show/hide the calendar, Middle-click to stop a notification,\n"
	"Double-click on a day to edit the tasks for this day."),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	pInterface->load_custom_widget = cd_clock_load_custom_widget;
	pInterface->save_custom_widget = cd_clock_save_custom_widget;
CD_APPLET_PRE_INIT_END


CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
	}
	
	if (myConfig.bSetName && myConfig.cLocation != NULL)
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cLocation+1);
	
	//\_______________ On charge notre theme.
	cd_clock_load_theme (myApplet);
	cd_clock_load_back_and_fore_ground (myApplet);
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
		cd_clock_load_textures (myApplet);
	
	///cd_clock_configure_digital (myApplet);  // mis en commentaire jusqu'a ce que ca soit fini.
	
	myData.cSystemLocation = g_strdup (g_getenv ("TZ"));
	myData.iLastCheckedMinute = -1;
	myData.iLastCheckedDay = -1;
	myData.iLastCheckedMonth = -1;
	myData.iLastCheckedYear = -1;
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOldStyle && myConfig.bShowSeconds && myConfig.iSmoothAnimationDuration != 0)
	{
		CD_APPLET_REGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
		cairo_dock_launch_animation (myContainer);
	}
	
	//\_______________ On enregistre les backends de gestion des taches.
	cd_clock_register_backend_default (myApplet);
	
	//\_______________ On liste les taches (apres avoir le temps courant).
	cd_clock_set_current_backend (myApplet);
	
	cd_clock_init_time (myApplet);
	cd_clock_list_tasks (myApplet);
	
	//\_______________ On lance le timer.
	if (! myConfig.bShowSeconds)  // pour ne pas attendre 1 mn avant d'avoir le dessin.
		cd_clock_update_with_time (myApplet);
	myData.iSidUpdateClock = g_timeout_add_seconds ((myConfig.bShowSeconds ? 1: 60), (GSourceFunc) cd_clock_update_with_time, (gpointer) myApplet);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
	
	//\_______________ On stoppe le timer.
	g_source_remove (myData.iSidUpdateClock);
	myData.iSidUpdateClock = 0;

	cd_clock_free_timezone_list ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
	}
	
	///cd_clock_configure_digital (myApplet);  // mis en commentaire jusqu'a ce que ca soit fini.
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On stoppe le timer.
		g_source_remove (myData.iSidUpdateClock);
		myData.iSidUpdateClock = 0;
		CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
		//\_______________ On efface tout
		cd_clock_clear_theme (myApplet, TRUE);
		//\_______________ On charge notre theme.
		cd_clock_load_theme (myApplet);
		//\_______________ On charge les surfaces d'avant et arriere-plan.
		cd_clock_load_back_and_fore_ground (myApplet);
		//\_______________ On charge les textures.
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
		{
			cd_clock_load_textures (myApplet);
			cairo_dock_launch_animation (myContainer);
		}
		
		if (myConfig.bSetName && myConfig.cLocation != NULL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cLocation+1);
		
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOldStyle && myConfig.bShowSeconds && myConfig.iSmoothAnimationDuration != 0)
		{
			CD_APPLET_REGISTER_FOR_UPDATE_ICON_SLOW_EVENT;
			cairo_dock_launch_animation (myContainer);
		}
		
		//\_______________ On reliste les taches avec le nouveau backend.
		cd_clock_set_current_backend (myApplet);
		cd_clock_list_tasks (myApplet);
		
		//\_______________ On relance le timer.
		myData.iLastCheckedMinute = -1;
		myData.iLastCheckedDay = -1;
		myData.iLastCheckedMonth = -1;
		myData.iLastCheckedYear = -1;
		cd_clock_update_with_time (myApplet);
		myData.iSidUpdateClock = g_timeout_add_seconds ((myConfig.bShowSeconds ? 1: 60), (GSourceFunc) cd_clock_update_with_time, (gpointer) myApplet);
	}
	else
	{
		//\_______________ On charge les surfaces d'avant et arriere-plan et les textures, les rsvg_handle ne dependent pas de la taille.
		cd_clock_clear_theme (myApplet, FALSE);
		cd_clock_load_back_and_fore_ground (myApplet);
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
			cd_clock_load_textures (myApplet);

		cd_clock_update_with_time (myApplet);
	}
CD_APPLET_RELOAD_END
