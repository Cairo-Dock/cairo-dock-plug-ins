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

#include "dbus-shared-names.h"
#include "applet-struct.h"
#include "about-me-menu-item.h"
#include "applet-menu.h"

#if (GTK_MAJOR_VERSION < 3) || defined (DBUSMENU_GTK3_NEW)
#include <libdbusmenu-gtk/menuitem.h>
#include <libdbusmenu-gtk/menu.h>
#else
#include <libdbusmenu-gtk3/menuitem.h>
#include <libdbusmenu-gtk3/menu.h>
#endif
#include <libido/idoentrymenuitem.h>

#if (DBUSMENU_MAJOR > 0 || DBUSMENU_MINOR >= 2)
static void
#if (INDICATOR_OLD_NAMES == 0)
entry_prop_change_cb (DbusmenuMenuitem *mi, gchar *prop, GVariant *value, GtkEntry *entry)
#else
entry_prop_change_cb (DbusmenuMenuitem *mi, gchar *prop, GValue *value, GtkEntry *entry)
#endif
{
	if (g_strcmp0 (prop, DBUSMENU_ENTRY_MENUITEM_PROP_TEXT) == 0) {
#if (INDICATOR_OLD_NAMES == 0)
		gtk_entry_set_text (entry, g_variant_get_string (value, NULL));

#else
		gtk_entry_set_text (entry, g_value_get_string (value));
#endif
	}
}

static void
entry_activate_cb (GtkEntry *entry, DbusmenuMenuitem *mi)
{
	g_return_if_fail (GTK_IS_ENTRY (entry));
	g_return_if_fail (DBUSMENU_IS_MENUITEM (mi));

#if (INDICATOR_OLD_NAMES == 0)
	GVariant *value = g_variant_new_string (gtk_entry_get_text (entry));
	dbusmenu_menuitem_handle_event (mi, "send", value, gtk_get_current_event_time());
#else
	GValue value = G_VALUE_INIT;
	g_value_init (&value, G_TYPE_STRING);
	g_value_set_static_string (&value, gtk_entry_get_text (entry));

	//g_print ("user typed: %s\n", g_value_get_string (&value));
	dbusmenu_menuitem_handle_event (mi, "send", &value, gtk_get_current_event_time());
#endif
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
#if (INDICATOR_OLD_NAMES == 0)
about_me_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, GVariant * value, AboutMeMenuItem *item)
#else
about_me_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, GValue * value, AboutMeMenuItem *item)
#endif
{
  g_return_if_fail (ABOUT_IS_ME_MENU_ITEM (item));

	if (!g_strcmp0(prop, DBUSMENU_ABOUT_ME_MENUITEM_PROP_ICON)) {
    /* reload the avatar icon */
#if (INDICATOR_OLD_NAMES == 0)
    about_me_menu_item_load_avatar (item, g_variant_get_string (value, NULL));
#else
    about_me_menu_item_load_avatar (item, g_value_get_string(value));
#endif
  } else if (!g_strcmp0(prop, DBUSMENU_MENUITEM_PROP_VISIBLE)) {
    /* normal, ignore */
	} else {
		cd_warning("Indicator Item property '%s' unknown", prop);
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


  ///////////////
 // MAKE MENU //
///////////////

void cd_me_add_menu_handler (DbusmenuGtkClient * client)
{
#if (INDICATOR_OLD_NAMES == 0)
	#if (DBUSMENU_MAJOR > 0 || DBUSMENU_MINOR >= 2)
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client), DBUSMENU_ENTRY_MENUITEM_TYPE, (DbusmenuClientTypeHandler) new_entry_item);
	#endif
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client), DBUSMENU_ABOUT_ME_MENUITEM_TYPE, (DbusmenuClientTypeHandler) new_about_me_item);
#else
	#if (DBUSMENU_MAJOR > 0 || DBUSMENU_MINOR >= 2)
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client), DBUSMENU_ENTRY_MENUITEM_TYPE, new_entry_item);
	#endif
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client), DBUSMENU_ABOUT_ME_MENUITEM_TYPE, new_about_me_item);
#endif
}
