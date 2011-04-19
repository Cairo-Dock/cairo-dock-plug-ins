/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * Copyright 2010 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of either or both of the following licenses:
 *
 * 1) the GNU Lesser General Public License version 3, as published by the
 * Free Software Foundation; and/or
 * 2) the GNU Lesser General Public License version 2.1, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the applicable version of the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of both the GNU Lesser General Public
 * License version 3 and version 2.1 along with this program.  If not, see
 * <http://www.gnu.org/licenses/>
 *
 * Authors:
 *    David Barth <david.barth@canonical.com>
 *    Cody Russell <crussell@canonical.com>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib/gstdio.h>

#include <gtk/gtk.h>
#include "about-me-menu-item.h"

static GObject* about_me_menu_item_constructor            (GType                  type,
                                                           guint                  n_construct_properties,
                                                           GObjectConstructParam *construct_params);
static void     about_me_menu_item_set_property           (GObject               *object,
                                                           guint                  prop_id,
                                                           const GValue          *value,
                                                           GParamSpec            *pspec);
static void     about_me_menu_item_get_property           (GObject               *object,
                                                           guint                  prop_id,
                                                           GValue                *value,
                                                           GParamSpec            *pspec);

struct _AboutMeMenuItemPrivate {
  GtkWidget     *label;
  GtkWidget     *image;
  GtkWidget     *hbox;
  gchar         *realname;
};

enum {
  PROP_0,
  PROP_REALNAME
};

G_DEFINE_TYPE (AboutMeMenuItem, about_me_menu_item, GTK_TYPE_MENU_ITEM)

#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), ABOUT_ME_TYPE_MENU_ITEM, AboutMeMenuItemPrivate))

static void
about_me_menu_item_class_init (AboutMeMenuItemClass *item_class)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (item_class);

  gobject_class->constructor  = about_me_menu_item_constructor;
  gobject_class->set_property = about_me_menu_item_set_property;
  gobject_class->get_property = about_me_menu_item_get_property;

  g_object_class_install_property (gobject_class,
                                   PROP_REALNAME,
                                   g_param_spec_string ("realname",
                                                        "Realname",
                                                        "The \"Realname\" for the user",
                                                        NULL,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (gobject_class, sizeof (AboutMeMenuItemPrivate));
}

static void
about_me_menu_item_init (AboutMeMenuItem *self)
{
  AboutMeMenuItemPrivate *priv = GET_PRIVATE (self);

  priv->label = NULL;
  priv->image = NULL;
  priv->realname = NULL;
}

static void
about_me_menu_item_set_property (GObject         *object,
                                 guint            prop_id,
                                 const GValue    *value,
                                 GParamSpec      *pspec)
{
	AboutMeMenuItem *menu_item = ABOUT_ME_MENU_ITEM (object);
	AboutMeMenuItemPrivate *priv = GET_PRIVATE (menu_item);

	switch (prop_id)
  {
	case PROP_REALNAME:
		g_assert (priv->realname == NULL);
    priv->realname = g_strdup (g_value_get_string (value));
		break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
about_me_menu_item_get_property (GObject         *object,
                                 guint            prop_id,
                                 GValue          *value,
                                 GParamSpec      *pspec)
{
	AboutMeMenuItem *menu_item = ABOUT_ME_MENU_ITEM (object);
	AboutMeMenuItemPrivate *priv = GET_PRIVATE (menu_item);

  switch (prop_id)
  {
	case PROP_REALNAME:
		g_value_set_string (value, priv->realname);
		break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

#define DEFAULT_PIXELS_PER_EM        10.0f

static gdouble
get_pixels_per_em (GtkWidget *widget)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), DEFAULT_PIXELS_PER_EM);

	/* Note: taken from indicator-session */
	GtkStyle * style = gtk_widget_get_style(widget);

	PangoLayout * layout = pango_layout_new(gtk_widget_get_pango_context(widget));
	pango_layout_set_text(layout, "M", -1);
	pango_layout_set_font_description(layout, style->font_desc);

	gint width;
	pango_layout_get_pixel_size(layout, &width, NULL);

	gint point = pango_font_description_get_size(style->font_desc);
	gdouble dpi = gdk_screen_get_resolution(gdk_screen_get_default());

	return ((point * dpi) / 72.0f) / PANGO_SCALE;
}


/* from n-osd */
static GdkPixbuf*
load_icon (const gchar* filename,
           gint         icon_size)
{
	GdkPixbuf*    buffer = NULL;
	GdkPixbuf*    pixbuf = NULL;
	GtkIconTheme* theme  = NULL;
	GError*       error  = NULL;

	/* sanity check */
	g_return_val_if_fail (filename, NULL);

	theme = gtk_icon_theme_get_default ();
	buffer = gtk_icon_theme_load_icon (theme,
                                     filename,
                                     icon_size,
                                     GTK_ICON_LOOKUP_FORCE_SVG |
                                     GTK_ICON_LOOKUP_GENERIC_FALLBACK |
                                     GTK_ICON_LOOKUP_FORCE_SIZE,
                                     &error);
	if (error)
	{
		/*//g_print ("loading icon '%s' caused error: '%s'",
             filename,
             error->message);*/
		g_error_free (error);
		error = NULL;
		pixbuf = NULL;
	}
	else
	{
		/* copy and unref buffer so on an icon-theme change old
		** icons are not kept in memory due to dangling
		** references, this also makes sure we do not need to
		** connect to GtkWidget::style-set signal for the
		** GdkPixbuf we get from gtk_icon_theme_load_icon() */
		pixbuf = gdk_pixbuf_copy (buffer);
		g_object_unref (buffer);
	}

	return pixbuf;
}

gboolean
about_me_menu_item_load_avatar (AboutMeMenuItem *self, const gchar *file)
{
  g_return_val_if_fail (ABOUT_IS_ME_MENU_ITEM (self), FALSE);

  AboutMeMenuItemPrivate *priv = GET_PRIVATE (self);

  g_debug ("loading avatar from file %s", file);

  struct stat buf;
  if (! (g_stat (file, &buf) == 0 && buf.st_size > 0)) {
    g_warning ("%s: not found or empty", file);
    return FALSE;
  }

  if (buf.st_size > 1024*1024) {
    g_warning ("avatar file too large (%lld)", (long long)buf.st_size);
    return FALSE;
  }

  GError *error = NULL;
  int size = get_pixels_per_em (priv->image) * 3;

  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale (file, -1, size, TRUE,
                                                         &error);
  if (pixbuf == NULL) {
    if (error != NULL) {
      g_warning ("Couldn't read file %s: %s", file, error->message);
      g_error_free (error);
    }
    return FALSE;
  }

  gtk_image_set_from_pixbuf (GTK_IMAGE (priv->image), pixbuf);

  g_object_unref (pixbuf);

  return TRUE;
}

static void
image_size_allocate (GtkWidget *widget,
                     GtkAllocation *allocation,
                     gpointer user_data)
{
  gint max = MAX (allocation->width, allocation->height);

  gtk_widget_set_size_request (widget, max, max);
}

static GObject*
about_me_menu_item_constructor (GType                  type,
                                guint                  n_construct_properties,
                                GObjectConstructParam *construct_params)
{
  GObject *object;
  GtkWidget *hbox;
  GtkWidget *align;
  AboutMeMenuItemPrivate *priv;
  object = G_OBJECT_CLASS (about_me_menu_item_parent_class)->constructor (type,
                                                                          n_construct_properties,
                                                                          construct_params);

  priv = GET_PRIVATE (object);

  GtkWidget *frame = gtk_frame_new (NULL);
  gdouble pixels_per_em = get_pixels_per_em (frame);
  GdkPixbuf *pixbuf = load_icon ("stock_person-panel", pixels_per_em * 3);
  if (pixbuf == NULL)
    pixbuf = load_icon ("stock_person", pixels_per_em * 3);
  priv->image = gtk_image_new_from_pixbuf (pixbuf);
  g_signal_connect (frame, "size-allocate", G_CALLBACK (image_size_allocate), NULL);
  gtk_misc_set_padding (GTK_MISC (priv->image), 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), priv->image);

  align = gtk_alignment_new (0, 0.3, 0, 0);
  priv->label = gtk_label_new (priv->realname);
  gtk_misc_set_padding (GTK_MISC (priv->label), 2, 2);
  gtk_container_add (GTK_CONTAINER (align), priv->label);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), align, TRUE, TRUE, DEFAULT_PIXELS_PER_EM);

  gtk_container_add (GTK_CONTAINER (object), hbox);
  gtk_widget_show_all (GTK_WIDGET(object));

  priv->hbox = hbox;
  
  return object;
}

/**
 * about_me_menu_item_new:
 * @realname: the name to display in the new menu item.
 * @returns: a new #AboutMeMenuItem.
 *
 * Creates a new #AboutMeMenuItem with a name.
 **/
GtkWidget*
about_me_menu_item_new (const gchar *realname)
{
	return g_object_new (ABOUT_ME_TYPE_MENU_ITEM,
                       "realname", realname,
                       NULL);
}

#define __ABOUT_ME_MENU_ITEM_C__
