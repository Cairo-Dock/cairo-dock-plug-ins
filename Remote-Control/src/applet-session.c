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
#include <gdk/gdkx.h>  // gldi_container_present

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-icon-finder.h"
#include "applet-session.h"


void cd_do_numberize_icons (CairoDock *pDock)
{
	int n = 0;
	int iWidth, iHeight;
	gchar number[2];
	number[1] = '\0';
	
	GldiTextDescription *pLabelDesc = gldi_text_description_duplicate (&myIconsParam.quickInfoTextDescription);
	int iSize = pLabelDesc->iSize;
	cairo_surface_t *pSurface;
	Icon *pIcon;
	GList *ic;
	for (ic = pDock->icons; ic != NULL && n < 10; ic = ic->next)
	{
		pIcon = ic->data;
		if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			if (n == 9)  // 10th icon is "0"
				number[0] = '0';
			else
				number[0] = '1' + n;  // the first icon will take the "1" number.
			
			pLabelDesc->iSize *= cairo_dock_get_icon_max_scale (pIcon);
			pSurface = cairo_dock_create_surface_from_text (number, pLabelDesc, &iWidth, &iHeight);
			pLabelDesc->iSize = iSize;
			CairoOverlay *pOverlay = cairo_dock_add_overlay_from_surface (pIcon, pSurface, iWidth, iHeight, CAIRO_OVERLAY_UPPER_RIGHT, myApplet);
			if (pOverlay)
				cairo_dock_set_overlay_scale (pOverlay, 0);  // don't scale with the icon.
			n ++;
		}
	}
	gldi_text_description_free (pLabelDesc);
}

void cd_do_remove_icons_number (CairoDock *pDock)
{
	Icon *pIcon;
	GList *ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)  // scan all the icons, in case a new icon has appeared amongst the first 10 icons during this session.
	{
		pIcon = ic->data;
		if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			cairo_dock_remove_overlay_at_position (pIcon, CAIRO_OVERLAY_UPPER_RIGHT, myApplet);
		}
	}
}


void cd_do_open_session (void)
{
	if (cd_do_session_is_running ())  // session already running, noting to do
		return;
	
	if (! cd_do_session_is_off ())  // session not yet closed, close it first to reset the state.
		cd_do_exit_session ();
	
	// wait for keyboard input.
	gldi_object_register_notification (&myContainerObjectMgr,
		NOTIFICATION_KEY_PRESSED, (GldiNotificationFunc) cd_do_key_pressed, GLDI_RUN_AFTER, NULL);
	gldi_object_register_notification (&myIconObjectMgr,
		NOTIFICATION_DESTROY, (GldiNotificationFunc) cd_do_check_icon_destroyed, GLDI_RUN_AFTER, NULL);
	gldi_object_register_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_ACTIVATED, (GldiNotificationFunc) cd_do_check_active_dock, GLDI_RUN_AFTER, NULL);
	
	myData.sCurrentText = g_string_sized_new (20);
	
	// set initial position (dock, icon).
	myData.pCurrentDock = NULL;
	myData.pCurrentIcon =  NULL;
	
	CairoDock *pDock = gldi_dock_get (myConfig.cDockName);
	if (pDock == NULL)
		pDock = g_pMainDock;
	Icon *pIcon = NULL;
	int n = g_list_length (pDock->icons);
	if (n > 0)
	{
		pIcon =  g_list_nth_data (pDock->icons, (n-1) / 2);
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon) && n > 1)
			pIcon = g_list_nth_data (pDock->icons, (n+1) / 2);
	}
	cd_do_change_current_icon (pIcon, pDock);
	
	// show the number of the 10 first icons
	cd_do_numberize_icons (pDock);
	
	// show the dock.
	myData.bIgnoreIconState = TRUE;
	gldi_dock_enter_synthetic (pDock);
	myData.bIgnoreIconState = FALSE;
	
	// give it focus for inputs.
	myData.pPreviouslyActiveWindow = gldi_windows_get_active ();
	gldi_container_present (CAIRO_CONTAINER (pDock));
	cairo_dock_freeze_docks (TRUE);
	
	// launch the animation.
	myData.iPromptAnimationCount = 0;
	if (myData.pArrowImage == NULL)
	{
		myData.pArrowImage = cairo_dock_create_image_buffer (MY_APPLET_SHARE_DATA_DIR"/arrows.svg",
			pDock->iActiveHeight,
			pDock->iActiveHeight,
			CAIRO_DOCK_KEEP_RATIO);
	}
	cairo_dock_launch_animation (CAIRO_CONTAINER (pDock));
	
	myData.iSessionState = CD_SESSION_RUNNING;
}

void cd_do_close_session (void)
{
	if (! cd_do_session_is_running ())  // session not running
		return;
	
	// no more keyboard input.
	gldi_object_remove_notification (&myContainerObjectMgr,
		NOTIFICATION_KEY_PRESSED,
		(GldiNotificationFunc) cd_do_key_pressed, NULL);
	gldi_object_remove_notification (&myIconObjectMgr,
		NOTIFICATION_DESTROY,
		(GldiNotificationFunc) cd_do_check_icon_destroyed, NULL);
	gldi_object_remove_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_ACTIVATED,
		(GldiNotificationFunc) cd_do_check_active_dock, NULL);
	
	g_string_free (myData.sCurrentText, TRUE);
	myData.sCurrentText = NULL;
	
	// reset session state.
	if (myData.pCurrentIcon != NULL)
	{
		myData.bIgnoreIconState = TRUE;
		gldi_icon_stop_animation (myData.pCurrentIcon);
		myData.bIgnoreIconState = FALSE;
		myData.pCurrentIcon = NULL;
	}
	
	myData.pPreviouslyActiveWindow = NULL;
	
	if (myData.pCurrentDock != NULL)
	{
		gldi_dock_leave_synthetic (myData.pCurrentDock);
		
		cd_do_remove_icons_number (myData.pCurrentDock);
		
		// launch closing animation.
		myData.iCloseTime = myConfig.iCloseDuration;
		cairo_dock_launch_animation (CAIRO_CONTAINER (myData.pCurrentDock));
	}
	
	cairo_dock_freeze_docks (FALSE);
	
	myData.iSessionState = CD_SESSION_CLOSING;
}


void cd_do_exit_session (void)
{
	if (cd_do_session_is_off ())  // session already off
		return;
	
	cd_do_close_session ();
	
	myData.iCloseTime = 0;
	
	cd_do_change_current_icon (NULL, NULL);
	
	myData.iSessionState = CD_SESSION_NONE;
}
