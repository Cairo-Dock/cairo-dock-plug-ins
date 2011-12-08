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
#include "applet-struct.h"
#include "applet-sound.h"

// we can use icons designed for Unity, or more common icons that we are likely to find in icons themes.
/// TODO: decide which one to use !
/// TODO-bis: include in our 'data' dir a set of these icons, to be used as fallback by 'cd_indicator_set_icon()'

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
}	

  ///////////
 // PROXY //
///////////

static void _update_sound_state_icon (gint iNewState, CairoDockModuleInstance *myApplet);

static gboolean _get_volume (gint iNewState)
{
	_update_sound_state_icon (iNewState, myApplet);
	return FALSE;
}

static void _update_sound_state_icon (gint iNewState, CairoDockModuleInstance *myApplet)
{
	g_return_if_fail (iNewState >= 0 && iNewState < NB_STATES);
	gboolean bWasMute = ! (iNewState == MUTED || iNewState == UNAVAILABLE || iNewState == BLOCKED);
	myData.iCurrentState = iNewState;
	
	switch (myConfig.iVolumeEffect)
	{
		// if we rely on the state to draw the icon, do it now
		case VOLUME_EFFECT_ICONS:
		{
			///const gchar *cIconName = _get_icon_from_state_unity (iNewState);
			const gchar *cIconName = _get_icon_from_state (iNewState);
			cd_indicator_set_icon (myData.pIndicator, cIconName);
		}
		break;

		// if we rely on the volume to draw the icon, juste handle the 'mute' case.
		case VOLUME_EFFECT_BAR:
			if (iNewState == MUTED)
			{
				CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cMuteIcon, MY_APPLET_SHARE_DATA_DIR"/mute.svg");
				CD_APPLET_REDRAW_MY_ICON;
			}
			else if (bWasMute)
			{
				double fVolume = 0;
				if (myData.volume_widget)
					fVolume = volume_widget_get_current_volume (myData.volume_widget);
				if (fVolume == 0.0 && iNewState != ZERO_LEVEL) // strange... should not be null, volume_widget_get_current_volume gives a wrong value at startup
					g_timeout_add_seconds (1, (GSourceFunc) _get_volume, GINT_TO_POINTER (iNewState));
				CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_BAR (myData.pSurface, fVolume * .01);
			}
		case VOLUME_EFFECT_GAUGE:
			if (iNewState == MUTED)
			{
				double fPercent = CAIRO_DATA_RENDERER_UNDEF_VALUE;
				CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fPercent);
			}
			else if (bWasMute)
			{
				double fVolume = 0, fPercent;
				if (myData.volume_widget)
					fVolume = volume_widget_get_current_volume (myData.volume_widget);
				if (fVolume == 0.0 && iNewState != ZERO_LEVEL) // strange... should not be null, volume_widget_get_current_volume gives a wrong value at startup
				{
					g_timeout_add_seconds (1, (GSourceFunc) _get_volume, GINT_TO_POINTER (iNewState));
					fPercent = CAIRO_DATA_RENDERER_UNDEF_VALUE;
				}
				else
					fPercent = (double) fVolume / 100.;
				CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fPercent);
			}
		break;

		default:
		break;
	}
}

static void
on_sound_state_updated (DBusGProxy * proxy, gint iNewState, CairoDockModuleInstance *myApplet)
{
	g_print ("++++ %s (iNewState : %d)\n", __func__, iNewState);
	CD_APPLET_ENTER;
	if (iNewState != myData.iCurrentState)
	{
		_update_sound_state_icon (iNewState, myApplet);
	}
	CD_APPLET_LEAVE();
}

void cd_sound_on_connect (CairoDockModuleInstance *myApplet)
{
	DBusGProxy * pServiceProxy = myData.pIndicator->pServiceProxy;
	
	dbus_g_proxy_add_signal(myData.pIndicator->pServiceProxy, INDICATOR_SOUND_SIGNAL_STATE_UPDATE, G_TYPE_INT, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pIndicator->pServiceProxy,
		INDICATOR_SOUND_SIGNAL_STATE_UPDATE,
		G_CALLBACK(on_sound_state_updated),
		myApplet,
		NULL);
}

void cd_sound_on_disconnect (CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	g_print ("++++ %s \n", __func__);
	// go to the unavailable mode to signal a possible problem (or at least that the applet is not usable any more)
	_update_sound_state_icon (UNAVAILABLE, myApplet);
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
	_update_sound_state_icon (iCurrentState, myApplet);
	CD_APPLET_LEAVE();
}
void cd_sound_get_initial_values (CairoDockModuleInstance *myApplet)
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
	if (!myData.volume_widget)
		return;
	double fVolume = volume_widget_get_current_volume (myData.volume_widget);
	CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("%s: %d%%", D_("Volume"), (int) fVolume);
	
	switch (myConfig.iVolumeEffect)
	{
		// if we rely on the state to draw the icon, do it now
		case VOLUME_EFFECT_ICONS:
		{
			///const gchar *cIconName = _get_icon_from_state_unity (iNewState);
			const gchar *cIconName = _get_icon_from_state (myData.iCurrentState);
			cd_indicator_set_icon (myData.pIndicator, cIconName);
		}
		break;

		// if we rely on the volume to draw the icon, juste handle the 'mute' case.
		case VOLUME_EFFECT_BAR:
			if (myData.iCurrentState == MUTED)
			{
				CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cMuteIcon, MY_APPLET_SHARE_DATA_DIR"/mute.svg");
				CD_APPLET_REDRAW_MY_ICON;
			}
			else
			{
				CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_BAR (myData.pSurface, fVolume * .01);
			}
		case VOLUME_EFFECT_GAUGE:
			if (myData.iCurrentState == MUTED)
			{
				double fPercent = CAIRO_DATA_RENDERER_UNDEF_VALUE;
				CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fPercent);
			}
			else
			{
				double fPercent = (double) fVolume / 100.;
				CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fPercent);
			}
		break;

		default:
		break;
	}
}


void cd_sound_load_surface (CairoDockModuleInstance *myApplet)
{
	if (myData.pSurface != NULL)
	{
		cairo_surface_destroy (myData.pSurface);
		myData.pSurface = NULL;
	}
	
	if (myConfig.cDefaultIcon != NULL)
	{
		gchar *cImagePath = cairo_dock_search_icon_s_path (myConfig.cDefaultIcon);
		if (cImagePath == NULL)
			cImagePath = cairo_dock_search_image_s_path (myConfig.cDefaultIcon);
		if (cImagePath != NULL)
		{
			myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
			g_free (cImagePath);
		}
	}
	
	if (myData.pSurface == NULL)
	{
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (MY_APPLET_SHARE_DATA_DIR"/default.svg");
	}
}
