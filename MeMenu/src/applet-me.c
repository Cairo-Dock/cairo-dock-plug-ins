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

#include <libdbusmenu-glib/menuitem.h>
#include <libdbusmenu-gtk/menu.h>

//#include <dbus/dbus-glib.h>
//#include <dbus/dbus-glib-bindings.h>

#include <libindicator/indicator.h>
#include <libindicator/indicator-object.h>
#include <libindicator/indicator-service-manager.h>
#include <libindicator/indicator-image-helper.h>
#include <libido/idoentrymenuitem.h>

#include "applet-struct.h"
#include "about-me-menu-item.h"
#include "me-service-client.h"
#include "applet-me.h"

#define INDICATOR_ME_DBUS_OBJECT "/org/ayatana/indicator/me/menu"
#define INDICATOR_ME_SERVICE_DBUS_OBJECT "/org/ayatana/indicator/me/service"
#define INDICATOR_ME_SERVICE_DBUS_INTERFACE "org.ayatana.indicator.me.service"
#define INDICATOR_ME_DBUS_NAME  "org.ayatana.indicator.me"
#define INDICATOR_ME_DBUS_VERSION  1
#define DBUSMENU_ENTRY_MENUITEM_PROP_TEXT      "text"
#define DBUSMENU_ENTRY_MENUITEM_TYPE           "x-canonical-entry-item"
#define DBUSMENU_ABOUT_ME_MENUITEM_PROP_ICON      "icon"
#define DBUSMENU_ABOUT_ME_MENUITEM_PROP_NAME      "name"
#define DBUSMENU_ABOUT_ME_MENUITEM_TYPE           "x-canonical-about-me-item"
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
	
	gchar *cIconPath = cairo_dock_search_icon_s_path (icons);
	if (cIconPath == NULL)
		cIconPath = g_strdup_printf ("%s/%s.svg", MY_APPLET_SHARE_DATA_DIR, icons);
	
	CD_APPLET_SET_IMAGE_ON_MY_ICON (cIconPath);
	CD_APPLET_REDRAW_MY_ICON;
	
	g_free (cIconPath);
	return;
}

static void
status_icon_changed (DBusGProxy * proxy, gchar * icon, CairoDockModuleInstance *myApplet)
{
	g_print ("Changing status icon: '%s'\n", icon);
	
	return status_icon_cb(proxy, icon, NULL, myApplet);
}

static void
connection_changed (IndicatorServiceManager * sm, gboolean connected, CairoDockModuleInstance *myApplet)
{
	if (connected)
	{
		// connect to the service.
		if (myData.status_proxy == NULL)
		{
			GError * error = NULL;
			DBusGConnection * sbus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
			myData.status_proxy = dbus_g_proxy_new_for_name_owner(sbus,
				INDICATOR_ME_DBUS_NAME,
				INDICATOR_ME_SERVICE_DBUS_OBJECT,
				INDICATOR_ME_SERVICE_DBUS_INTERFACE,
				&error);
			if (error != NULL)
			{
				g_warning("Unable to get status proxy: %s", error->message);
				g_error_free(error);
			}
			if (myData.status_proxy == NULL)
				return;
			
			dbus_g_proxy_add_signal(myData.status_proxy, "StatusIconsChanged", G_TYPE_STRING, G_TYPE_INVALID);
			dbus_g_proxy_connect_signal(myData.status_proxy, "StatusIconsChanged", G_CALLBACK(status_icon_changed), myApplet, NULL);
			
			dbus_g_proxy_add_signal(myData.status_proxy, "UserChanged", G_TYPE_STRING, G_TYPE_INVALID);
			dbus_g_proxy_connect_signal(myData.status_proxy, "UserChanged", G_CALLBACK(username_changed), myApplet, NULL);  /// username_cb
		}
		
		// query the service to display initial values.
		org_ayatana_indicator_me_service_status_icons_async(myData.status_proxy, (org_ayatana_indicator_me_service_status_icons_reply)status_icon_cb, myApplet);
		
		org_ayatana_indicator_me_service_pretty_user_name_async(myData.status_proxy, (org_ayatana_indicator_me_service_status_icons_reply)username_cb, myApplet);
	}
	else  // If we're disconnecting, go back to offline.
	{
		status_icon_cb (NULL, DEFAULT_ICON, NULL, myApplet);
	}

	return;
}

void cd_me_connect_to_service (CairoDockModuleInstance *myApplet)
{
	myData.service = indicator_service_manager_new_version (INDICATOR_ME_DBUS_NAME, INDICATOR_ME_DBUS_VERSION);
	g_signal_connect (G_OBJECT(myData.service), INDICATOR_SERVICE_MANAGER_SIGNAL_CONNECTION_CHANGE, G_CALLBACK(connection_changed), myApplet);  // on sera appele une fois la connexion etablie.
}


void cd_me_disconnect_from_service (CairoDockModuleInstance *myApplet)
{
	if (myData.service)
		g_object_unref (myData.service);
	if (myData.status_proxy)
		g_object_unref (myData.status_proxy);
}

  ////////////////
 // MENU ENTRY //
////////////////

#if (DBUSMENU_MAJOR > 0 || DBUSMENU_MINOR >= 2)
static void
entry_prop_change_cb (DbusmenuMenuitem *mi, gchar *prop, GValue *value, GtkEntry *entry)
{
	if (g_strcmp0 (prop, DBUSMENU_ENTRY_MENUITEM_PROP_TEXT) == 0) {
		gtk_entry_set_text (entry, g_value_get_string (value));
	}
}

static void
entry_activate_cb (GtkEntry *entry, DbusmenuMenuitem *mi)
{
	g_return_if_fail (GTK_IS_ENTRY (entry));
	g_return_if_fail (DBUSMENU_IS_MENUITEM (mi));

	GValue value = G_VALUE_INIT;
	g_value_init (&value, G_TYPE_STRING);
	g_value_set_static_string (&value, gtk_entry_get_text (entry));

	g_print ("user typed: %s\n", g_value_get_string (&value));
	dbusmenu_menuitem_handle_event (mi, "send", &value, gtk_get_current_event_time());
}

static gboolean
menu_visibility_changed (GtkWidget          *widget,
                         IdoEntryMenuItem   *menuitem)
{
	if (GTK_IS_WIDGET (widget) && IDO_IS_ENTRY_MENU_ITEM (menuitem))
		gtk_menu_shell_select_item (GTK_MENU_SHELL (widget), GTK_WIDGET (menuitem));
	
	return FALSE;
}

static void
entry_parent_changed (GtkWidget *widget,
                      gpointer   user_data)
{
	GtkWidget *parent = gtk_widget_get_parent (widget);
	
	if (parent && GTK_IS_MENU_SHELL (parent))
	{
		g_signal_connect (parent,
			"map", G_CALLBACK (menu_visibility_changed),
			widget);  // des que le menu devient visible, on selectionne l'entry pour pouvoir ecrire dedans direct.
	}
}

static gboolean
new_entry_item (DbusmenuMenuitem * newitem,
                DbusmenuMenuitem * parent,
                DbusmenuClient   * client)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);
	/* Note: not checking parent, it's reasonable for it to be NULL */

	IdoEntryMenuItem *ido = IDO_ENTRY_MENU_ITEM (ido_entry_menu_item_new ());
	GtkEntry *entry = GTK_ENTRY(ido_entry_menu_item_get_entry (ido));
	
	const gchar *cPropText = dbusmenu_menuitem_property_get (newitem, DBUSMENU_ENTRY_MENUITEM_PROP_TEXT);
	if (cPropText != NULL)
		gtk_entry_set_text (entry, cPropText);
	gtk_entry_set_width_chars (entry, 23); /* set some nice aspect ratio for the menu */
	gtk_entry_set_max_length (entry, 140); /* enforce current gwibber limit */
	
	g_signal_connect (ido,
		"notify::parent", G_CALLBACK (entry_parent_changed),
		NULL);

	dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, GTK_MENU_ITEM(ido), parent);
	/* disconnect the activate signal that newitem_base connected with the wrong
	   widget, ie menuitem, and re-connect it with the /entry/ instead */
	gulong signal_id = g_signal_handler_find (GTK_MENU_ITEM (ido), G_SIGNAL_MATCH_DATA, 0, 0, 0, 0, parent);
	if (signal_id > 0)
		g_signal_handler_disconnect(GTK_MENU_ITEM (ido), signal_id);

	g_signal_connect (DBUSMENU_MENUITEM (newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK (entry_prop_change_cb), entry);
	g_signal_connect (GTK_ENTRY (entry), "activate", G_CALLBACK (entry_activate_cb), newitem);

	return TRUE;
}
#endif

  //////////////
 // ABOUT ME //
//////////////

/* Whenever we have a property change on a DbusmenuMenuitem
   we need to be responsive to that. */
static void
about_me_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, GValue * value, AboutMeMenuItem *item)
{
  g_return_if_fail (ABOUT_IS_ME_MENU_ITEM (item));

	if (!g_strcmp0(prop, DBUSMENU_ABOUT_ME_MENUITEM_PROP_ICON)) {
    /* reload the avatar icon */
    about_me_menu_item_load_avatar (item, g_value_get_string(value));
  } else if (!g_strcmp0(prop, DBUSMENU_MENUITEM_PROP_VISIBLE)) {
    /* normal, ignore */
	} else {
		g_warning("Indicator Item property '%s' unknown", prop);
	}

	return;
}

static gboolean
new_about_me_item (DbusmenuMenuitem * newitem,
                   DbusmenuMenuitem * parent,
                   DbusmenuClient * client)
{
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);
	/* Note: not checking parent, it's reasonable for it to be NULL */

	const gchar *name = dbusmenu_menuitem_property_get (newitem, DBUSMENU_ABOUT_ME_MENUITEM_PROP_NAME);
	AboutMeMenuItem *about = ABOUT_ME_MENU_ITEM (about_me_menu_item_new (name));
	if (about != NULL) {
	dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, GTK_MENU_ITEM(about), parent);
	const gchar *avatar = dbusmenu_menuitem_property_get (newitem, DBUSMENU_ABOUT_ME_MENUITEM_PROP_ICON);
	about_me_menu_item_load_avatar (about, avatar);
	g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(about_me_prop_change_cb), about);
	}
	
	return TRUE;
}


  //////////
 // MENU //
//////////

GtkMenu *cd_me_get_menu (CairoDockModuleInstance *myApplet)
{
	if (myData.pMenu == NULL)
	{
		myData.pMenu = dbusmenu_gtkmenu_new (INDICATOR_ME_DBUS_NAME, INDICATOR_ME_DBUS_OBJECT);
		DbusmenuGtkClient * client = dbusmenu_gtkmenu_get_client (myData.pMenu);
		
		#if (DBUSMENU_MAJOR > 0 || DBUSMENU_MINOR >= 2)
		dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client), DBUSMENU_ENTRY_MENUITEM_TYPE, new_entry_item);
		#endif
		dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client), DBUSMENU_ABOUT_ME_MENUITEM_TYPE, new_about_me_item);
	}
	return GTK_MENU (myData.pMenu);
}

