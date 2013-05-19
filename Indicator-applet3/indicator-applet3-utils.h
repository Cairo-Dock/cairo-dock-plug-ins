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

#ifndef __CD_INDICATOR_APPLET3_UTILS__
#define  __CD_INDICATOR_APPLET3_UTILS__

#include <gtk/gtk.h>
#include <cairo-dock.h>
#include <libindicator/indicator-object.h>

/**
 * Use the image of a GtkImage to draw a Cairo-Dock Icon
 *  and give the name of the image if it's possible.
 */
gboolean cd_indicator3_update_image (GtkImage *pImage,
	gchar **cName,
	GldiModuleInstance *myApplet,
	const gchar *cDefaultFile);

/**
 * Connect to the right signal in order to be notified when the image of
 * a GtkImage has been updated
 */
void cd_indicator3_notify_image (GtkImage *pImage, GCallback pCallBack, gpointer data);

/**
 * Update the title of the icon with the description, defaultTitle or the name of the applet
 */
void cd_indicator3_accessible_desc_update (IndicatorObjectEntry *pEntry, const gchar *defaultTitle, gpointer data);

/**
 * Connect to the show/hide signal in order to show or hide the icon.
 */
void cd_indicator3_notify_visibility (GtkImage *pImage, GCallback pCallBack, gpointer data);

/**
 * Disconnect to the show/hide signal and hide the icon if it's needed
 */
void cd_indicator3_disconnect_visibility (GtkImage *pImage, GldiModuleInstance *myApplet, gboolean bHide);

/**
 * Check if the widget (pImage) exists and if it's visible.
 * If no, hide the icon and return TRUE
 */
gboolean cd_indicator3_hide_if_not_visible (GtkImage *pImage, GldiModuleInstance *myApplet);

/**
 * Check the visibility of a widget and then show/hide the icon
 */
void cd_indicator3_check_visibility (GtkImage *pImage, GldiModuleInstance *myApplet);

#endif /* __CD_INDICATOR_APPLET3_UTILS__ */
