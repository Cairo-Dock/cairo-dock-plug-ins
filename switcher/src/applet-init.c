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


CD_APPLET_DEFINE_BEGIN (N_("switcher"),
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
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_ALLOW_EMPTY_TITLE
CD_APPLET_DEFINE_END


static gboolean _get_desktop_names (G_GNUC_UNUSED gpointer data)
{
	// retrieve the desktop names
	myData.cDesktopNames = gldi_desktop_get_names ();
	myData.iNbNames = g_strv_length (myData.cDesktopNames);
	cd_debug ("got desktop names: %s, ...", myData.cDesktopNames ? myData.cDesktopNames[0] : NULL);
	// if no names are set yet, set the names stored in the config.
	if ((myData.cDesktopNames == NULL || *myData.cDesktopNames == NULL) && myConfig.cDesktopNames != NULL)
	{
		gldi_desktop_set_names (myConfig.cDesktopNames);
	}
	myData.iSidGetDesktopNames = 0;
	return FALSE;
}

CD_APPLET_INIT_BEGIN
	//\_______________ Rregister the notifications we'll need
	CD_APPLET_SET_STATIC_ICON;
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	gldi_object_register_notification (&myDesktopMgr,
		NOTIFICATION_DESKTOP_GEOMETRY_CHANGED,
		(GldiNotificationFunc) on_change_screen_geometry,
		GLDI_RUN_AFTER, myApplet);
	gldi_object_register_notification (&myDesktopMgr,
		NOTIFICATION_DESKTOP_CHANGED,
		(GldiNotificationFunc) on_change_desktop,
		GLDI_RUN_AFTER, myApplet);
	gldi_object_register_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_SIZE_POSITION_CHANGED,
		(GldiNotificationFunc) on_window_size_position_changed,
		GLDI_RUN_AFTER, myApplet);
	gldi_object_register_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_Z_ORDER_CHANGED,
		(GldiNotificationFunc) on_change_window_order,
		GLDI_RUN_AFTER, myApplet);  // also handles creation/destruction of a window, since this event will change the stack order
	gldi_object_register_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_STATE_CHANGED,
		(GldiNotificationFunc) on_change_window_state,
		GLDI_RUN_AFTER, myApplet);  // minimizing a window doesn't change the z-order, so we register to this event too
	gldi_object_register_notification (&myDesktopMgr,
		NOTIFICATION_DESKTOP_NAMES_CHANGED,
		(GldiNotificationFunc) on_change_desktop_names,
		GLDI_RUN_AFTER, myApplet);
	if (myConfig.bCompactView)  // in this mode we need to monitor the cursor
	{
		gldi_object_register_notification (myContainer,
			NOTIFICATION_MOUSE_MOVED,
			(GldiNotificationFunc) on_mouse_moved,
			GLDI_RUN_AFTER, myApplet);
		if (myDesklet)  // in this mode we draw the name of the pointed desktop
		{
			gldi_object_register_notification (&myDeskletObjectMgr,
				NOTIFICATION_RENDER,
				(GldiNotificationFunc) on_render_desklet,
				GLDI_RUN_AFTER, myApplet);  // we register on the parent manager to be drawn after the desklet, until the notifications order is reversed.
			gldi_object_register_notification (myContainer,
				NOTIFICATION_UPDATE,
				(GldiNotificationFunc) on_update_desklet,
				GLDI_RUN_AFTER, myApplet);
			gldi_object_register_notification (myContainer,
				NOTIFICATION_LEAVE_DESKLET,
				(GldiNotificationFunc) on_leave_desklet,
				GLDI_RUN_AFTER, myApplet);
		}
	}
	if (myConfig.iIconDrawing == SWICTHER_MAP_WALLPAPER)
	{
		gldi_object_register_notification (&myDesktopMgr,
			NOTIFICATION_DESKTOP_WALLPAPER_CHANGED,
			(GldiNotificationFunc) on_change_wallpaper,
			GLDI_RUN_AFTER, myApplet); // FIRST: we want to intercept the signal
	}

	//\___________________ load the desktops with a delay (because it's quite heavy and because the desktop may not be up-to-date at the very beginning of the session).
	if (myDesklet)
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	cd_switcher_trigger_update_from_screen_geometry (FALSE);  // FALSE = delayed
	
	//\___________________ retrieve the desktop names (wait a fex seconds so that the settings daemon can set them).
	myData.iSidGetDesktopNames = g_timeout_add_seconds (2, _get_desktop_names, NULL);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ Discard the pending drawings.
	if (myData.iSidRedrawMainIconIdle != 0)
	{
		g_source_remove (myData.iSidRedrawMainIconIdle);
	}
	if (myData.iSidUpdateIdle != 0)
	{
		g_source_remove (myData.iSidUpdateIdle);
	}
	if (myData.iSidGetDesktopNames != 0)
	{
		g_source_remove (myData.iSidGetDesktopNames);
	}
	
	//\_______________ Unregister notifications
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	gldi_object_remove_notification (&myDesktopMgr,
		NOTIFICATION_DESKTOP_GEOMETRY_CHANGED,
		(GldiNotificationFunc) on_change_screen_geometry, myApplet);
	gldi_object_remove_notification (&myDesktopMgr,
		NOTIFICATION_DESKTOP_CHANGED,
		(GldiNotificationFunc) on_change_desktop, myApplet);
	gldi_object_remove_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_SIZE_POSITION_CHANGED,
		(GldiNotificationFunc) on_window_size_position_changed, myApplet);
	gldi_object_remove_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_Z_ORDER_CHANGED,
		(GldiNotificationFunc) on_change_window_order, myApplet);
	gldi_object_remove_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_STATE_CHANGED,
		(GldiNotificationFunc) on_change_window_state, myApplet);
	gldi_object_remove_notification (&myDesktopMgr,
		NOTIFICATION_DESKTOP_NAMES_CHANGED,
		(GldiNotificationFunc) on_change_desktop_names, myApplet);
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_MOUSE_MOVED,
		(GldiNotificationFunc) on_mouse_moved, myApplet);
	gldi_object_remove_notification (&myDeskletObjectMgr,
		NOTIFICATION_RENDER,
		(GldiNotificationFunc) on_render_desklet, myApplet);
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_UPDATE,
		(GldiNotificationFunc) on_update_desklet, myApplet);
	gldi_object_remove_notification (myContainer,
		NOTIFICATION_LEAVE_DESKLET,
		(GldiNotificationFunc) on_leave_desklet, myApplet);
	gldi_object_remove_notification (&myDesktopMgr,
		NOTIFICATION_DESKTOP_WALLPAPER_CHANGED,
		(GldiNotificationFunc) on_change_wallpaper, myApplet);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myData.iSidRedrawMainIconIdle != 0)
	{
		g_source_remove (myData.iSidRedrawMainIconIdle);
		myData.iSidRedrawMainIconIdle = 0;
	}
	
	// if our size has changed, re-compute the grid.
	if (myData.iSidUpdateIdle == 0)
	{
		cd_switcher_compute_nb_lines_and_columns ();
		
		cd_switcher_compute_coordinates_from_desktop (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY, &myData.switcher.iCurrentLine, &myData.switcher.iCurrentColumn);
	}
	
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
		
		// in case our container or our mode has changed (desklet <-> dock, compact <-> expanded), register to the needed notifications.
		gldi_object_remove_notification (CD_APPLET_MY_OLD_CONTAINER,
			NOTIFICATION_MOUSE_MOVED,
			(GldiNotificationFunc) on_mouse_moved, myApplet);
		gldi_object_remove_notification (&myDeskletObjectMgr,
			NOTIFICATION_RENDER,
			(GldiNotificationFunc) on_render_desklet, myApplet);
		gldi_object_remove_notification (CD_APPLET_MY_OLD_CONTAINER,
			NOTIFICATION_UPDATE,
			(GldiNotificationFunc) on_update_desklet, myApplet);
		gldi_object_remove_notification (CD_APPLET_MY_OLD_CONTAINER,
			NOTIFICATION_LEAVE_DESKLET,
			(GldiNotificationFunc) on_leave_desklet, myApplet);
		gldi_object_remove_notification (&myDesktopMgr,
			NOTIFICATION_DESKTOP_WALLPAPER_CHANGED,
			(GldiNotificationFunc) on_change_wallpaper, myApplet);
		
		if (myConfig.bCompactView)
		{
			gldi_object_register_notification (myContainer,
				NOTIFICATION_MOUSE_MOVED,
				(GldiNotificationFunc) on_mouse_moved,
				GLDI_RUN_AFTER, myApplet);
			if (myDesklet)
			{
				gldi_object_register_notification (&myDeskletObjectMgr,
					NOTIFICATION_RENDER,
					(GldiNotificationFunc) on_render_desklet,
					GLDI_RUN_AFTER, myApplet);
				gldi_object_register_notification (myContainer,
					NOTIFICATION_UPDATE,
					(GldiNotificationFunc) on_update_desklet,
					GLDI_RUN_AFTER, myApplet);
				gldi_object_register_notification (myContainer,
					NOTIFICATION_LEAVE_DESKLET,
					(GldiNotificationFunc) on_leave_desklet,
					GLDI_RUN_AFTER, myApplet);
			}
		}

		if (myConfig.iIconDrawing == SWICTHER_MAP_WALLPAPER)
		{
			gldi_object_register_notification (&myDesktopMgr,
				NOTIFICATION_DESKTOP_WALLPAPER_CHANGED,
				(GldiNotificationFunc) on_change_wallpaper,
				GLDI_RUN_AFTER, myApplet);
		}

		if (myData.iSidUpdateIdle == 0)
		{
			if (myConfig.bDisplayNumDesk)
			{
				int iIndex = cd_switcher_compute_index_from_desktop (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1);
			}
			else
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			
			cd_switcher_load_icons ();
		}
	}
	else if (myData.iSidUpdateIdle == 0)  // if only our size has changed, re-load the images.
	{
		if (myConfig.iIconDrawing == SWICTHER_MAP_WALLPAPER)  // on recharge le wallpaper a la taille de l'applet.
		{
			cd_switcher_load_desktop_bg_map_surface ();
		}
		if (myData.pDesktopBgMapSurface == NULL)
			cd_switcher_load_default_map_surface ();
		///if (! myConfig.bCompactView)
		///	cd_switcher_trigger_paint_icons ();
	}
	
	// redraw.
	if (myData.iSidUpdateIdle == 0)
		cd_switcher_draw_main_icon ();
CD_APPLET_RELOAD_END
