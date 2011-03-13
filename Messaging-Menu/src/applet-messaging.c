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

#include "applet-struct.h"
#include "messages-service-client.h"
#include "applet-messaging.h"

#define DEFAULT_ICON "indicator-messages"

  ///////////
 // PROXY //
///////////

/* Called everytime the attention changes in the service. */
static void
attention_changed_cb (DBusGProxy * proxy, gboolean dot, CairoDockModuleInstance *myApplet)
{
	//g_print ("%s (attention : %d)\n", __func__, dot);
	if (dot)
	{
		cd_indicator_set_icon (myData.pIndicator, "indicator-messages-new");
		
		if (myConfig.cAnimationName != NULL)
			CD_APPLET_DEMANDS_ATTENTION (myConfig.cAnimationName, 60);
	}
	else
	{
		cd_indicator_set_icon (myData.pIndicator, "indicator-messages");
		
		CD_APPLET_STOP_DEMANDING_ATTENTION;
	}
}

/* Change the icon to whether it should be visible or not */
static void
icon_changed_cb (DBusGProxy * proxy, gboolean hidden, CairoDockModuleInstance *myApplet)
{
	//g_print ("%s (hidden : %d)\n", __func__, hidden);
	if (hidden)
	{
		myIcon->fAlpha = .5;
	}
	else
	{
		myIcon->fAlpha = 1.;
	}
	CD_APPLET_REDRAW_MY_ICON;
}

/* Callback from getting the attention status from the service. */
static void
attention_cb (DBusGProxy * proxy, gboolean dot, GError * error, CairoDockModuleInstance *myApplet)
{
	if (error != NULL) {
		g_warning("Unable to get attention status: %s", error->message);
		g_error_free(error);
		return;
	}

	return attention_changed_cb(proxy, dot, myApplet);
}

/* Change from getting the icon visibility from the service */
static void
icon_cb (DBusGProxy * proxy, gboolean hidden, GError * error, CairoDockModuleInstance *myApplet)
{
	if (error != NULL) {
		g_warning("Unable to get icon visibility: %s", error->message);
		g_error_free(error);
		return;
	}

	return icon_changed_cb(proxy, hidden, myApplet);
}

void cd_messaging_on_connect (CairoDockModuleInstance *myApplet)
{
	DBusGProxy * pServiceProxy = myData.pIndicator->pServiceProxy;
	
	dbus_g_proxy_add_signal(myData.pIndicator->pServiceProxy, "AttentionChanged", G_TYPE_BOOLEAN, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pIndicator->pServiceProxy,
		"AttentionChanged",
		G_CALLBACK(attention_changed_cb),
		myApplet,
		NULL);
	dbus_g_proxy_add_signal(myData.pIndicator->pServiceProxy, "IconChanged", G_TYPE_BOOLEAN, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(myData.pIndicator->pServiceProxy,
		"IconChanged",
		G_CALLBACK(icon_changed_cb),
		myApplet,
		NULL);
}

void cd_messaging_on_disconnect (CairoDockModuleInstance *myApplet)
{
	//g_print ("disconnected\n");
	cd_indicator_set_icon (myData.pIndicator, DEFAULT_ICON);  // If we're disconnecting, go back to offline.
}

void cd_messaging_get_initial_values (CairoDockModuleInstance *myApplet)
{
	// query the service to display initial values.
	DBusGProxy * pServiceProxy = myData.pIndicator->pServiceProxy;
	
	org_ayatana_indicator_messages_service_attention_requested_async(myData.pIndicator->pServiceProxy,
		(org_ayatana_indicator_messages_service_attention_requested_reply)attention_cb,
		myApplet);
	
	org_ayatana_indicator_messages_service_icon_shown_async(myData.pIndicator->pServiceProxy,
		(org_ayatana_indicator_messages_service_icon_shown_reply)icon_cb,
		myApplet);
}
