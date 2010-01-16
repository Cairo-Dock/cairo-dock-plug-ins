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

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-theme.h"
#include "applet-animation.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("Cairo-Penguin",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("Add a lively Penguin in your dock !\n"
	"Left click to change the animation,\n"
	"Middle-click to disturb him ^_^\n"
	"Tux images are taken from Pingus, some other characters are available or can be added easily."),
	"Fabounet (Fabrice Rey)")


CD_APPLET_INIT_BEGIN
	CD_APPLET_SET_STATIC_ICON;
	penguin_load_theme (myApplet, myConfig.cThemePath);
	
	penguin_start_animating_with_delay (myApplet);
	
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON,
		(CairoDockNotificationFunc) CD_APPLET_ON_CLICK_FUNC,
		CAIRO_DOCK_RUN_FIRST,
		myApplet);
	cairo_dock_register_notification (CAIRO_DOCK_MIDDLE_CLICK_ICON,
		(CairoDockNotificationFunc) CD_APPLET_ON_MIDDLE_CLICK_FUNC,
		CAIRO_DOCK_RUN_FIRST,
		myApplet);
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	penguin_remove_notfications();
	
	if (myData.iSidRestartDelayed != 0)
	{
		g_source_remove (myData.iSidRestartDelayed);
		myData.iSidRestartDelayed = 0;
	}
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On stoppe tout.
		if (myData.iSidRestartDelayed != 0)
		{
			g_source_remove (myData.iSidRestartDelayed);
			myData.iSidRestartDelayed = 0;
		}
		penguin_remove_notfications();
		
		//\_______________ On efface sa derniere position.
		PenguinAnimation *pAnimation = penguin_get_current_animation ();
		if (pAnimation != NULL)
		{
			GdkRectangle area;
			area.x = (myDock->container.iWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX;
			area.y = myDock->container.iHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight;
			area.width = pAnimation->iFrameWidth;
			area.height = pAnimation->iFrameHeight + myDock->container.bUseReflect * g_fReflectSize;
			gdk_window_invalidate_rect (myContainer->pWidget->window, &area, FALSE);
		}
		
		//\_______________ On recharge tout de zero (changement de theme).
		reset_data (myApplet);  // applet multi-instance => ok.
		
		penguin_load_theme (myApplet, myConfig.cThemePath);
		
		//\_______________ On libere le pingouin ou au contraire on le cloisonne.
		if (myConfig.bFree)
		{
			cairo_dock_detach_icon_from_dock (myIcon, myDock, myIcons.bSeparateIcons);
			cairo_dock_update_dock_size (myDock);
		}
		else
		{
			cairo_dock_insert_icon_in_dock (myIcon, myDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON);
		}
		
		penguin_start_animating (myApplet);
	}
	else
	{
		// rien a faire, la taille du pinguoin est fixe.
	}
CD_APPLET_RELOAD_END
