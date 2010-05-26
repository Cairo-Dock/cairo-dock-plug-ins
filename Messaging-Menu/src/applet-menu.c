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

#include <libdbusmenu-gtk/menuitem.h>
#include <libdbusmenu-gtk/menu.h>

#include "applet-struct.h"
#include "dbus-data.h"
#include "applet-menu.h"

  //////////////////////
 // APPLICATION ITEM //
//////////////////////

/* Sets the icon when it changes. */
static void
application_icon_change_cb (DbusmenuMenuitem * mi, gchar * prop, GValue * value, gpointer user_data)
{
	if (!g_strcmp0(prop, APPLICATION_MENUITEM_PROP_ICON)) {
		/* Set the main icon */
		if (GTK_IS_IMAGE(user_data)) {
			gtk_image_set_from_icon_name(GTK_IMAGE(user_data), g_value_get_string(value), GTK_ICON_SIZE_MENU);
		}
	}

	return;
}

/* Sets the label when it changes. */
static void
application_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, GValue * value, gpointer user_data)
{
	if (!g_strcmp0(prop, APPLICATION_MENUITEM_PROP_NAME)) {
		/* Set the main label */
		if (GTK_IS_LABEL(user_data)) {
			gtk_label_set_text(GTK_LABEL(user_data), g_value_get_string(value));
		}
	}

	return;
}

/* Builds a menu item representing a running application in the
   messaging menu */
static gboolean
new_application_item (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client)
{
	g_print ("%s ()\n", __func__);
	GtkMenuItem * gmi = GTK_MENU_ITEM(gtk_image_menu_item_new());

	gint padding = 4;
	gtk_widget_style_get(GTK_WIDGET(gmi), "horizontal-padding", &padding, NULL);

	GtkWidget * hbox = gtk_hbox_new(FALSE, 0);

	/* Set the minimum size, we always want it to take space */
	gint width, height;
	gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);

	GtkWidget * icon = gtk_image_new_from_icon_name(dbusmenu_menuitem_property_get(newitem, APPLICATION_MENUITEM_PROP_ICON), GTK_ICON_SIZE_MENU);
	gtk_widget_set_size_request(icon, width, height);
	gtk_misc_set_alignment(GTK_MISC(icon), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, padding);
	gtk_widget_show(icon);

	/* Application name in a label */
	g_print ("  name : %s\n", dbusmenu_menuitem_property_get(newitem, APPLICATION_MENUITEM_PROP_NAME));
	GtkWidget * label = gtk_label_new(dbusmenu_menuitem_property_get(newitem, APPLICATION_MENUITEM_PROP_NAME));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, padding);
	gtk_widget_show(label);

	/* Insert the hbox */
	gtk_container_add(GTK_CONTAINER(gmi), hbox);
	gtk_widget_show(hbox);

	/* Build up the running icon */
	GtkWidget * running_icon = gtk_image_new_from_icon_name("application-running", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(gmi), running_icon);
	gtk_widget_show(running_icon);

	/* Make sure it always appears */
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(gmi), TRUE);

	/* Attach some of the standard GTK stuff */
	dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, gmi, parent);

	/* Make sure we can handle the label changing */
	g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(application_prop_change_cb), label);
	g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(application_icon_change_cb), icon);

	return TRUE;
}


  ////////////////////
 // INDICATOR ITEM //
////////////////////

typedef struct _indicator_item_t indicator_item_t;
struct _indicator_item_t {
	GtkWidget * icon;
	GtkWidget * label;
	GtkWidget * right;
};

/* Whenever we have a property change on a DbusmenuMenuitem
   we need to be responsive to that. */
static void
indicator_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, GValue * value, indicator_item_t * mi_data)
{
	g_print ("%s (%s)\n", __func__, prop);
	if (!g_strcmp0(prop, INDICATOR_MENUITEM_PROP_LABEL)) {
		/* Set the main label */
		gtk_label_set_text(GTK_LABEL(mi_data->label), g_value_get_string(value));
	} else if (!g_strcmp0(prop, INDICATOR_MENUITEM_PROP_RIGHT)) {
		/* Set the right label */
		gtk_label_set_text(GTK_LABEL(mi_data->right), g_value_get_string(value));
	} else if (!g_strcmp0(prop, INDICATOR_MENUITEM_PROP_ICON)) {
		/* We don't use the value here, which is probably less efficient, 
		   but it's easier to use the easy function.  And since th value
		   is already cached, shouldn't be a big deal really.  */
		GdkPixbuf * pixbuf = dbusmenu_menuitem_property_get_image(mi, INDICATOR_MENUITEM_PROP_ICON);
		if (pixbuf != NULL) {
			/* If we've got a pixbuf we need to make sure it's of a reasonable
			   size to fit in the menu.  If not, rescale it. */
			GdkPixbuf * resized_pixbuf;
			gint width, height;
			gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);
			if (gdk_pixbuf_get_width(pixbuf) > width ||
					gdk_pixbuf_get_height(pixbuf) > height) {
				g_debug("Resizing icon from %dx%d to %dx%d", gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf), width, height);
				resized_pixbuf = gdk_pixbuf_scale_simple(pixbuf,
				                                         width,
				                                         height,
				                                         GDK_INTERP_BILINEAR);
			} else {
				g_debug("Happy with icon sized %dx%d", gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
				resized_pixbuf = pixbuf;
			}
	  
			gtk_image_set_from_pixbuf(GTK_IMAGE(mi_data->icon), resized_pixbuf);

			/* The other pixbuf should be free'd by the dbusmenu. */
			if (resized_pixbuf != pixbuf) {
				g_object_unref(resized_pixbuf);
			}
		}
	}

	return;
}

/* We have a small little menuitem type that handles all
   of the fun stuff for indicators.  Mostly this is the
   shifting over and putting the icon in with some right
   side text that'll be determined by the service.  */
static gboolean
new_indicator_item (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client)
{
	g_print ("%s ()\n", __func__);
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);
	/* Note: not checking parent, it's reasonable for it to be NULL */

	indicator_item_t * mi_data = g_new0(indicator_item_t, 1);

	GtkMenuItem * gmi = GTK_MENU_ITEM(gtk_menu_item_new());

	gint padding = 4;
	gtk_widget_style_get(GTK_WIDGET(gmi), "horizontal-padding", &padding, NULL);

	GtkWidget * hbox = gtk_hbox_new(FALSE, 0);

	/* Icon, probably someone's face or avatar on an IM */
	mi_data->icon = gtk_image_new();

	/* Set the minimum size, we always want it to take space */
	gint width, height;
	gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);
	gtk_widget_set_size_request(mi_data->icon, width, height);

	GdkPixbuf * pixbuf = dbusmenu_menuitem_property_get_image(newitem, INDICATOR_MENUITEM_PROP_ICON);
	if (pixbuf != NULL) {
		/* If we've got a pixbuf we need to make sure it's of a reasonable
		   size to fit in the menu.  If not, rescale it. */
		GdkPixbuf * resized_pixbuf;
		if (gdk_pixbuf_get_width(pixbuf) > width ||
		        gdk_pixbuf_get_height(pixbuf) > height) {
			g_debug("Resizing icon from %dx%d to %dx%d", gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf), width, height);
			resized_pixbuf = gdk_pixbuf_scale_simple(pixbuf,
			                                         width,
			                                         height,
			                                         GDK_INTERP_BILINEAR);
		} else {
			g_debug("Happy with icon sized %dx%d", gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
			resized_pixbuf = pixbuf;
		}
  
		gtk_image_set_from_pixbuf(GTK_IMAGE(mi_data->icon), resized_pixbuf);

		/* The other pixbuf should be free'd by the dbusmenu. */
		if (resized_pixbuf != pixbuf) {
			g_object_unref(resized_pixbuf);
		}
	}
	gtk_misc_set_alignment(GTK_MISC(mi_data->icon), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), mi_data->icon, FALSE, FALSE, padding);
	gtk_widget_show(mi_data->icon);

	/* Label, probably a username, chat room or mailbox name */
	mi_data->label = gtk_label_new(dbusmenu_menuitem_property_get(newitem, INDICATOR_MENUITEM_PROP_LABEL));
	gtk_misc_set_alignment(GTK_MISC(mi_data->label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), mi_data->label, TRUE, TRUE, padding);
	gtk_widget_show(mi_data->label);

	/* Usually either the time or the count on the individual
	   item. */
	mi_data->right = gtk_label_new(dbusmenu_menuitem_property_get(newitem, INDICATOR_MENUITEM_PROP_RIGHT));
	gtk_size_group_add_widget(myData.indicator_right_group, mi_data->right);
	gtk_misc_set_alignment(GTK_MISC(mi_data->right), 1.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), mi_data->right, FALSE, FALSE, padding);
	gtk_widget_show(mi_data->right);

	gtk_container_add(GTK_CONTAINER(gmi), hbox);
	gtk_widget_show(hbox);

	dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, gmi, parent);

	g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(indicator_prop_change_cb), mi_data);
	g_object_weak_ref(G_OBJECT(newitem), (GWeakNotify)g_free, mi_data);

	return TRUE;
}


  ///////////////
 // MAKE MENU //
///////////////

void cd_messaging_add_menu_handler (DbusmenuGtkClient * client)
{
	dbusmenu_client_add_type_handler(DBUSMENU_CLIENT(client), INDICATOR_MENUITEM_TYPE, new_indicator_item);
	dbusmenu_client_add_type_handler(DBUSMENU_CLIENT(client), APPLICATION_MENUITEM_TYPE, new_application_item);
}
