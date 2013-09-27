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
#include "indicator-applet3.h"

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

	gchar *cName = NULL;
	if (G_IS_THEMED_ICON (pIcon))
	{
		const gchar * const *cFileNames = g_themed_icon_get_names (G_THEMED_ICON (pIcon));
		for (int i = 0; cFileNames[i] != NULL && cName == NULL; i++)
		{
			gchar *path = cairo_dock_search_icon_s_path (cFileNames[i], CAIRO_DOCK_DEFAULT_ICON_SIZE);
			if (path)
			{
				g_free (path);
				cName = g_strdup (cFileNames[i]);
			}
		}
		cd_debug ("GIcon: it's a GThemedIcon, found: %s", cName);
	}
	else if (G_IS_FILE_ICON (pIcon))
	{
		GFile *pFile = g_file_icon_get_file (G_FILE_ICON (pIcon));
		cName = g_file_get_basename (pFile);
		cd_debug ("GIcon: it's a GFileIcon, found: %s", cName);
	}
	else
		return g_icon_to_string (pIcon);

	return cName;
}

static gboolean _update_image_from_pixbuf (gpointer *data)
{
	GdkPixbuf *pPixbuf = data[0];
	GldiModuleInstance *myApplet = data[1];
	g_free (data);

	cd_debug ("Icon Pixbuf: %p", pPixbuf);
	gdouble fWidth, fHeight;
	int iWidth, iHeight;
	cairo_dock_get_icon_extent (myIcon, &iWidth, &iHeight);
	cairo_surface_t *pSurface = cairo_dock_create_surface_from_pixbuf (pPixbuf,
		1.,
		iWidth, iHeight,
		0,
		&fWidth, &fHeight,
		NULL, NULL);
	cd_debug ("Pixbuf: %fx%f", fWidth, fHeight);

	CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);

	cairo_surface_destroy (pSurface);
	g_object_unref (pPixbuf); // was ref by ourself

	return FALSE;
}

static gboolean _set_new_image_pixbuf (GtkImage *pImage, GldiModuleInstance *myApplet)
{
	GdkPixbuf *pPixbuf = gtk_image_get_pixbuf (pImage);
	g_return_val_if_fail (pPixbuf != NULL, FALSE);

	g_object_ref (pPixbuf); // to be sure that it will be available after the delay
	gpointer *data = g_new (gpointer, 2);
	data[0] = pPixbuf;
	data[1] = myApplet;

	cd_debug ("Icon Pixbuf: %p - add delay: 125ms", pPixbuf);

	/* Add short delay: most of the time, we receive a GdkPixbuf but it seems
	 *  the image is not fully loaded: it has the right dimensions, rowstride
	 *  and the number of bytes seem ok but the icon is fully transparent.
	 * Unfortunately, we don't have the GdkPixbufLoader structure and we can't
	 *  be connected to the "closed" signal...
	 * => not (easily) possible to detect if the image looks fine and impossible
	 *  to be notified when the image is available.
	 */
	g_timeout_add (200, (GSourceFunc)_update_image_from_pixbuf, data);

	return TRUE;
}

const gchar *_get_image_stock (GtkImage *pImage)
{
	gchar *cName;
	gtk_image_get_stock (pImage, &cName, NULL);
	cd_debug ("Get stock: %s", cName);
	return cName;
}

static gboolean _set_image_on_icon (const gchar *cName, GldiModuleInstance *myApplet, const gchar *cDefaultFile)
{
	if (! cName)
		return FALSE;
	cairo_dock_set_image_on_icon_with_default (myDrawContext,
		cName, myIcon, myContainer, cDefaultFile);
	return TRUE;
}

gboolean cd_indicator3_update_image (GtkImage *pImage, gchar **cName, GldiModuleInstance *myApplet, const gchar *cDefaultFile)
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
			return _set_new_image_pixbuf (pImage, myApplet);
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
	gboolean bResult = FALSE;
	GtkImageType iType = gtk_image_get_storage_type (pImage);
	cd_debug ("Icon notify: type %d", iType);
	switch (iType)
	{
		case GTK_IMAGE_ICON_NAME:
			g_signal_connect (G_OBJECT (pImage), "notify::icon-name", pCallBack, data);
			g_signal_emit_by_name (G_OBJECT (pImage), "notify::icon-name", NULL, &bResult);
		break;
		case GTK_IMAGE_GICON:
			g_signal_connect (G_OBJECT (pImage), "notify::gicon", pCallBack, data);
			g_signal_emit_by_name (G_OBJECT (pImage), "notify::gicon", NULL, &bResult);
		break;
		case GTK_IMAGE_PIXBUF:
			g_signal_connect (G_OBJECT (pImage), "notify::pixbuf", pCallBack, data);
			g_signal_emit_by_name (G_OBJECT (pImage), "notify::pixbuf", NULL, &bResult);
		break;
		case GTK_IMAGE_STOCK:
			g_signal_connect (G_OBJECT (pImage), "notify::stock", pCallBack, data);
			g_signal_emit_by_name (G_OBJECT (pImage), "notify::stock", NULL, &bResult);
		break;
		case GTK_IMAGE_EMPTY:
			cd_debug ("No image (type is empty)...");
			// we will be notified later
		break;
		case GTK_IMAGE_ANIMATION:
		case GTK_IMAGE_ICON_SET:
		default:
			cd_warning ("This icon type (%d) is not (yet) supported", iType);
		break;
	}
}

void cd_indicator3_accessible_desc_update (IndicatorObjectEntry *pEntry, const gchar *defaultTitle, GldiModuleInstance *myApplet)
{
	const gchar *cDesc = cd_indicator3_get_accessible_desc (pEntry);
	cd_debug ("Get Accessible description: %s", cDesc);
	if (cDesc != NULL && *cDesc != '\0')
		CD_APPLET_SET_NAME_FOR_MY_ICON (cDesc);
	else if (defaultTitle != NULL && *defaultTitle != '\0')
		CD_APPLET_SET_NAME_FOR_MY_ICON (defaultTitle);
	else
		CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
}


static void _show (G_GNUC_UNUSED GtkWidget *pWidget, gpointer data)
{
	GldiModuleInstance *myApplet = data;
	
	if (myDock)
	{
		cd_debug ("Re-insert the icon");
		gldi_icon_insert_in_container (myIcon, myContainer, ! CAIRO_DOCK_ANIMATE_ICON);
	}
	else
		cd_debug ("It's not possible to re-insert the icon (%p)", myApplet);
}

static void _hide (G_GNUC_UNUSED GtkWidget *pWidget, gpointer data)
{
	GldiModuleInstance *myApplet = data;

	if (myDock)
	{
		cd_debug ("Detach the icon");
		gldi_icon_detach (myIcon);
	}
	else
		cd_debug ("It's not possible to detach the icon (%p)", myApplet);
}

void cd_indicator3_notify_visibility (GtkImage *pImage, GCallback pCallBack, gpointer data)
{
	GldiModuleInstance *myApplet = data;

	g_signal_connect (G_OBJECT (pImage), "show", G_CALLBACK (_show), myApplet);
	g_signal_connect (G_OBJECT (pImage), "hide", G_CALLBACK (_hide), myApplet);
}

gboolean cd_indicator3_hide_if_not_visible (GtkImage *pImage, GldiModuleInstance *myApplet)
{
	if (pImage == NULL || ! gtk_widget_get_visible (GTK_WIDGET (pImage)))
	{
		_hide (NULL, myApplet);
		return TRUE;
	}
	return FALSE;
}

void cd_indicator3_check_visibility (GtkImage *pImage, GldiModuleInstance *myApplet)
{
	if (! cd_indicator3_hide_if_not_visible (pImage, myApplet))
		_show (NULL, myApplet);
		// _image_update (G_OBJECT (pImage), NULL, myApplet);
		// cd_printers_accessible_desc_update (pIndicator, pEntry, data);
}

void cd_indicator3_disconnect_visibility (GtkImage *pImage, GldiModuleInstance *myApplet, gboolean bHide)
{
	g_signal_handlers_disconnect_by_func (G_OBJECT (pImage), G_CALLBACK (_show), myApplet);
	g_signal_handlers_disconnect_by_func (G_OBJECT (pImage), G_CALLBACK (_hide), myApplet);

	if (bHide)
		_hide (NULL, myApplet);
}
