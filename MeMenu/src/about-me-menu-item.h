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

#ifndef __ABOUT_ME_MENU_ITEM_H__
#define __ABOUT_ME_MENU_ITEM_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ABOUT_ME_TYPE_MENU_ITEM         (about_me_menu_item_get_type ())
#define ABOUT_ME_MENU_ITEM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), ABOUT_ME_TYPE_MENU_ITEM, AboutMeMenuItem))
#define ABOUT_ME_MENU_ITEM_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST ((c), ABOUT_ME_TYPE_MENU_ITEM, AboutMeMenuItemClass))
#define ABOUT_IS_ME_MENU_ITEM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), ABOUT_ME_TYPE_MENU_ITEM))
#define ABOUT_IS_ME_MENU_ITEM_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE ((c), ABOUT_ME_TYPE_MENU_ITEM))
#define ABOUT_ME_MENU_ITEM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), ABOUT_ME_TYPE_MENU_ITEM, AboutMeMenuItemClass))


typedef struct _AboutMeMenuItem        AboutMeMenuItem;
typedef struct _AboutMeMenuItemClass   AboutMeMenuItemClass;
typedef struct _AboutMeMenuItemPrivate AboutMeMenuItemPrivate;

struct _AboutMeMenuItem
{
	GtkMenuItem parent_instance;

	AboutMeMenuItemPrivate *priv;
};

struct _AboutMeMenuItemClass
{
	GtkMenuItemClass parent_class;
};


GType	   about_me_menu_item_get_type            (void) G_GNUC_CONST;
GtkWidget *about_me_menu_item_new                 (const gchar *name);
gboolean   about_me_menu_item_load_avatar         (AboutMeMenuItem *self, const gchar *file);

G_END_DECLS

#endif /* __ABOUT_ME_MENU_ITEM_H__ */
