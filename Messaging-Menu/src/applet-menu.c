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
#include <math.h>

#include "applet-struct.h"
#include "applet-menu.h"

#include <libdbusmenu-gtk/menuitem.h>
#include <libdbusmenu-gtk/menu.h>

#define FORCE_REMOVE_DOUBLE_ENTRIES

#define M_PI G_PI
#define RIGHT_LABEL_FONT_SIZE 12
#define RIGHT_LABEL_RADIUS 20

#if (INDICATOR_MESSAGES_HAS_LOZENGE != 1)
static GtkSizeGroup * indicator_right_group = NULL;  /// TODO: check if it needs to be freed...
#endif

  //////////////////////
 // APPLICATION ITEM //
//////////////////////

/* Sets the icon when it changes. */
static void
application_icon_change_cb (DbusmenuMenuitem * mi, gchar * prop, GVariant * value, gpointer user_data)
{
	if (!g_strcmp0(prop, APPLICATION_MENUITEM_PROP_ICON)) {
		/* Set the main icon */
		if (GTK_IS_IMAGE(user_data)) {
			gtk_image_set_from_icon_name(GTK_IMAGE(user_data), g_variant_get_string(value, NULL), GTK_ICON_SIZE_MENU);
		}
	}

	return;
}

/* Sets the label when it changes. */
static void
application_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, GVariant * value, gpointer user_data)
{
	if (!g_strcmp0(prop, APPLICATION_MENUITEM_PROP_NAME)) {
		/* Set the main label */
		if (GTK_IS_LABEL(user_data)) {
			gtk_label_set_text(GTK_LABEL(user_data), g_variant_get_string(value, NULL));
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
application_triangle_draw_cb (GtkWidget *widget, cairo_t *ctx, gpointer data)
{
	int x, y, arrow_width, arrow_height;

	if (!GTK_IS_WIDGET (widget)) return FALSE;
	if (!DBUSMENU_IS_MENUITEM (data)) return FALSE;

	/* render the triangle indicator only if the application is running */
	if (! dbusmenu_menuitem_property_get_bool (DBUSMENU_MENUITEM(data), APPLICATION_MENUITEM_PROP_RUNNING))
		return FALSE;;

	arrow_width = 5; /* the pixel-based reference triangle is 5x9 */
	arrow_height = 9;
	
	/* get style + arrow position */
	double red, green, blue;
	GtkStyleContext *style = gtk_widget_get_style_context (widget);
	GdkRGBA color;
	gtk_style_context_get_color (style, gtk_widget_get_state(widget), &color);
	red = color.red;
	green = color.green;
	blue = color.blue;
	x = 0;  // the context is already translated so that (0;0) is the top-left corner of the widget.
	y = gtk_widget_get_allocated_height (widget)/2.0 - (double)arrow_height/2.0;

	cairo_set_line_width (ctx, 1.0);

	/* cairo drawing code */
	cairo_move_to (ctx, x, y);
	cairo_line_to (ctx, x, y + arrow_height);
	cairo_line_to (ctx, x + arrow_width, y + (double)arrow_height/2.0);
	cairo_close_path (ctx);
	cairo_set_source_rgb (ctx, red, green, blue);
	cairo_fill (ctx);

	return FALSE;
}

static gint
gtk_widget_get_font_size (GtkWidget *widget)
{
	const PangoFontDescription *font;

	font = gtk_style_context_get_font (gtk_widget_get_style_context (widget),
	                                   gtk_widget_get_state_flags (widget));

	return pango_font_description_get_size (font) / PANGO_SCALE;
}

/* Custom function to draw rounded rectangle with max radius */
static void
custom_cairo_rounded_rectangle (cairo_t *cr,
                                double x, double y, double w, double h)
{
	double radius = h / 2.0;

	cairo_move_to (cr, x+radius, y);
	cairo_arc (cr, x+w-radius, y+radius, radius, M_PI*1.5, M_PI*2);
	cairo_arc (cr, x+w-radius, y+h-radius, radius, 0, M_PI*0.5);
	cairo_arc (cr, x+radius,   y+h-radius, radius, M_PI*0.5, M_PI);
	cairo_arc (cr, x+radius,   y+radius,   radius, M_PI, M_PI*1.5);
}

/* Draws a rounded rectangle with text inside. */
static gboolean
numbers_draw_cb (GtkWidget *widget, cairo_t *ctx, gpointer data)
{
	double x, y, w, h;
	PangoLayout * layout;
	gint font_size = gtk_widget_get_font_size (widget);

	#if (INDICATOR_MESSAGES_HAS_LOZENGE == 1)
	gboolean is_lozenge = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "is-lozenge"));
	/* let the label handle the drawing if it's not a lozenge */
	if (!is_lozenge)
		return FALSE;
	#endif

	if (!GTK_IS_WIDGET (widget)) return FALSE;

	/* get style + arrow position / dimensions */
	double red, green, blue;
	GtkStyleContext *style = gtk_widget_get_style_context (widget);
	GdkRGBA color;
	gtk_style_context_get_color (style, gtk_widget_get_state(widget), &color);
	red = color.red;
	green = color.green;
	blue = color.blue;
	GtkAllocation allocation;
	gtk_widget_get_allocation (widget, &allocation);
	w = allocation.width;
	h = allocation.height;
	x = y = 0;

	layout = gtk_label_get_layout (GTK_LABEL(widget));
	PangoRectangle layout_extents;
	pango_layout_get_extents (layout, NULL, &layout_extents);
	pango_extents_to_pixels (&layout_extents, NULL);

	if (layout_extents.width == 0)
		return TRUE;

	cairo_set_line_width (ctx, 1.0);

	cairo_set_fill_rule (ctx, CAIRO_FILL_RULE_EVEN_ODD);

	/* cairo drawing code */
	custom_cairo_rounded_rectangle (ctx, x - font_size/2.0, y, w + font_size, h);

	cairo_set_source_rgba (ctx, red,
	                           green,
	                           blue, 0.5);

	x += (allocation.width - layout_extents.width) / 2.0;
	y += (allocation.height - layout_extents.height) / 2.0;
	cairo_move_to (ctx, floor (x), floor (y));
	pango_cairo_layout_path (ctx, layout);
	cairo_fill (ctx);

	return TRUE;
}

/* Builds a menu item representing a running application in the
   messaging menu */
static gboolean
new_application_item (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client)
{
	gchar *cName = g_strdup (dbusmenu_menuitem_property_get(newitem, APPLICATION_MENUITEM_PROP_NAME));
	const gchar *sIconName = dbusmenu_menuitem_property_get (newitem, APPLICATION_MENUITEM_PROP_ICON);

	cd_debug ("%s (\"%s\")", __func__, cName);

#ifdef FORCE_REMOVE_DOUBLE_ENTRIES
	if (newitem == NULL || !dbusmenu_menuitem_property_get_bool(newitem, DBUSMENU_MENUITEM_PROP_VISIBLE)
		#if INDICATOR_MESSAGES_HAS_LOZENGE == 1
		&& sIconName != NULL && *sIconName != '\0' // these menu 
		#endif
		)
	{
		dbusmenu_menuitem_child_delete (parent, newitem);
		cd_debug ("Not visible: %s", cName);
		g_free (cName);
		return TRUE;
	}
#endif

	GtkMenuItem * gmi = GTK_MENU_ITEM(gtk_image_menu_item_new());

	gint padding = 4;
	gtk_widget_style_get(GTK_WIDGET(gmi), "toggle-spacing", &padding, NULL);

	GtkWidget * hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, padding);

	// Added for Cairo-Dock
	#if INDICATOR_MESSAGES_HAS_LOZENGE == 1 // we add a left margin
	if (! dbusmenu_menuitem_property_get_bool(newitem, DBUSMENU_MENUITEM_PROP_VISIBLE)
		&& (sIconName == NULL || *sIconName == '\0'))
	{
		gint width, height;
		gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);
		gtk_widget_set_size_request(GTK_WIDGET (gmi), -1, height + 4);
		gtk_widget_set_margin_left (hbox, width + padding);
	}
	#endif
	// end

	GtkWidget * icon = gtk_image_new_from_icon_name(sIconName, GTK_ICON_SIZE_MENU);
	gtk_misc_set_alignment(GTK_MISC(icon), 1.0 /* right aligned */, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

	/* Application name in a label */
	GtkWidget * label = gtk_label_new(cName);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(gmi), hbox);
	gtk_widget_show_all (GTK_WIDGET (gmi));

	/* Attach some of the standard GTK stuff */
	dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, gmi, parent);

	/* Make sure we can handle the label changing */
	g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(application_prop_change_cb), label);
	g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(application_icon_change_cb), icon);
	g_signal_connect_after(G_OBJECT (gmi), "draw", G_CALLBACK (application_triangle_draw_cb), newitem);

	g_free (cName);

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
indicator_prop_change_cb (DbusmenuMenuitem * mi, gchar * prop, GVariant * value, indicator_item_t * mi_data)
{
	cd_debug ("%s (\"%s\": %s)", __func__, prop, g_variant_get_string(value, NULL));

	if (!g_strcmp0(prop, INDICATOR_MENUITEM_PROP_LABEL)) {
		/* Set the main label */
		gtk_label_set_text(GTK_LABEL(mi_data->label), g_variant_get_string(value, NULL));
	} else if (!g_strcmp0(prop, INDICATOR_MENUITEM_PROP_RIGHT)) {
		/* Set the right label */
		gtk_label_set_text(GTK_LABEL(mi_data->right), g_variant_get_string(value, NULL));
#if (INDICATOR_MESSAGES_HAS_LOZENGE == 1)
	} else if (!g_strcmp0(prop, INDICATOR_MENUITEM_PROP_RIGHT_IS_LOZENGE)) {
		g_object_set_data (G_OBJECT (mi_data->right), "is-lozenge", GINT_TO_POINTER (TRUE));
		gtk_widget_queue_draw (mi_data->right);
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

	if (newitem == NULL || !dbusmenu_menuitem_property_get_bool(newitem, DBUSMENU_MENUITEM_PROP_VISIBLE))
	{
		cd_debug ("Not visible: %s", dbusmenu_menuitem_property_get(newitem, INDICATOR_MENUITEM_PROP_LABEL));
		return TRUE;
	}

	indicator_item_t * mi_data = g_new0(indicator_item_t, 1);

	GtkMenuItem * gmi = GTK_MENU_ITEM(gtk_menu_item_new());

	gint padding = 4;
	gint font_size = gtk_widget_get_font_size (GTK_WIDGET (gmi));
	gtk_widget_style_get(GTK_WIDGET(gmi), "toggle-spacing", &padding, NULL);

	GtkWidget * hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, padding);

	/* Icon, probably someone's face or avatar on an IM */
	mi_data->icon = gtk_image_new();

	/* Set the minimum size, we always want it to take space */
	gint width, height;
	gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);
	gtk_widget_set_size_request(GTK_WIDGET (gmi), -1, height + 4);

	gtk_widget_set_margin_left (hbox, width + padding);

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
	gtk_box_pack_start(GTK_BOX(hbox), mi_data->icon, FALSE, FALSE, 0);

	if (pixbuf != NULL) {
		gtk_widget_show(mi_data->icon);
	}

	/* Label, probably a username, chat room or mailbox name */
	mi_data->label = gtk_label_new(dbusmenu_menuitem_property_get(newitem, INDICATOR_MENUITEM_PROP_LABEL));
	gtk_misc_set_alignment(GTK_MISC(mi_data->label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), mi_data->label, TRUE, TRUE, 0);
	gtk_widget_show(mi_data->label);

	/* Usually either the time or the count on the individual
	   item. */
	mi_data->right = gtk_label_new(dbusmenu_menuitem_property_get(newitem, INDICATOR_MENUITEM_PROP_RIGHT));
	#if (INDICATOR_MESSAGES_HAS_LOZENGE == 1)
	g_object_set_data (G_OBJECT (mi_data->right),
			   "is-lozenge",
			   GINT_TO_POINTER (dbusmenu_menuitem_property_get_bool (newitem, INDICATOR_MENUITEM_PROP_RIGHT_IS_LOZENGE)));
	#else
	gtk_size_group_add_widget(indicator_right_group, mi_data->right);
	#endif
	/* install extra decoration overlay */
	g_signal_connect (G_OBJECT (mi_data->right), "draw", G_CALLBACK (numbers_draw_cb), NULL);
	
	gtk_misc_set_alignment(GTK_MISC(mi_data->right), 1.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), mi_data->right, FALSE, FALSE, padding + font_size/2.0);
	gtk_label_set_width_chars (GTK_LABEL (mi_data->right), 2);
	#if INDICATOR_MESSAGES_HAS_LOZENGE == 1
	gtk_style_context_add_class (gtk_widget_get_style_context (mi_data->right),
	                             "accelerator");
	#endif
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
	#if (INDICATOR_MESSAGES_HAS_LOZENGE != 1)
	indicator_right_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	#endif
	
	dbusmenu_client_add_type_handler(DBUSMENU_CLIENT(client), INDICATOR_MENUITEM_TYPE, (DbusmenuClientTypeHandler) new_indicator_item);
	dbusmenu_client_add_type_handler(DBUSMENU_CLIENT(client), APPLICATION_MENUITEM_TYPE, (DbusmenuClientTypeHandler) new_application_item);
}
