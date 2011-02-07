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
#include <libido/idoentrymenuitem.h>

#include "dbus-shared-names.h"
#include "applet-struct.h"
#include "about-me-menu-item.h"
#include "applet-menu.h"
//~ 
//~ static gboolean new_entry_item (DbusmenuMenuitem * newitem,
                                //~ DbusmenuMenuitem * parent,
                                //~ DbusmenuClient   * client);
//~ static gboolean new_about_me_item (DbusmenuMenuitem * newitem,
                                //~ DbusmenuMenuitem * parent,
                                //~ DbusmenuClient   * client);
//~ static void     entry_activate_cb (GtkEntry *entry, DbusmenuMenuitem *mi);
//~ static void     entry_prop_change_cb (DbusmenuMenuitem *mi, gchar *prop, GValue *value, GtkEntry *entry);
//~ static gboolean entry_hint_is_shown (GtkWidget *widget);
//~ 
//~ static IdoEntryMenuItem *ido_entry = NULL;
//~ 
  //~ ////////////////
 //~ // TEXT ENTRY //
//~ ////////////////
//~ 
//~ 
//~ static void
//~ item_destroyed_cb (GtkObject *item,
                   //~ gpointer   user_data)
//~ {
  //~ g_signal_handlers_disconnect_by_func (user_data,
                                        //~ G_CALLBACK (entry_activate_cb),
                                        //~ item);
//~ 
  //~ g_signal_handlers_disconnect_by_func (user_data,
                                        //~ G_CALLBACK (entry_prop_change_cb),
                                        //~ item);
//~ }
//~ 
//~ void cd_me_delete_entry (void)
//~ {
	//~ if (ido_entry == NULL)
		//~ return;
	//~ DbusmenuMenuitem *mi = g_object_get_data (G_OBJECT (ido_entry),
                                              //~ "dbusmenuitem");
//~ 
	//~ g_signal_handlers_disconnect_by_func (ido_entry,
									  //~ G_CALLBACK (entry_activate_cb),
									  //~ mi);
//~ 
	//~ g_signal_handlers_disconnect_by_func (ido_entry,
									  //~ G_CALLBACK (entry_prop_change_cb),
									  //~ mi);
//~ 
	//~ gtk_widget_destroy (GTK_WIDGET (ido_entry));
	//~ ido_entry = NULL;
//~ }
//~ 
//~ 
//~ 
//~ static void
//~ entry_hint_set_shown (GtkWidget *widget, gboolean flag)
//~ {
  //~ g_object_set_data (G_OBJECT (widget),
                     //~ DBUSMENU_ENTRY_MENUITEM_PROP_HINT "_shown",
                     //~ GUINT_TO_POINTER (flag));
//~ }
//~ 
//~ 
//~ static void
//~ entry_set_style (GtkEntry *entry, GtkStateType state)
//~ {
    //~ GtkStyle *style = gtk_widget_get_style (GTK_WIDGET (entry));
    //~ GdkColor *color = &style->text[state];
    //~ gtk_widget_modify_text (GTK_WIDGET (entry), GTK_STATE_NORMAL, color);
    //~ gtk_widget_queue_draw (GTK_WIDGET (entry));
//~ }
//~ 
//~ /* unconditionnaly show the hint in the entry */
//~ static void
//~ entry_hint_show_hint (GtkEntry *entry)
//~ {
  //~ g_return_if_fail (GTK_IS_ENTRY (entry));
//~ 
  //~ const gchar *hint = g_object_get_data (G_OBJECT (entry),
                                         //~ DBUSMENU_ENTRY_MENUITEM_PROP_HINT);
  //~ gtk_entry_set_text (entry, hint);
  //~ entry_set_style (entry, GTK_STATE_INSENSITIVE);
  //~ entry_hint_set_shown (GTK_WIDGET (entry), TRUE);
//~ }
//~ 
//~ static void
//~ entry_maybe_show_hint (GtkEntry *entry)
//~ {
  //~ g_return_if_fail (GTK_IS_ENTRY (entry));
//~ 
  //~ const gchar *hint = g_object_get_data (G_OBJECT (entry),
                                         //~ DBUSMENU_ENTRY_MENUITEM_PROP_HINT);
//~ 
  //~ /* enforce style when typing a message */
  //~ if (GTK_WIDGET_HAS_FOCUS (entry)) {
    //~ entry_set_style (entry, GTK_STATE_NORMAL);
    //~ entry_hint_set_shown (GTK_WIDGET (entry), FALSE);
//~ 
    //~ return;
  //~ }
//~ 
  //~ if (gtk_entry_get_text_length (entry) > 0) {
//~ 
    //~ entry_set_style (entry, GTK_STATE_NORMAL);
    //~ entry_hint_set_shown (GTK_WIDGET (entry), FALSE);
//~ 
  //~ } else {
    //~ g_debug ("%s, nothing in the entry or not focused, so setting the hint to: %s", __func__, hint);
//~ 
    //~ entry_hint_show_hint (entry);
  //~ }
//~ }
//~ 
//~ static void
//~ entry_set_hint (GtkEntry *entry, const char *hint)
//~ {
    //~ g_return_if_fail (GTK_IS_ENTRY (entry));
//~ 
    //~ g_debug ("entry hint: %s", hint);
		//~ g_object_set_data_full (G_OBJECT (entry), DBUSMENU_ENTRY_MENUITEM_PROP_HINT,
                            //~ g_strdup (hint), (GDestroyNotify) g_free);
    //~ entry_maybe_show_hint (entry);
//~ }
//~ 
//~ static gboolean
//~ entry_hint_is_shown (GtkWidget *widget)
//~ {
  //~ gboolean shown = GPOINTER_TO_UINT
    //~ (g_object_get_data (G_OBJECT (widget),
                        //~ DBUSMENU_ENTRY_MENUITEM_PROP_HINT "_shown"));
//~ 
  //~ return shown;
//~ }
//~ 
//~ static gboolean
//~ entry_focus_grab_cb (GtkWidget *widget, GdkEventFocus *event)
//~ {
  //~ GtkEntry *entry = GTK_ENTRY (widget);
  //~ GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (entry));
  //~ gboolean select_on_focus;
//~ 
  //~ g_debug ("%s", __func__);
//~ 
  //~ if (entry_hint_is_shown (GTK_WIDGET (entry))) {
    //~ /* override select-on-focus */
    //~ g_object_get (settings, "gtk-entry-select-on-focus", &select_on_focus, NULL);
    //~ g_object_set (settings, "gtk-entry-select-on-focus", FALSE, NULL);
    //~ gtk_entry_set_text (entry, "");
    //~ g_object_set (settings, "gtk-entry-select-on-focus", select_on_focus, NULL);
  //~ }
//~ 
  //~ entry_hint_set_shown (GTK_WIDGET (entry), FALSE);
  //~ entry_set_style (entry, GTK_STATE_NORMAL);
//~ 
  //~ return FALSE;
//~ }
//~ 
//~ static gboolean
//~ entry_focus_out_cb (GtkWidget *widget, GdkEventFocus *event)
//~ {
  //~ g_return_val_if_fail (GTK_IS_ENTRY (widget), FALSE);
//~ 
  //~ GtkEntry *entry = GTK_ENTRY (widget);
//~ 
  //~ g_debug ("%s", __func__);
//~ 
  //~ if (! entry_hint_is_shown (GTK_WIDGET (entry)))
    //~ if (gtk_entry_get_text_length (entry) == 0)
      //~ /* show the hint unconditionnaly, as the focus change
         //~ may not have propagated already and entry_maybe_show_hint
         //~ could get confused */
      //~ entry_hint_show_hint (entry);
//~ 
  //~ return FALSE;
//~ }
//~ 
//~ static void
//~ entry_prop_change_cb (DbusmenuMenuitem *mi, gchar *prop, GValue *value, GtkEntry *entry)
//~ {
	//~ g_return_if_fail (GTK_IS_ENTRY (entry));
	//~ if (g_strcmp0 (prop, DBUSMENU_ENTRY_MENUITEM_PROP_TEXT) == 0) {
		//~ gtk_entry_set_text (entry, g_value_get_string (value));
		//~ if (gtk_entry_get_text_length (entry) == 0)
			//~ entry_maybe_show_hint (entry);
	//~ }
	//~ if (g_strcmp0 (prop, DBUSMENU_ENTRY_MENUITEM_PROP_HINT) == 0) {
    //~ entry_set_hint (entry, g_value_get_string (value));
    //~ if (entry_hint_is_shown (GTK_WIDGET (entry)))
        //~ entry_maybe_show_hint (entry); /* visual update */
	//~ }
//~ }
//~ 
//~ static void
//~ entry_activate_cb (GtkEntry *entry, DbusmenuMenuitem *mi)
//~ {
  //~ g_return_if_fail (GTK_IS_ENTRY (entry));
  //~ g_return_if_fail (DBUSMENU_IS_MENUITEM (mi));
//~ 
	//~ GValue value = G_VALUE_INIT;
	//~ g_value_init (&value, G_TYPE_STRING);
	//~ g_value_set_static_string (&value, gtk_entry_get_text (entry));
//~ 
	//~ //g_print ("user typed: %s\n", g_value_get_string (&value));
	//~ dbusmenu_menuitem_handle_event (mi, "send", &value, gtk_get_current_event_time());
//~ }
//~ 
//~ static gboolean
//~ new_entry_item (DbusmenuMenuitem * newitem,
                //~ DbusmenuMenuitem * parent,
                //~ DbusmenuClient   * client)
//~ {
	//~ g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
	//~ g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);
	//~ /* Note: not checking parent, it's reasonable for it to be NULL */
	//~ 
	//~ g_print ("%s ()\n", __func__);
	//~ IdoEntryMenuItem *ido = IDO_ENTRY_MENU_ITEM (ido_entry_menu_item_new ());
	//~ GtkEntry *entry = GTK_ENTRY(ido_entry_menu_item_get_entry (ido));
	//~ if (dbusmenu_menuitem_property_get (newitem, DBUSMENU_ENTRY_MENUITEM_PROP_TEXT) != NULL)
		//~ gtk_entry_set_text(entry, dbusmenu_menuitem_property_get(newitem, DBUSMENU_ENTRY_MENUITEM_PROP_TEXT));
	//~ if (dbusmenu_menuitem_property_get (newitem, DBUSMENU_ENTRY_MENUITEM_PROP_HINT) != NULL) {
    //~ entry_set_hint (entry, dbusmenu_menuitem_property_get (newitem, DBUSMENU_ENTRY_MENUITEM_PROP_HINT));
	//~ }
//~ 
  //~ g_object_set_data (G_OBJECT (ido),
                     //~ "dbusmenitem",
                     //~ newitem);
//~ 
	//~ gtk_entry_set_width_chars (entry, 23); /* set some nice aspect ratio for the menu */
  //~ gtk_entry_set_max_length (entry, 140); /* enforce current gwibber limit */
//~ 
  //~ /* override select-on-focus */
  //~ GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (entry));
  //~ g_object_set (settings, "gtk-entry-select-on-focus", FALSE, NULL);
//~ 
  //~ ido_entry = ido;
//~ 
//~ #if 0
  //~ /* this is dangerous: it leaves the signal connected even if the dbusmenu
     //~ object is disposed, for example if the service quits
  //~ */
  //~ g_signal_connect (ido,
                    //~ "notify::parent", G_CALLBACK (entry_parent_changed),
                    //~ NULL);
//~ #endif
//~ 
	//~ dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, GTK_MENU_ITEM(ido), parent);
//~ 
	//~ /* disconnect the activate signal that newitem_base connected with the wrong
	   //~ widget, ie menuitem, and re-connect it with the /entry/ instead */
	//~ gulong signal_id = g_signal_handler_find (GTK_MENU_ITEM (ido), G_SIGNAL_MATCH_DATA, 0, 0, 0, 0, parent);
  //~ if (signal_id > 0)
    //~ g_signal_handler_disconnect(GTK_MENU_ITEM (ido), signal_id);
//~ 
	//~ g_signal_connect (DBUSMENU_MENUITEM (newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK (entry_prop_change_cb), entry);
//~ 
	//~ g_signal_connect (GTK_ENTRY (entry), "activate", G_CALLBACK (entry_activate_cb), newitem);
//~ 
  //~ g_signal_connect (entry,
                    //~ "grab-focus", G_CALLBACK (entry_focus_grab_cb),
                    //~ NULL);
//~ 
  //~ g_signal_connect (entry,
                    //~ "focus-out-event", G_CALLBACK (entry_focus_out_cb),
                    //~ NULL);
//~ 
  //~ g_signal_connect (G_OBJECT (newitem),
                    //~ "destroy",
                    //~ G_CALLBACK (item_destroyed_cb),
                    //~ entry);
//~ 
	//~ return TRUE;
//~ }
//~ 
//~ /* Whenever we have a property change on a DbusmenuMenuitem
   //~ we need to be responsive to that. */
//~ static void
//~ about_me_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, GValue * value, AboutMeMenuItem *item)
//~ {
  //~ g_return_if_fail (ABOUT_IS_ME_MENU_ITEM (item));
//~ 
	//~ if (!g_strcmp0(prop, DBUSMENU_ABOUT_ME_MENUITEM_PROP_ICON)) {
    //~ /* reload the avatar icon */
    //~ about_me_menu_item_load_avatar (item, g_value_get_string(value));
  //~ } else if (!g_strcmp0(prop, DBUSMENU_MENUITEM_PROP_VISIBLE)) {
    //~ /* normal, ignore */
	//~ } else {
		//~ g_warning("Indicator Item property '%s' unknown", prop);
	//~ }
//~ 
	//~ return;
//~ }
//~ 
//~ static gboolean
//~ new_about_me_item (DbusmenuMenuitem * newitem,
                   //~ DbusmenuMenuitem * parent,
                   //~ DbusmenuClient * client)
//~ {
	//~ g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
	//~ g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);
	//~ /* Note: not checking parent, it's reasonable for it to be NULL */
//~ 
	//~ const gchar *name = dbusmenu_menuitem_property_get (newitem, DBUSMENU_ABOUT_ME_MENUITEM_PROP_NAME);
	//~ g_print ("about: %s\n", name);
	//~ AboutMeMenuItem *about = ABOUT_ME_MENU_ITEM (about_me_menu_item_new (name));
  //~ if (about != NULL) {
    //~ dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, GTK_MENU_ITEM(about), parent);
    //~ const gchar *avatar = dbusmenu_menuitem_property_get (newitem, DBUSMENU_ABOUT_ME_MENUITEM_PROP_ICON);
    //~ about_me_menu_item_load_avatar (about, avatar);
    //~ g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(about_me_prop_change_cb), about);
  //~ }
//~ 
	//~ return TRUE;
//~ }




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

	//g_print ("user typed: %s\n", g_value_get_string (&value));
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


  ///////////////
 // MAKE MENU //
///////////////

void cd_me_add_menu_handler (DbusmenuGtkClient * client)
{
	#if (DBUSMENU_MAJOR > 0 || DBUSMENU_MINOR >= 2)
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client), DBUSMENU_ENTRY_MENUITEM_TYPE, new_entry_item);
	#endif
	dbusmenu_client_add_type_handler (DBUSMENU_CLIENT(client), DBUSMENU_ABOUT_ME_MENUITEM_TYPE, new_about_me_item);
}
