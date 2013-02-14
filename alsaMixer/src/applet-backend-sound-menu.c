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
#include "applet-backend-alsamixer.h"  // cd_mixer_init_alsa + stop (fallback alsa backend)
#include "applet-backend-sound-menu.h"


static void _entry_added (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	cd_debug ("Entry Added: %p", pEntry);
	g_return_if_fail (myData.pEntry == NULL && pEntry != NULL); // should not happen... only one entry

	myData.pEntry = pEntry;
}

static void _entry_removed (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data)
{
	// should not happen... except at the end.
	cd_debug ("Entry Removed");

	// no (more) sound service... now rely on alsa.
	if (myData.pEntry == pEntry) // the same entry as before, we can remove the previous one
		myData.pEntry = NULL;
}

static void _show_menu (void)
{
	GtkMenu *pMenu = cd_indicator3_get_menu (myData.pEntry);
	if (pMenu)
		cairo_dock_popup_menu_on_icon (GTK_WIDGET (pMenu), myIcon, myContainer);
	else // if problem
	{
		cd_mixer_init_alsa ();
		myData.ctl.show_hide ();
	}
}

static void _stop (void)
{
	_entry_removed (myData.pIndicator, myData.pEntry, myApplet);
	cd_mixer_stop_alsa ();
}

void cd_mixer_connect_to_sound_service (void)
{
	myData.pIndicator = cd_indicator3_load (myConfig.cIndicatorName,
		_entry_added,
		_entry_removed,
		NULL,
		NULL,
		myApplet);

	cd_mixer_init_alsa ();  // alsa backend
	if (myData.pIndicator)
	{
		myData.ctl.show_hide = _show_menu; // but with our menu
		myData.ctl.stop = _stop;
	}

}
