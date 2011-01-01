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
#include "string.h"

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-load-icons.h"
#include "applet-desktops.h"
#include "applet-draw.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("switcher"),
	2, 0, 9,
	CAIRO_DOCK_CATEGORY_APPLET_DESKTOP,
	N_("This applet allows you to interact with your workspaces :\n"
	" - switch between your workspaces (scroll up/down with the mouse),\n"
	" - name them (in the config),\n"
	" - quickly add/remove a workspace (in the menu),\n"
	" - show the desktop (middle-click or in the menu),\n"
	" - list all windows on each workspace ((middle-click or in the menu)\n"
	"It has 2 modes : compact (on 1 icon) and expanded (with a sub-dock)."),
	"Cchumi &amp; Fabounet")


CD_APPLET_INIT_BEGIN
	CD_APPLET_SET_STATIC_ICON;
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	cairo_dock_register_notification_on_object (&myDesktopMgr,
		NOTIFICATION_SCREEN_GEOMETRY_ALTERED,
		(CairoDockNotificationFunc) on_change_screen_geometry,
		CAIRO_DOCK_RUN_AFTER, myApplet);
	cairo_dock_register_notification_on_object (&myDesktopMgr,
		NOTIFICATION_DESKTOP_CHANGED,
		(CairoDockNotificationFunc) on_change_desktop,
		CAIRO_DOCK_RUN_AFTER, myApplet);
	cairo_dock_register_notification_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_CONFIGURED,
		(CairoDockNotificationFunc) on_window_configured,
		CAIRO_DOCK_RUN_AFTER, myApplet);
	cairo_dock_register_notification_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_ACTIVATED,
		(CairoDockNotificationFunc) on_change_active_window,
		CAIRO_DOCK_RUN_AFTER, myApplet);
	if (myConfig.bCompactView)
	{
		cairo_dock_register_notification_on_object (myContainer,
			NOTIFICATION_MOUSE_MOVED,
			(CairoDockNotificationFunc) on_mouse_moved,
			CAIRO_DOCK_RUN_AFTER, myApplet);
		if (myDesklet)
		{
			cairo_dock_register_notification_on_object (myContainer,
				NOTIFICATION_RENDER_DESKLET,
				(CairoDockNotificationFunc) on_render_desklet,
				CAIRO_DOCK_RUN_AFTER, myApplet);
			cairo_dock_register_notification_on_object (myContainer,
				NOTIFICATION_UPDATE_DESKLET,
				(CairoDockNotificationFunc) on_update_desklet,
				CAIRO_DOCK_RUN_AFTER, myApplet);
			cairo_dock_register_notification_on_object (myContainer,
				NOTIFICATION_LEAVE_DESKLET,
				(CairoDockNotificationFunc) on_leave_desklet,
				CAIRO_DOCK_RUN_AFTER, myApplet);
		}
	}
	
	cd_switcher_update_from_screen_geometry ();
	
	//\___________________ On affiche le numero du bureau courant.
	if (myConfig.bDisplayNumDesk)
	{
		int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1);
	}
	
	//\___________________ Dans le cas ou l'applet demarre au chargement de la session, le nombre de bureaux peut etre incorrect.
	if (cairo_dock_is_loading ())
		myData.iSidAutoRefresh = g_timeout_add_seconds (2, (GSourceFunc) cd_switcher_refresh_desktop_values, myApplet);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	if (myData.iSidRedrawMainIconIdle != 0)
	{
		g_source_remove (myData.iSidRedrawMainIconIdle);
	}
	if (myData.iSidAutoRefresh != 0)
	{
		g_source_remove (myData.iSidAutoRefresh);
	}
	if (myData.iSidPainIcons != 0)
		g_source_remove (myData.iSidPainIcons);
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	cairo_dock_remove_notification_func_on_object (&myDesktopMgr,
		NOTIFICATION_SCREEN_GEOMETRY_ALTERED,
		(CairoDockNotificationFunc) on_change_screen_geometry, myApplet);
	cairo_dock_remove_notification_func_on_object (&myDesktopMgr,
		NOTIFICATION_DESKTOP_CHANGED,
		(CairoDockNotificationFunc) on_change_desktop, myApplet);
	cairo_dock_remove_notification_func_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_CONFIGURED,
		(CairoDockNotificationFunc) on_window_configured, myApplet);
	cairo_dock_remove_notification_func_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_ACTIVATED,
		(CairoDockNotificationFunc) on_change_active_window, myApplet);
	cairo_dock_remove_notification_func_on_object (myContainer,
		NOTIFICATION_MOUSE_MOVED,
		(CairoDockNotificationFunc) on_mouse_moved, myApplet);
	cairo_dock_remove_notification_func_on_object (myContainer,
		NOTIFICATION_RENDER_DESKLET,
		(CairoDockNotificationFunc) on_render_desklet, myApplet);
	cairo_dock_remove_notification_func_on_object (myContainer,
		NOTIFICATION_UPDATE_DESKLET,
		(CairoDockNotificationFunc) on_update_desklet, myApplet);
	cairo_dock_remove_notification_func_on_object (myContainer,
		NOTIFICATION_LEAVE_DESKLET,
		(CairoDockNotificationFunc) on_leave_desklet, myApplet);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myData.iSidRedrawMainIconIdle != 0)
	{
		g_source_remove (myData.iSidRedrawMainIconIdle);
		myData.iSidRedrawMainIconIdle = 0;
	}
	
	cd_switcher_compute_nb_lines_and_columns ();
	
	cd_switcher_compute_desktop_coordinates (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY, &myData.switcher.iCurrentLine, &myData.switcher.iCurrentColumn);
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			if (myConfig.bCompactView)
			{
				CD_APPLET_SET_DESKLET_RENDERER ("Simple");
			}
			else
			{
				CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Slide", NULL);
			}
		}
		
		if (CD_APPLET_MY_OLD_CONTAINER != myContainer || ! myConfig.bCompactView)
		{
			cairo_dock_remove_notification_func_on_object (CD_APPLET_MY_OLD_CONTAINER,
				NOTIFICATION_MOUSE_MOVED,
				(CairoDockNotificationFunc) on_mouse_moved, myApplet);
			cairo_dock_remove_notification_func_on_object (CD_APPLET_MY_OLD_CONTAINER,
				NOTIFICATION_RENDER_DESKLET,
				(CairoDockNotificationFunc) on_render_desklet, myApplet);
			if (myConfig.bCompactView)
			{
				cairo_dock_register_notification_on_object (myContainer,
					NOTIFICATION_MOUSE_MOVED,
					(CairoDockNotificationFunc) on_mouse_moved,
					CAIRO_DOCK_RUN_AFTER, myApplet);
				if (myDesklet)
				{
					cairo_dock_register_notification_on_object (myDesklet,
						NOTIFICATION_RENDER_DESKLET,
						(CairoDockNotificationFunc) on_render_desklet,
						CAIRO_DOCK_RUN_AFTER, myApplet);
					cairo_dock_register_notification_on_object (myDesklet,
						NOTIFICATION_UPDATE_DESKLET,
						(CairoDockNotificationFunc) on_update_desklet,
						CAIRO_DOCK_RUN_AFTER, myApplet);
					cairo_dock_register_notification_on_object (myDesklet,
						NOTIFICATION_LEAVE_DESKLET,
						(CairoDockNotificationFunc) on_leave_desklet,
						CAIRO_DOCK_RUN_AFTER, myApplet);
				}
			}
		}
		if (myConfig.bDisplayNumDesk)
		{
			int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1);
		}
		else
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		
		cd_switcher_load_icons ();
	}
	else
	{
		if (myConfig.bMapWallpaper)  // on recharge le wallpaper a la taille de l'applet.
		{
			cd_switcher_load_desktop_bg_map_surface ();
		}
		if (myData.pDesktopBgMapSurface == NULL)
			cd_switcher_load_default_map_surface ();
		if (! myConfig.bCompactView)
			cd_switcher_trigger_paint_icons ();
	}
	
	cd_switcher_draw_main_icon ();
CD_APPLET_RELOAD_END
