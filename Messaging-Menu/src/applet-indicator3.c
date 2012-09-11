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

void _check_demanding_attention (const gchar *cName, const gchar *cAnimationName)
{
	if (g_str_has_prefix (cName, "indicator-messages-new"))
		CD_APPLET_DEMANDS_ATTENTION (cAnimationName, 60);
	else
		CD_APPLET_STOP_DEMANDING_ATTENTION;
}

static void _icon_updated (GObject *pObject, GParamSpec *pParam G_GNUC_UNUSED, CairoDockModuleInstance *myApplet)
{
	g_return_if_fail (GTK_IS_IMAGE (pObject));
	GtkImage *pImage = GTK_IMAGE (pObject);

	gchar *cName = NULL;
	if (! cd_indicator3_update_image (pImage, &cName, myApplet, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE))
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;

	if (cName)
	{
		_check_demanding_attention (cName, myConfig.cAnimationName);
		g_free (cName);
	}
}

void cd_messaging_accessible_desc_update (IndicatorObject *pIndicator G_GNUC_UNUSED, IndicatorObjectEntry *pEntry, gpointer data)
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

void cd_messaging_entry_added (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	cd_debug ("Entry Added: %p", pEntry);
	CairoDockModuleInstance *myApplet = data;
	g_return_if_fail (myData.pEntry == NULL); // should not happen... only one entry

	myData.pEntry = pEntry;

	// get the icon and the label
	_icon_updated (G_OBJECT (pEntry->image), NULL, myApplet);
	// we have a GtkImage but we just need its name: connect to the signal.
	// It seems we can have a gicon or a icon-name.
	g_signal_connect (G_OBJECT (pEntry->image),"notify::gicon", G_CALLBACK (_icon_updated), myApplet);
	g_signal_connect (G_OBJECT (pEntry->image),"notify::icon-name", G_CALLBACK (_icon_updated), myApplet);
	cd_messaging_accessible_desc_update (pIndicator, pEntry, data);
}

void cd_messaging_entry_removed (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	// should not happen... except at the end.
	cd_debug ("Entry Removed");
	CairoDockModuleInstance *myApplet = data;
	if (pEntry && pEntry->image)
		g_signal_handlers_disconnect_by_func (G_OBJECT (pEntry->image), G_CALLBACK (_icon_updated), myApplet);

	myData.pEntry = NULL;
}

void cd_messaging_destroy (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	cd_debug ("Destroy");
	CairoDockModuleInstance *myApplet = data;
	cd_messaging_entry_removed (pIndicator, pEntry, data);
}

void cd_messaging_reload (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	cd_debug ("Reload");
	g_return_if_fail (pEntry != NULL);
	cd_messaging_accessible_desc_update (pIndicator, pEntry, data);
	_icon_updated (G_OBJECT (pEntry->image), NULL, data);
}
