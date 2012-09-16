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

#include "applet-struct.h"
#include "applet-indicator3.h"
#include "indicator-applet3-utils.h"

static void _icon_updated (GObject *pObject, GParamSpec *pParam G_GNUC_UNUSED, CairoDockModuleInstance *myApplet)
{
	g_return_if_fail (GTK_IS_IMAGE (pObject));
	GtkImage *pImage = GTK_IMAGE (pObject);

	gchar *cName = NULL;
	if (! cd_indicator3_update_image (pImage, &cName, myApplet, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE))
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
	g_free (cName);
}

static void _show (GtkWidget *pWidget G_GNUC_UNUSED, gpointer data)
{
	CairoDockModuleInstance *myApplet = data;

	if (myDock && myData.bIsHidden)
	{
		cd_debug ("Re-insert the icon");
		cairo_dock_insert_icon_in_dock (myIcon, myDock, ! CAIRO_DOCK_ANIMATE_ICON);
		///cairo_dock_redraw_container (CAIRO_CONTAINER (myDock)); // dock refresh forced
		myData.bIsHidden = FALSE;
	}
	else
		cd_debug ("It's not possible to re-insert the icon: Hidden = %d (%o)", myData.bIsHidden, myDock);
}

static void _hide (GtkWidget *pWidget G_GNUC_UNUSED, gpointer data)
{
	CairoDockModuleInstance *myApplet = data;

	if (myDock && ! myData.bIsHidden)
	{
		cd_debug ("Detach the icon");
		cairo_dock_detach_icon_from_dock (myIcon, myDock);
		myData.bIsHidden = TRUE;
	}
	else
		cd_debug ("It's not possible to detach the icon: Hidden = %d (%o)", myData.bIsHidden, myDock);
}

void cd_printers_accessible_desc_update (IndicatorObject *pIndicator G_GNUC_UNUSED, IndicatorObjectEntry *pEntry, gpointer data)
{
	const gchar *cDesc = cd_indicator3_get_accessible_desc (pEntry);
	cd_debug ("Get Accessible description: %s", cDesc);
	CairoDockModuleInstance *myApplet = data;
	if (cDesc != NULL && *cDesc != '\0')
		CD_APPLET_SET_NAME_FOR_MY_ICON (cDesc);
	else if (myConfig.defaultTitle != NULL && *myConfig.defaultTitle != '\0')
		CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
	else
		CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
}

void cd_printers_entry_added (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	cd_debug ("Entry Added: %p", pEntry);
	CairoDockModuleInstance *myApplet = data;
	g_return_if_fail (myData.pEntry == NULL && pEntry != NULL); // should not happen... only one entry

	myData.pEntry = pEntry;
	GtkImage *pImage = cd_indicator3_get_image (pEntry);

	g_return_if_fail (pImage != NULL);

	if (! gtk_widget_get_visible (GTK_WIDGET (pImage)))
		_hide (NULL, myApplet);
	else
	{
		_show (NULL, myApplet);
		_icon_updated (G_OBJECT (pImage), NULL, myApplet);
		cd_printers_accessible_desc_update (pIndicator, pEntry, data);
	}

	// signals
	cd_indicator3_notify_image (pImage, G_CALLBACK (_icon_updated), myApplet);

	g_signal_connect (G_OBJECT (pImage), "show", G_CALLBACK (_show), myApplet);
	g_signal_connect (G_OBJECT (pImage), "hide", G_CALLBACK (_hide), myApplet);
}

void cd_printers_entry_removed (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	// should not happen... except at the end.
	cd_debug ("Entry Removed");
	CairoDockModuleInstance *myApplet = data;
	if (pEntry && pEntry->image)
	{
		g_signal_handlers_disconnect_by_func (G_OBJECT (pEntry->image), G_CALLBACK (_icon_updated), myApplet);
		g_signal_handlers_disconnect_by_func (G_OBJECT (pEntry->image), G_CALLBACK (_show), myApplet);
		g_signal_handlers_disconnect_by_func (G_OBJECT (pEntry->image), G_CALLBACK (_hide), myApplet);
	}

	_hide (NULL, myApplet);

	myData.pEntry = NULL;
}

void cd_printers_destroy (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	cd_debug ("Destroy");
	CairoDockModuleInstance *myApplet = data;
	cd_printers_entry_removed (pIndicator, pEntry, data);
}

void cd_printers_reload (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	cd_debug ("Reload");
	g_return_if_fail (pEntry != NULL);
	cd_printers_accessible_desc_update (pIndicator, pEntry, data);
	_icon_updated (G_OBJECT (pEntry->image), NULL, data);
}
