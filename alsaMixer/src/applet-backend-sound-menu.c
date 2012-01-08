/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* based on indicator-messages.c written by :
*  Ted Gould <ted@canonical.com>
*  Cody Russell <cody.russell@canonical.com>
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

#if (INDICATOR_OLD_NAMES == 0)
#include "dbus-shared-names.h"
#else
#include "dbus-shared-names-old.h"
#endif

#include "common-defs.h"
#include "volume-widget.h"
#include "mute-widget.h"
#include "applet-menu.h"
#include "applet-backend-alsamixer.h"  // cd_mixer_init_alsa (fallback alsa backend)
#include "applet-struct.h"
#include "applet-generic.h"
#include "applet-draw.h"
#include "applet-backend-sound-menu.h"

/*
// we can use icons designed for Unity, or more common icons that we are likely to find in icons themes.
static const gchar *_get_icon_from_state_unity (gint iState)
{
	switch (iState)
	{
		case MUTED: return "audio-volume-muted-panel";
		case ZERO_LEVEL: return "audio-volume-low-zero-panel";
		case LOW_LEVEL: return "audio-volume-low-panel";
		case MEDIUM_LEVEL: return "audio-volume-medium-panel";
		case HIGH_LEVEL: return "audio-volume-high-panel";
		case BLOCKED: return "audio-volume-muted-blocking-panel";
		default: return "audio-output-none-panel";
	}
}
static const gchar *_get_icon_from_state (gint iState)
{
	switch (iState)
	{
		case ZERO_LEVEL: return "audio-volume-off";
		case LOW_LEVEL: return "audio-volume-low";
		case MEDIUM_LEVEL: return "audio-volume-medium";
		case HIGH_LEVEL: return "audio-volume-high";
		default: return "audio-volume-muted";
	}
}*/

  ///////////
 // PROXY //
///////////

static void
on_sound_state_updated (DBusGProxy * proxy, gint iNewState, CairoDockModuleInstance *myApplet)
{
	g_print ("++++ %s (iNewState : %d)\n", __func__, iNewState);
	CD_APPLET_ENTER;
	if (iNewState != myData.iCurrentState)
	{
		gboolean bIsMute = (iNewState == MUTED
		|| iNewState == UNAVAILABLE
		|| iNewState == BLOCKED);
		if (myData.bIsMute != bIsMute)
		{
			myData.bIsMute = bIsMute;
			cd_update_icon ();
		}
	}
	CD_APPLET_LEAVE();
}

static int _get_volume (void)
{
	if (myData.volume_widget)
		return (int)volume_widget_get_current_volume (myData.volume_widget);
	else
		return 0;
}

static void _set_volume (int iVolume)
{
	if (myData.volume_widget)
		volume_widget_update (VOLUME_WIDGET(myData.volume_widget), (gdouble)iVolume, "scroll updates");
}

static void _toggle_mute (void)
{
	if (myData.mute_widget)
		mute_widget_toggle (MUTE_WIDGET (myData.mute_widget));
	//~ /// TODO: is there any way to send a "mute" to the service ?...
	//~ static double s_fVolume=0;
	//~ if (myData.volume_widget)
	//~ {
		//~ double fVolume = _get_volume ();
		//~ if (fVolume == 0)  // currently mute, set the previous value
			//~ _set_volume (s_fVolume);
		//~ else  // not mute -> set 0
			//~ _set_volume (0.);
		//~ s_fVolume = fVolume;
	//~ }
}

static void _show_menu (void)
{
	cd_indicator_show_menu (myData.pIndicator);
}

static void _stop (void)
{
	cd_indicator_destroy (myData.pIndicator);
}

static void cd_sound_on_connect (CairoDockModuleInstance *myApplet)
{
	g_print ("%s ()\n", __func__);
	// the sound service is up and running, stop the alsa mixer if ever we initialized it before.
	mixer_stop ();
	
	// and set the interface
	myData.ctl.get_volume = _get_volume;
	myData.ctl.set_volume = _set_volume;
	myData.ctl.toggle_mute = _toggle_mute;
	myData.ctl.show_hide = _show_menu;
	myData.ctl.stop = _stop;
	myData.ctl.reload = NULL;
	
	// connect to the service signals.
	DBusGProxy * pServiceProxy = myData.pIndicator->pServiceProxy;
	
	dbus_g_proxy_add_signal(myData.pIndicator->pServiceProxy, INDICATOR_SOUND_SIGNAL_STATE_UPDATE, G_TYPE_INT, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pIndicator->pServiceProxy,
		INDICATOR_SOUND_SIGNAL_STATE_UPDATE,
		G_CALLBACK(on_sound_state_updated),
		myApplet,
		NULL);
}

static void cd_sound_on_disconnect (CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	g_print ("++++ %s \n", __func__);
	
	if (myData.ctl.get_volume == _get_volume)  // the backend was set, unset it
	{
		memset (&myData.ctl, 0, sizeof (CDSoundCtl));
		myData.volume_widget = NULL;
		myData.transport_widgets_list = NULL;
		myData.voip_widget = NULL;
		myData.mute_widget = NULL;
	}
	
	// no (more) sound service, now rely on alsa.
	cd_mixer_init_alsa ();
	
	CD_APPLET_LEAVE();
}

static void _on_got_sound_state (DBusGProxy *proxy, DBusGProxyCall *call_id, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	int iCurrentState = 0;
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		NULL,
		G_TYPE_INT,
		&iCurrentState,
		G_TYPE_INVALID);
	g_print ("-> got sound state: %d\n", iCurrentState);
	
	// update the icon.
	myData.iCurrentState = iCurrentState;
	myData.bIsMute = (iCurrentState == MUTED
	|| iCurrentState == UNAVAILABLE
	|| iCurrentState == BLOCKED);
	myData.iCurrentVolume = _get_volume ();
	
	cd_update_icon ();
	CD_APPLET_LEAVE();
}
static void cd_sound_get_initial_values (CairoDockModuleInstance *myApplet)
{
	// query the service to display initial values.
	DBusGProxy * pServiceProxy = myData.pIndicator->pServiceProxy;

	DBusGProxyCall *pCall = dbus_g_proxy_begin_call (myData.pIndicator->pServiceProxy, "GetSoundState",
		(DBusGProxyCallNotify)_on_got_sound_state,
		myApplet,
		(GDestroyNotify) NULL,
		G_TYPE_INVALID);
}



void update_accessible_desc (CairoDockModuleInstance *myApplet)
{
	g_print ("%s (%p)\n", __func__, myData.volume_widget);
	if (!myData.volume_widget)
		return;
	
	myData.iCurrentVolume = volume_widget_get_current_volume (myData.volume_widget);
	cd_update_icon ();
}

void cd_mixer_connect_to_sound_service (void)
{
	myData.pIndicator = cd_indicator_new (myApplet,
		INDICATOR_SOUND_DBUS_NAME,
		INDICATOR_SOUND_SERVICE_DBUS_OBJECT_PATH,
		INDICATOR_SOUND_DBUS_INTERFACE,
		INDICATOR_SOUND_MENU_DBUS_OBJECT_PATH,
		0);	
	myData.pIndicator->on_connect 			= cd_sound_on_connect;
	myData.pIndicator->on_disconnect 		= cd_sound_on_disconnect;
	myData.pIndicator->get_initial_values 	= cd_sound_get_initial_values;
	myData.pIndicator->add_menu_handler 	= cd_sound_add_menu_handler;
}
