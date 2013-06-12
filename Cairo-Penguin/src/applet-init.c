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


CD_APPLET_DEFINE_BEGIN ("Cairo-Penguin",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_APPLET_FUN,
	N_("Add a lively Penguin in your dock !\n"
	"Left click to change the animation,\n"
	"Middle-click to disturb him ^_^\n"
	"Tux images are taken from Pingus, some other characters are available or can be added easily."),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_REDEFINE_TITLE (N_("Cairo-Penguin"));
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_CAN_DOCK);
CD_APPLET_DEFINE_END


CD_APPLET_INIT_BEGIN
	CD_APPLET_SET_STATIC_ICON;
	penguin_load_theme (myApplet, myConfig.cThemePath);
	
	penguin_start_animating_with_delay (myApplet);
	
	gldi_object_register_notification (myContainer,
		NOTIFICATION_CLICK_ICON,
		(GldiNotificationFunc) CD_APPLET_ON_CLICK_FUNC,
		GLDI_RUN_FIRST,
		myApplet);
	gldi_object_register_notification (myContainer,
		NOTIFICATION_MIDDLE_CLICK_ICON,
		(GldiNotificationFunc) CD_APPLET_ON_MIDDLE_CLICK_FUNC,
		GLDI_RUN_FIRST,
		myApplet);
	gldi_object_register_notification (myContainer,
		NOTIFICATION_BUILD_CONTAINER_MENU,
		(GldiNotificationFunc) on_build_container_menu,
		GLDI_RUN_FIRST,
		myApplet);
	gldi_object_register_notification (myContainer,
		NOTIFICATION_BUILD_ICON_MENU,
		(GldiNotificationFunc) CD_APPLET_ON_BUILD_MENU_FUNC,
		GLDI_RUN_FIRST,
		myApplet);
	gldi_object_register_notification (myDock,
		NOTIFICATION_DESTROY,
		(GldiNotificationFunc) cd_on_dock_destroyed,
		GLDI_RUN_AFTER,
		myApplet);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_CLICK_ICON,
		(GldiNotificationFunc) CD_APPLET_ON_CLICK_FUNC,
		myApplet);
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_MIDDLE_CLICK_ICON,
		(GldiNotificationFunc) CD_APPLET_ON_MIDDLE_CLICK_FUNC,
		myApplet);
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_BUILD_ICON_MENU,
		(GldiNotificationFunc) CD_APPLET_ON_BUILD_MENU_FUNC,
		myApplet);
	gldi_object_remove_notification (myDock,
		NOTIFICATION_DESTROY,
		(GldiNotificationFunc) cd_on_dock_destroyed,
		myApplet);
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
			area.height = pAnimation->iFrameHeight + myDock->container.bUseReflect * myDock->iIconSize * myIconsParam.fReflectHeightRatio;
			cairo_dock_redraw_container_area (myContainer, &area);
		}
		
		//\_______________ On recharge tout de zero (changement de theme).
		cd_penguin_reset_data (myApplet);
		
		penguin_load_theme (myApplet, myConfig.cThemePath);
		
		//\_______________ On libere le pingouin ou au contraire on le cloisonne.
		if (myConfig.bFree)
		{
			gldi_icon_detach (myIcon);
		}
		else
		{
			gldi_icon_insert_in_container (myIcon, myContainer, ! CAIRO_DOCK_ANIMATE_ICON);
		}
		
		penguin_start_animating (myApplet);
	}
	else
	{
		// rien a faire, la taille du pingouin est fixe.
	}
CD_APPLET_RELOAD_END
