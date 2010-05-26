/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* based on indicator-me.c written by :
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

#include <libindicator/indicator.h>
#include <libindicator/indicator-object.h>
#include <libindicator/indicator-service-manager.h>

#include "applet-struct.h"
#include "me-service-client.h"
#include "applet-me.h"

#define INDICATOR_ME_DBUS_OBJECT "/org/ayatana/indicator/me/menu"
#define INDICATOR_ME_SERVICE_DBUS_OBJECT "/org/ayatana/indicator/me/service"
#define INDICATOR_ME_SERVICE_DBUS_INTERFACE "org.ayatana.indicator.me.service"
#define INDICATOR_ME_DBUS_NAME  "org.ayatana.indicator.me"
#define INDICATOR_ME_DBUS_VERSION  1
#define DEFAULT_ICON "user-offline"


  ///////////
 // PROXY //
///////////

static void
username_cb (DBusGProxy * proxy, const char * username, GError *error, CairoDockModuleInstance *myApplet)
{
	g_print (" + new username: '%s'\n", username);
	CD_APPLET_SET_NAME_FOR_MY_ICON (username);  // username peut etre NULL ou vide, c'est pas genant.
}

static void
username_changed (DBusGProxy * proxy, gchar * username, CairoDockModuleInstance *myApplet)
{
	g_print ("Changing username: '%s'\n", username);
	
	return username_cb(proxy, username, NULL, myApplet);
}

static void
status_icon_cb (DBusGProxy * proxy, const char * icons, GError *error, CairoDockModuleInstance *myApplet)
{
	g_return_if_fail(icons != NULL);
	g_return_if_fail(icons[0] != '\0');
	g_print (" + new icon: '%s'\n", icons);
	
	cd_indicator_set_icon (myData.pIndicator, icons);
	CD_APPLET_REDRAW_MY_ICON;
	
	return;
}

static void
status_icon_changed (DBusGProxy * proxy, gchar * icon, CairoDockModuleInstance *myApplet)
{
	g_print ("Changing status icon: '%s'\n", icon);
	
	return status_icon_cb(proxy, icon, NULL, myApplet);
}

void cd_me_on_connect (CairoDockModuleInstance *myApplet)
{
	DBusGProxy * pServiceProxy = myData.pIndicator->pServiceProxy;
	
	dbus_g_proxy_add_signal (pServiceProxy, "StatusIconsChanged", G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal (pServiceProxy, "StatusIconsChanged", G_CALLBACK(status_icon_changed), myApplet, NULL);
	
	dbus_g_proxy_add_signal (pServiceProxy, "UserChanged", G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal (pServiceProxy, "UserChanged", G_CALLBACK(username_changed), myApplet, NULL);
}

void cd_me_on_disconnect (CairoDockModuleInstance *myApplet)
{
	g_print ("disconnected\n");
	status_icon_cb (NULL, DEFAULT_ICON, NULL, myApplet);  // If we're disconnecting, go back to offline.
}

void cd_me_get_initial_values (CairoDockModuleInstance *myApplet)
{
	// query the service to display initial values.
	DBusGProxy * pServiceProxy = myData.pIndicator->pServiceProxy;
	
	org_ayatana_indicator_me_service_status_icons_async (pServiceProxy,
		(org_ayatana_indicator_me_service_status_icons_reply)status_icon_cb,
		myApplet);
	
	org_ayatana_indicator_me_service_pretty_user_name_async (pServiceProxy,
		(org_ayatana_indicator_me_service_status_icons_reply)username_cb,
		myApplet);
}
