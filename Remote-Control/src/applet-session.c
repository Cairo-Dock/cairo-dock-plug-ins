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

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-icon-finder.h"
#include "applet-session.h"


void cd_do_open_session (void)
{
	if (cd_do_session_is_running ())  // session already running
		return;
	
	// register to draw on dock.
	/**if (cd_do_session_is_off ())
	{
		cairo_dock_register_notification_on_container (CAIRO_CONTAINER (g_pMainDock),
			CAIRO_DOCK_UPDATE_DOCK,
			(CairoDockNotificationFunc) cd_do_update_container,
			CAIRO_DOCK_RUN_AFTER, NULL);
		cairo_dock_register_notification_on_container (CAIRO_CONTAINER (g_pMainDock),
			CAIRO_DOCK_RENDER_DOCK,
			(CairoDockNotificationFunc) cd_do_render,
			CAIRO_DOCK_RUN_AFTER, NULL);
	}*/
	
	// wait for keyboard input.
	cairo_dock_register_notification (CAIRO_DOCK_KEY_PRESSED, (CairoDockNotificationFunc) cd_do_key_pressed, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_STOP_ICON, (CairoDockNotificationFunc) cd_do_check_icon_stopped, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_WINDOW_ACTIVATED, (CairoDockNotificationFunc) cd_do_check_active_dock, CAIRO_DOCK_RUN_AFTER, NULL);
	
	myData.sCurrentText = g_string_sized_new (20);
	myData.iPromptAnimationCount = 0;
	if (myData.pArrowImage == NULL)
	{
		myData.pArrowImage = cairo_dock_create_image_buffer (MY_APPLET_SHARE_DATA_DIR"/arrows.svg",
			g_pMainDock->iMaxDockHeight,
			g_pMainDock->iMaxDockHeight,
			CAIRO_DOCK_KEEP_RATIO);
	}
	
	// set initial position.
	myData.pCurrentDock = NULL;
	myData.pCurrentIcon =  NULL;
	
	CairoDock *pDock = g_pMainDock;
	Icon *pIcon = NULL;
	int n = g_list_length (g_pMainDock->icons);
	if (n > 0)
	{
		pIcon =  g_list_nth_data (pDock->icons, (n-1) / 2);
		if (CAIRO_DOCK_IS_SEPARATOR (pIcon) && n > 1)
			pIcon = g_list_nth_data (pDock->icons, (n+1) / 2);
	}
	cd_do_change_current_icon (pIcon, pDock);
	
	// show main dock.
	myData.bIgnoreIconState = TRUE;
	cairo_dock_emit_enter_signal (CAIRO_CONTAINER (g_pMainDock));
	myData.bIgnoreIconState = FALSE;
	
	// give focus to main dock for inputs.
	myData.iPreviouslyActiveWindow = cairo_dock_get_active_xwindow ();
	
	///gtk_window_present (GTK_WINDOW (g_pMainDock->container.pWidget));
	gtk_window_present_with_time (GTK_WINDOW (g_pMainDock->container.pWidget), gdk_x11_get_server_time (g_pMainDock->container.pWidget->window));  // pour eviter la prevention du vol de focus.
	cairo_dock_freeze_docks (TRUE);
	
	// launch animation.
	cairo_dock_launch_animation (CAIRO_CONTAINER (g_pMainDock));
	
	myData.iSessionState = 2;
}

void cd_do_close_session (void)
{
	if (! cd_do_session_is_running ())  // session not running
		return;
	
	// no more keyboard input.
	cairo_dock_remove_notification_func (CAIRO_DOCK_KEY_PRESSED, (CairoDockNotificationFunc) cd_do_key_pressed, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_STOP_ICON, (CairoDockNotificationFunc) cd_do_check_icon_stopped, NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_WINDOW_ACTIVATED, (CairoDockNotificationFunc) cd_do_check_active_dock, NULL);
	
	g_string_free (myData.sCurrentText, TRUE);
	myData.sCurrentText = NULL;
	
	// give back focus.
	if (myData.iPreviouslyActiveWindow != 0)
	{
		// ne le faire que si on a encore le focus, sinon c'est que l'utilisateur a change lui-meme de fenetre...
		Window iActiveWindow = cairo_dock_get_active_xwindow ();
		if (myData.pCurrentDock && iActiveWindow == GDK_WINDOW_XID (myData.pCurrentDock->container.pWidget->window))
			cairo_dock_show_xwindow (myData.iPreviouslyActiveWindow);
		
		myData.iPreviouslyActiveWindow = 0;
	}
	
	// reset session state.
	if (myData.pCurrentIcon != NULL)
	{
		myData.bIgnoreIconState = TRUE;
		cairo_dock_stop_icon_animation (myData.pCurrentIcon);
		myData.bIgnoreIconState = FALSE;
		myData.pCurrentIcon = NULL;
	}
	
	if (myData.pCurrentDock != NULL)
	{
		cairo_dock_emit_leave_signal (CAIRO_CONTAINER (myData.pCurrentDock));
	}
	
	// launch closing animation.
	myData.iCloseTime = myConfig.iCloseDuration;
	cairo_dock_launch_animation (CAIRO_CONTAINER (g_pMainDock));
	cairo_dock_freeze_docks (FALSE);
	
	myData.iSessionState = 1;
}


void cd_do_exit_session (void)
{
	if (cd_do_session_is_off ())  // session already off
		return;
	
	cd_do_close_session ();
	
	myData.iCloseTime = 0;
	
	cd_do_change_current_icon (NULL, NULL);
	
	myData.iSessionState = 0;
}
