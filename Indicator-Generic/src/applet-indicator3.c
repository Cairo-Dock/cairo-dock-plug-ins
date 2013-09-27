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
#include "applet-struct.h"
#include "applet-indicator3.h"


static void _icon_updated (GObject *pObject, G_GNUC_UNUSED GParamSpec *pParam, GldiModuleInstance *myApplet)
{
	g_return_if_fail (GTK_IS_IMAGE (pObject));
	GtkImage *pImage = GTK_IMAGE (pObject);

	gchar *cName = NULL;
	if (! cd_indicator3_update_image (pImage, &cName, myApplet, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE))
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
	g_free (cName);
}

static void _accessible_desc_update (G_GNUC_UNUSED IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, GldiModuleInstance *myApplet)
{
	cd_indicator3_accessible_desc_update (pEntry, myConfig.defaultTitle, myApplet);
}

static void _entry_added (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, GldiModuleInstance *myApplet)
{
	cd_debug ("Entry Added: %s", myConfig.cIndicatorName);
	g_return_if_fail (myData.pEntry == NULL && pEntry != NULL); // should not happen...
	// only one entry (or should we support more than one entry => ex: global-menu?)

	myData.pEntry = pEntry;
	GtkImage *pImage = cd_indicator3_get_image (pEntry);

	g_return_if_fail (pImage != NULL);

	// signals
	cd_indicator3_notify_image (pImage, G_CALLBACK (_icon_updated), myApplet);
	cd_indicator3_notify_visibility (pImage, G_CALLBACK (_icon_updated), myApplet);

	cd_indicator3_check_visibility (pImage, myApplet);
	_accessible_desc_update (pIndicator, pEntry, myApplet);
}

static void _entry_removed (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, GldiModuleInstance *myApplet)
{
	// should not happen so often... except at the end.
	cd_debug ("Entry Removed: %s", myConfig.cIndicatorName);
	
	// the same entry as before, we can remove the previous one
	gboolean bHide = FALSE;
	if (myData.pEntry != NULL && myData.pEntry == pEntry) // only if an entry was already added and it was the same that we want to remove
	{
		myData.pEntry = NULL;
		bHide = TRUE;
	}

	// it's not a fake signal
	if (pEntry && pEntry->image)
	{
		g_signal_handlers_disconnect_by_func (G_OBJECT (pEntry->image), G_CALLBACK (_icon_updated), myApplet);
		cd_indicator3_disconnect_visibility (pEntry->image, myApplet, bHide);
	}
}

static void _destroy (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, GldiModuleInstance *myApplet)
{
	_entry_removed (pIndicator, pEntry, myApplet);
}

void cd_indicator_generic_indicator_reload (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	g_return_if_fail (pEntry != NULL);
	_accessible_desc_update (pIndicator, pEntry, data);
	_icon_updated (G_OBJECT (pEntry->image), NULL, data);
}

void cd_indicator_generic_load_one_indicator (GldiModuleInstance *myApplet)
{
	cd_debug ("Load: %s", myConfig.cIndicatorName);
	myData.pIndicator = cd_indicator3_load (myConfig.cIndicatorName,
		_entry_added,
		_entry_removed,
		_accessible_desc_update,
		NULL, // menu show
		myApplet);

	// hide it if there is no entry or no image
	cd_indicator3_hide_if_not_visible (myData.pEntry ? myData.pEntry->image : NULL, myApplet);

	if (! myData.pIndicator)
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
}

void cd_indicator_generic_indicator_stop (GldiModuleInstance *myApplet)
{
	cd_debug ("Stop: %s", myConfig.cIndicatorName);
	// It seems we doesn't need to free the indicator (object and event)
	_destroy (myData.pIndicator, myData.pEntry, myApplet); // remove the connection to signals (menu)

	cd_indicator3_unload (myData.pIndicator, // remove the connection to signals (indicator)
		_entry_added,
		_entry_removed,
		_accessible_desc_update,
		NULL,
		myApplet);
}
