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
#include "applet-timer.h"
#include "applet-config.h"
#include "applet-theme.h"
#include "applet-calendar.h"
#include "applet-backend-default.h"
#include "applet-backend-ical.h"
#include "applet-notifications.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN (N_("clock"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_APPLET_ACCESSORY,
	N_("This applet displays time, date and a calandar.\n"
	"2 view are available : <b>numeric</b> and <b>analogic</b>.\n"
	" Analogic view is compatible with the Cairo-Clock's themes, and you can detach the applet to be a perfect clone of Cairo-Clock.\n"
	"It displays a <b>calendar</b> on left-click, which lets you <b>manage tasks</b>.\n"
	"It also supports alarms, and allows you to setup time and date.\n"
	"Left-click to show/hide the calendar, Middle-click to stop a notification,\n"
	"Double-click on a day to edit the tasks for this day."),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_ALLOW_EMPTY_TITLE
	pInterface->load_custom_widget = cd_clock_load_custom_widget;
	pInterface->save_custom_widget = cd_clock_save_custom_widget;
CD_APPLET_DEFINE_END

static gboolean _cd_check_new_minute (GldiModuleInstance *myApplet)
{
	myData.iSidUpdateClock = g_timeout_add_seconds (60,
		(GSourceFunc) cd_clock_update_with_time,
		(gpointer) myApplet); // this new g_timeout should start approximately at 00" (between 00" and 01" ; better than between [00-60])
	cd_clock_update_with_time (myApplet); // update the time right now and not after 60sec
	return FALSE;
}

static void _cd_launch_timer (GldiModuleInstance *myApplet)
{
	cd_clock_update_with_time (myApplet); // should update myData.currentTime

	if (! myConfig.bShowSeconds)  // can be interesting to show the correct minute... Start the new timer with a delay
	{
		guint iWaitSeconds = 60 - myData.currentTime.tm_sec;
		cd_debug ("Waiting for a new minute during %d sec", iWaitSeconds);

		myData.iSidUpdateClock = g_timeout_add_seconds (iWaitSeconds,
			(GSourceFunc) _cd_check_new_minute,
			(gpointer) myApplet);
	}
	else
		myData.iSidUpdateClock = g_timeout_add_seconds (1, (GSourceFunc) cd_clock_update_with_time, (gpointer) myApplet);
}

static void _relaunch_timer (GldiModuleInstance *myApplet)
{
	if (! myConfig.bShowSeconds) // not interesting if the hour is updated each second
	{
		g_source_remove (myData.iSidUpdateClock); // stop the timer
		myData.iSidUpdateClock = 0;
		_cd_launch_timer (myApplet); // relaunch the timer to be sync with the right second.
	}
}

static void _on_prepare_for_sleep (G_GNUC_UNUSED DBusGProxy *proxy_item, gboolean bSuspend, GldiModuleInstance *myApplet)
{
	cd_debug ("Refresh timer after resuming: login1 (%d)", bSuspend);
	if (! bSuspend) // bSuspend <=> false when resuming
		_relaunch_timer (myApplet);
}

static void _on_resuming (G_GNUC_UNUSED DBusGProxy *proxy_item, GldiModuleInstance *myApplet)
{
	cd_debug ("Refresh timer after resuming: UPower");
	_relaunch_timer (myApplet);
}

static gboolean s_bUsedLogind;

static void _cd_connect_to_resuming_signal (GldiModuleInstance *myApplet)
{
	if ((s_bUsedLogind = cairo_dock_dbus_detect_system_application ("org.freedesktop.login1")))
		myData.pProxyResuming = cairo_dock_create_new_system_proxy (
			"org.freedesktop.login1",
			"/org/freedesktop/login1",
			"org.freedesktop.login1.Manager");
	else if (cairo_dock_dbus_detect_system_application ("org.freedesktop.UPower"))
		myData.pProxyResuming = cairo_dock_create_new_system_proxy (
			"org.freedesktop.UPower",
			"/org/freedesktop/UPower",
			"org.freedesktop.UPower");

	if (myData.pProxyResuming == NULL) // no proxy found
	{
		cd_debug ("LoginD and UPower bus are not available, can't connect to 'resuming' signal");
		return;
	}

	if (s_bUsedLogind)
	{
		dbus_g_object_register_marshaller (
			g_cclosure_marshal_VOID__BOOLEAN,
			G_TYPE_BOOLEAN,
			G_TYPE_INVALID);

		dbus_g_proxy_add_signal (myData.pProxyResuming, "PrepareForSleep",
			G_TYPE_BOOLEAN, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal (myData.pProxyResuming, "PrepareForSleep",
			G_CALLBACK (_on_prepare_for_sleep), myApplet, NULL);
	}
	else // UPower
	{
		dbus_g_object_register_marshaller (
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE,
			G_TYPE_INVALID);

		dbus_g_proxy_add_signal (myData.pProxyResuming, "Resuming",
			G_TYPE_INVALID);
		dbus_g_proxy_connect_signal (myData.pProxyResuming, "Resuming",
			G_CALLBACK (_on_resuming), myApplet, NULL);
	}
}

static void _cd_disconnect_from_resuming_signal (GldiModuleInstance *myApplet)
{
	if (myData.pProxyResuming)
	{
		if (s_bUsedLogind)
			dbus_g_proxy_disconnect_signal (myData.pProxyResuming, "PrepareForSleep",
				G_CALLBACK (_on_prepare_for_sleep), myApplet);
		else
			dbus_g_proxy_disconnect_signal (myData.pProxyResuming, "Resuming",
				G_CALLBACK (_on_resuming), myApplet);

		g_object_unref (myData.pProxyResuming);
	}
}

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
	
	myData.cSystemLocation = g_strdup (g_getenv ("TZ"));
	myData.iLastCheckedMinute = -1;
	myData.iLastCheckedDay = -1;
	myData.iLastCheckedMonth = -1;
	myData.iLastCheckedYear = -1;
	myData.iTextLayout = myConfig.iPreferedTextLayout;

	myData.fDpi = gdk_screen_get_resolution (gdk_screen_get_default());
	
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
	cd_clock_register_backend_ical (myApplet);
	
	//\_______________ On liste les taches (apres avoir le temps courant).
	cd_clock_set_current_backend (myApplet);
	
	cd_clock_init_time (myApplet);
	cd_clock_list_tasks (myApplet);
	
	//\_______________ On lance le timer.
	_cd_launch_timer (myApplet);

	//\_______________ Connect to UPower's resuming signal (if available)
	_cd_connect_to_resuming_signal (myApplet);
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

	//\_______________ Disconnect from UPower's resuming signal
	_cd_disconnect_from_resuming_signal (myApplet);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	myData.iTextLayout = myConfig.iPreferedTextLayout;  // on recalcule l'orientation si elle est automatique.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ set a desklet view.
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
			CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
		}
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
		CDClockTaskBackend *pOldBackend = myData.pBackend;
		cd_clock_set_current_backend (myApplet);
		if (pOldBackend != myData.pBackend)
			cd_clock_list_tasks (myApplet);

		myData.fDpi = gdk_screen_get_resolution (gdk_screen_get_default());

		//\_______________ On relance le timer.
		myData.iLastCheckedMinute = -1;
		myData.iLastCheckedDay = -1;
		myData.iLastCheckedMonth = -1;
		myData.iLastCheckedYear = -1;
		_cd_launch_timer (myApplet);
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
