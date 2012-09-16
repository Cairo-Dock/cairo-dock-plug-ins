/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
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

#include "indicator-applet3-utils.h"

static const gchar * _get_name_from_icon_name (GtkImage *pImage)
{
	const gchar *cName;
	gtk_image_get_icon_name (pImage, &cName, NULL);
	cd_debug ("Get icon name: %s", cName);
	return cName;
}

static gchar * _get_name_from_gicon (GtkImage *pImage)
{
	GIcon *pIcon;
	gtk_image_get_gicon (pImage, &pIcon, NULL);
	g_return_val_if_fail (pIcon != NULL, NULL);

	gchar *cName = g_icon_to_string (pIcon);
	cd_debug ("Get GIcon: %s", cName);
	return cName;
}

static gboolean _set_new_image_pixbuf (GtkImage *pImage)
{
	GdkPixbuf *pPixbuf = gtk_image_get_pixbuf (pImage);
	g_return_val_if_fail (pPixbuf != NULL, FALSE);

	cd_debug ("Icon Pixbuf: %p\n", pPixbuf);
	gdouble fWidth, fHeight;
	cairo_surface_t *pSurface = cairo_dock_create_surface_from_pixbuf (pPixbuf,
		1.,
		myIcon->iImageWidth, myIcon->iImageHeight,
		0,
		&fWidth, &fHeight,
		NULL, NULL);
	cd_debug ("Pixbuf: %fx%f", fWidth, fHeight);

	CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);

	return TRUE;
}

const gchar *_get_image_stock (GtkImage *pImage)
{
	gchar *cName;
	gtk_image_get_stock (pImage, &cName, NULL);
	cd_debug ("Get stock: %s", cName);
	return cName;
}

static gboolean _set_image_on_icon (const gchar *cName, CairoDockModuleInstance *myApplet, const gchar *cDefaultFile)
{
	if (! cName)
		return FALSE;
	cairo_dock_set_image_on_icon_with_default (myDrawContext,
		cName, myIcon, myContainer, cDefaultFile);
	return TRUE;
}

gboolean cd_indicator3_update_image (GtkImage *pImage, gchar **cName, CairoDockModuleInstance *myApplet, const gchar *cDefaultFile)
{
	GtkImageType iType = gtk_image_get_storage_type (pImage);
	cd_debug ("Icon updated: type %d", iType);
	switch (iType)
	{
		case GTK_IMAGE_ICON_NAME:
			*cName = g_strdup (_get_name_from_icon_name (pImage));
			return _set_image_on_icon (*cName, myApplet, cDefaultFile);
		break;
		case GTK_IMAGE_GICON:
			*cName = _get_name_from_gicon (pImage);
			return _set_image_on_icon (*cName, myApplet, cDefaultFile);
		break;
		case GTK_IMAGE_PIXBUF:
			return _set_new_image_pixbuf (pImage);
		break;
		case GTK_IMAGE_STOCK:
			*cName = g_strdup (_get_image_stock (pImage));
			return _set_image_on_icon (*cName, myApplet, cDefaultFile);
		break;
		case GTK_IMAGE_EMPTY:
			cd_debug ("No image...");
			return FALSE;
		break;
		case GTK_IMAGE_ANIMATION:
		case GTK_IMAGE_ICON_SET:
		default:
			cd_warning ("This icon type (%d) is not (yet) supported", iType);
			return FALSE;
		break;
	}
	return TRUE;
}

void cd_indicator3_notify_image (GtkImage *pImage, GCallback pCallBack, gpointer data)
{
	GtkImageType iType = gtk_image_get_storage_type (pImage);
	cd_debug ("Icon notify: type %d", iType);
	switch (iType)
	{
		case GTK_IMAGE_ICON_NAME:
			g_signal_connect (G_OBJECT (pImage), "notify::icon-name", pCallBack, data);
		break;
		case GTK_IMAGE_GICON:
			g_signal_connect (G_OBJECT (pImage), "notify::gicon", pCallBack, data);
		break;
		case GTK_IMAGE_PIXBUF:
			g_signal_connect (G_OBJECT (pImage), "notify::pixbuf", pCallBack, data);
		break;
		case GTK_IMAGE_STOCK:
			g_signal_connect (G_OBJECT (pImage), "notify::stock", pCallBack, data);
		break;
		case GTK_IMAGE_EMPTY:
			cd_debug ("No image (type is empty)... Connect to all signals");
			g_signal_connect (G_OBJECT (pImage), "notify::icon-name", pCallBack, data);
			g_signal_connect (G_OBJECT (pImage), "notify::gicon", pCallBack, data);
			g_signal_connect (G_OBJECT (pImage), "notify::pixbuf", pCallBack, data);
			g_signal_connect (G_OBJECT (pImage), "notify::stock", pCallBack, data);
		break;
		case GTK_IMAGE_ANIMATION:
		case GTK_IMAGE_ICON_SET:
		default:
			cd_warning ("This icon type (%d) is not (yet) supported", iType);
		break;
	}
}
