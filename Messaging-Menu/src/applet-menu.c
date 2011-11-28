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
#include "applet-menu.h"

#define M_PI G_PI
#define RIGHT_LABEL_FONT_SIZE 12
#define RIGHT_LABEL_RADIUS 20

static GtkSizeGroup * indicator_right_group = NULL;  /// TODO: check if it needs to be freed...

  //////////////////////
 // APPLICATION ITEM //
//////////////////////

/* Sets the icon when it changes. */
static void
#if (INDICATOR_OLD_NAMES == 0)
application_icon_change_cb (DbusmenuMenuitem * mi, gchar * prop, GVariant * value, gpointer user_data)
#else
application_icon_change_cb (DbusmenuMenuitem * mi, gchar * prop, GValue * value, gpointer user_data)
#endif
{
	if (!g_strcmp0(prop, APPLICATION_MENUITEM_PROP_ICON)) {
		/* Set the main icon */
		if (GTK_IS_IMAGE(user_data)) {
#if (INDICATOR_OLD_NAMES == 0)
			gtk_image_set_from_icon_name(GTK_IMAGE(user_data), g_variant_get_string(value, NULL), GTK_ICON_SIZE_MENU);
#else
			gtk_image_set_from_icon_name(GTK_IMAGE(user_data), g_value_get_string(value), GTK_ICON_SIZE_MENU);
#endif
		}
	}

	return;
}

/* Sets the label when it changes. */
static void
#if (INDICATOR_OLD_NAMES == 0)
application_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, GVariant * value, gpointer user_data)
#else
application_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, GValue * value, gpointer user_data)
#endif
{
	if (!g_strcmp0(prop, APPLICATION_MENUITEM_PROP_NAME)) {
		/* Set the main label */
		if (GTK_IS_LABEL(user_data)) {
#if (INDICATOR_OLD_NAMES == 0)
			gtk_label_set_text(GTK_LABEL(user_data), g_variant_get_string(value, NULL));
#else
			gtk_label_set_text(GTK_LABEL(user_data), g_value_get_string(value));
#endif
		}
	}

	if (!g_strcmp0(prop, APPLICATION_MENUITEM_PROP_RUNNING)) {
		/* TODO: should hide/show the triangle live if the menu was open.
		   In practice, this is rarely needed. */
	}

	return;
}

/* Draws a triangle on the left, using fg[STATE_TYPE] color. */
static gboolean
application_triangle_draw_cb (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	GtkStyle *style;
	cairo_t *cr;
	int x, y, arrow_width, arrow_height;

	if (!GTK_IS_WIDGET (widget)) return FALSE;
	if (!DBUSMENU_IS_MENUITEM (data)) return FALSE;

	/* render the triangle indicator only if the application is running */
	if (! dbusmenu_menuitem_property_get_bool (DBUSMENU_MENUITEM(data), APPLICATION_MENUITEM_PROP_RUNNING))
		return FALSE;;

	/* get style */
	style = gtk_widget_get_style (widget);
	GtkAllocation allocation;
	gtk_widget_get_allocation (widget, &allocation);

	/* set arrow position / dimensions */
	arrow_width = 5; /* the pixel-based reference triangle is 5x9 */
	arrow_height = 9;
	x = allocation.x;
	y = allocation.y + allocation.height/2.0 - (double)arrow_height/2.0;

	/* initialize cairo drawing area */
	cr = (cairo_t*) gdk_cairo_create (gtk_widget_get_window (widget));

	/* set line width */	
	cairo_set_line_width (cr, 1.0);

	/* cairo drawing code */
	cairo_move_to (cr, x, y);
	cairo_line_to (cr, x, y + arrow_height);
	cairo_line_to (cr, x + arrow_width, y + (double)arrow_height/2.0);
	cairo_close_path (cr);
	cairo_set_source_rgb (cr, style->fg[gtk_widget_get_state(widget)].red/65535.0,
	                          style->fg[gtk_widget_get_state(widget)].green/65535.0,
	                          style->fg[gtk_widget_get_state(widget)].blue/65535.0);
	cairo_fill (cr);

	/* remember to destroy cairo context to avoid leaks */
	cairo_destroy (cr);

	return FALSE;
}

/* Custom function to draw rounded rectangle with max radius */
static void
custom_cairo_rounded_rectangle (cairo_t *cr,
                                double x, double y, double w, double h)
{
	double radius = MIN (w/2.0, h/2.0);

	cairo_move_to (cr, x+radius, y);
	cairo_arc (cr, x+w-radius, y+radius, radius, M_PI*1.5, M_PI*2);
	cairo_arc (cr, x+w-radius, y+h-radius, radius, 0, M_PI*0.5);
	cairo_arc (cr, x+radius,   y+h-radius, radius, M_PI*0.5, M_PI);
	cairo_arc (cr, x+radius,   y+radius,   radius, M_PI, M_PI*1.5);
}

/* Draws a rounded rectangle with text inside. */
static gboolean
numbers_draw_cb (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	GtkStyle *style;
	cairo_t *cr;
	double x, y, w, h;
	PangoLayout * layout;
	gint font_size = RIGHT_LABEL_FONT_SIZE;

	if (!GTK_IS_WIDGET (widget)) return FALSE;

	/* get style */
	style = gtk_widget_get_style (widget);

	/* set arrow position / dimensions */
	GtkAllocation allocation;
	gtk_widget_get_allocation (widget, &allocation);
	w = allocation.width;
	h = allocation.height;
	x = allocation.x;
	y = allocation.y;

	layout = gtk_label_get_layout (GTK_LABEL(widget));

	/* This does not work, don't ask me why but font_size is 0.
	 * I wanted to use a dynamic font size to adjust the padding on left/right edges
	 * of the rounded rectangle. Andrea Cimitan */
	/* const PangoFontDescription * font_description = pango_layout_get_font_description (layout);
	font_size = pango_font_description_get_size (font_description); */

	/* initialize cairo drawing area */
	cr = (cairo_t*) gdk_cairo_create (gtk_widget_get_window (widget));

	/* set line width */	
	cairo_set_line_width (cr, 1.0);

	cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);

	/* cairo drawing code */
	custom_cairo_rounded_rectangle (cr, x - font_size/2.0, y, w + font_size, h);

	cairo_set_source_rgba (cr, style->fg[gtk_widget_get_state(widget)].red/65535.0,
	                           style->fg[gtk_widget_get_state(widget)].green/65535.0,
	                           style->fg[gtk_widget_get_state(widget)].blue/65535.0, 0.5);

	cairo_move_to (cr, x, y);
	pango_cairo_layout_path (cr, layout);
	cairo_fill (cr);

	/* remember to destroy cairo context to avoid leaks */
        cairo_destroy (cr);

	return TRUE;
}

/* Builds a menu item representing a running application in the
   messaging menu */
static gboolean
new_application_item (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client)
{
	gchar *cName = g_strdup (dbusmenu_menuitem_property_get(newitem, APPLICATION_MENUITEM_PROP_NAME));

	cd_debug ("%s (\"%s\")", __func__, cName);

#if (INDICATOR_OLD_NAMES == 0)
	if (newitem == NULL || !dbusmenu_menuitem_property_get_bool(newitem, DBUSMENU_MENUITEM_PROP_VISIBLE))
	{
		cd_debug ("Not visible: %s", cName);
		g_free (cName);
		return TRUE;
	}
#endif

	GtkMenuItem * gmi = GTK_MENU_ITEM(gtk_image_menu_item_new());
#if (GTK_MAJOR_VERSION > 2 || GTK_MINOR_VERSION >= 16)
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(gmi), TRUE);
#endif

	/* Set the minimum size, we always want it to take space */
	gint width, height;
	gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);

	GtkWidget * icon = gtk_image_new_from_icon_name(dbusmenu_menuitem_property_get(newitem, APPLICATION_MENUITEM_PROP_ICON), GTK_ICON_SIZE_MENU);
	gtk_widget_set_size_request(icon, width
								+ 5 /* ref triangle is 5x9 pixels */
								+ 2 /* padding */,
								height);
	gtk_misc_set_alignment(GTK_MISC(icon), 1.0 /* right aligned */, 0.5);
#if (GTK_MAJOR_VERSION > 2 || GTK_MINOR_VERSION >= 16)
	gtk_image_menu_item_set_always_show_image (GTK_IMAGE_MENU_ITEM (gmi), TRUE);
#endif
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(gmi), icon);
	gtk_widget_show(icon);

	/* Application name in a label */
	GtkWidget * label = gtk_label_new(cName);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_widget_show(label);

	/* Insert the hbox */
	gtk_container_add(GTK_CONTAINER(gmi), label);

	/* Attach some of the standard GTK stuff */
	dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, gmi, parent);

	/* Make sure we can handle the label changing */
	g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(application_prop_change_cb), label);
	g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(application_icon_change_cb), icon);
	g_signal_connect_after(G_OBJECT (gmi), "expose_event", G_CALLBACK (application_triangle_draw_cb), newitem);

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
#if (INDICATOR_OLD_NAMES == 0)
indicator_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, GVariant * value, indicator_item_t * mi_data)
#else
indicator_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, GValue * value, indicator_item_t * mi_data)
#endif
{
#if (INDICATOR_OLD_NAMES == 0)
	cd_debug ("%s (\"%s\": %s)", __func__, prop, g_variant_get_string(value, NULL));
#else
	cd_debug ("%s (\"%s\": %s)", __func__, prop, g_value_get_string(value));
#endif

	if (!g_strcmp0(prop, INDICATOR_MENUITEM_PROP_LABEL)) {
		/* Set the main label */
#if (INDICATOR_OLD_NAMES == 0)
		gtk_label_set_text(GTK_LABEL(mi_data->label), g_variant_get_string(value, NULL));
#else
		gtk_label_set_text(GTK_LABEL(mi_data->label), g_value_get_string(value));
#endif
	} else if (!g_strcmp0(prop, INDICATOR_MENUITEM_PROP_RIGHT)) {
		/* Set the right label */
#if (INDICATOR_OLD_NAMES == 0)
		gtk_label_set_text(GTK_LABEL(mi_data->right), g_variant_get_string(value, NULL));
#else
		gtk_label_set_text(GTK_LABEL(mi_data->right), g_value_get_string(value));
#endif
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
				cd_debug("Resizing icon from %dx%d to %dx%d", gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf), width, height);
				resized_pixbuf = gdk_pixbuf_scale_simple(pixbuf,
				                                         width,
				                                         height,
				                                         GDK_INTERP_BILINEAR);
				g_object_unref(pixbuf);
			} else {
				cd_debug("Happy with icon sized %dx%d", gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
				resized_pixbuf = pixbuf;
			}
	  
			gtk_image_set_from_pixbuf(GTK_IMAGE(mi_data->icon), resized_pixbuf);

			g_object_unref(resized_pixbuf);

			gtk_widget_show(mi_data->icon);
		} else {
			gtk_widget_hide(mi_data->icon);
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
	g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
	g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);
	/* Note: not checking parent, it's reasonable for it to be NULL */

	cd_debug ("%s (\"%s\")", __func__, dbusmenu_menuitem_property_get(newitem, INDICATOR_MENUITEM_PROP_LABEL));

#if (INDICATOR_OLD_NAMES == 0)
	if (newitem == NULL || !dbusmenu_menuitem_property_get_bool(newitem, DBUSMENU_MENUITEM_PROP_VISIBLE))
	{
		cd_debug ("Not visible: %s", dbusmenu_menuitem_property_get(newitem, INDICATOR_MENUITEM_PROP_LABEL));
		return TRUE;
	}
#endif

	indicator_item_t * mi_data = g_new0(indicator_item_t, 1);

	GtkMenuItem * gmi = GTK_MENU_ITEM(gtk_menu_item_new());

	gint padding = 4;
	gint font_size = RIGHT_LABEL_FONT_SIZE;
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
			cd_debug("Resizing icon from %dx%d to %dx%d", gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf), width, height);
			resized_pixbuf = gdk_pixbuf_scale_simple(pixbuf,
			                                         width,
			                                         height,
			                                         GDK_INTERP_BILINEAR);
			g_object_unref(pixbuf);
		} else {
			cd_debug("Happy with icon sized %dx%d", gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
			resized_pixbuf = pixbuf;
		}
  
		gtk_image_set_from_pixbuf(GTK_IMAGE(mi_data->icon), resized_pixbuf);

		g_object_unref(resized_pixbuf);
	}
	gtk_misc_set_alignment(GTK_MISC(mi_data->icon), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), mi_data->icon, FALSE, FALSE, padding);

	if (pixbuf != NULL) {
		gtk_widget_show(mi_data->icon);
	}

	/* Label, probably a username, chat room or mailbox name */
	mi_data->label = gtk_label_new(dbusmenu_menuitem_property_get(newitem, INDICATOR_MENUITEM_PROP_LABEL));
	gtk_misc_set_alignment(GTK_MISC(mi_data->label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), mi_data->label, TRUE, TRUE, padding);
	gtk_widget_show(mi_data->label);

	/* Usually either the time or the count on the individual
	   item. */
	mi_data->right = gtk_label_new(dbusmenu_menuitem_property_get(newitem, INDICATOR_MENUITEM_PROP_RIGHT));
	gtk_size_group_add_widget(indicator_right_group, mi_data->right);
	/* install extra decoration overlay */
	g_signal_connect (G_OBJECT (mi_data->right), "expose_event",
	                  G_CALLBACK (numbers_draw_cb), NULL);

	gtk_misc_set_alignment(GTK_MISC(mi_data->right), 1.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), mi_data->right, FALSE, FALSE, padding + font_size/2.0);
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
	indicator_right_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	
	dbusmenu_client_add_type_handler(DBUSMENU_CLIENT(client), INDICATOR_MENUITEM_TYPE, (DbusmenuClientTypeHandler) new_indicator_item);
	dbusmenu_client_add_type_handler(DBUSMENU_CLIENT(client), APPLICATION_MENUITEM_TYPE, (DbusmenuClientTypeHandler) new_application_item);
}
