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
#include "applet-generic.h"
#include "applet-backend-alsamixer.h"  // cd_mixer_init_alsa
#include "applet-backend-sound-menu.h"

static void _show_menu (void);
static void (*_stop_parent) (void) = NULL;
static void (*_show_menu_parent) (void)  = NULL;


static void _entry_added (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, GldiModuleInstance *myApplet)
{
	cd_debug ("Entry Added: %p", pEntry);
	g_return_if_fail (myData.pEntry == NULL && pEntry != NULL); // should not happen... only one entry

	myData.pEntry = pEntry;
}

static void _entry_removed (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, GldiModuleInstance *myApplet)
{
	// should not happen... except at the end.
	cd_debug ("Entry Removed");

	// no (more) sound service... now rely on alsa to display the menu/dialog.
	if (myData.pEntry == pEntry) // the same entry as before, we can remove the previous one
	{
		myData.pEntry = NULL;
	}
}


static void _show_menu (void)
{
	GtkMenu *pMenu = NULL;
	if (myData.pEntry)
		pMenu = cd_indicator3_get_menu (myData.pEntry);
	if (pMenu)
	{
		GList *entries = gtk_container_get_children (GTK_CONTAINER (pMenu));
		if (entries)  // if the menu is ok
		{
			CD_APPLET_POPUP_MENU_ON_MY_ICON (GTK_WIDGET (pMenu));
			g_list_free (entries);
		}
		else  // else, the daemon was probaby not launched.
			pMenu = NULL;
	}
	if (!pMenu)  // if no menu, it's maybe because the daemon has not started, or has stopped (the entry is not removed).
	{
		if (_show_menu_parent) _show_menu_parent ();
	}
}


static void _stop (void)
{
	_entry_removed (myData.pIndicator, myData.pEntry, myApplet);
	if (_stop_parent)
		_stop_parent ();
}


void cd_mixer_connect_to_sound_service (void)
{
	// load the indicator (we only want its menu, label and image are set by us).
	myData.pIndicator = cd_indicator3_load (myConfig.cIndicatorName,
		_entry_added,
		_entry_removed,
		NULL,
		NULL,
		myApplet);
	
	// init the backend. we'll use the alsa backend (to get the exact volume and volume changes), and we'll override only the functions we need.
	cd_mixer_init_alsa ();  // alsa backend
	if (myData.pIndicator)
	{
		_stop_parent = myData.ctl.stop;
		myData.ctl.stop = _stop;
		_show_menu_parent = myData.ctl.show_hide;
		myData.ctl.show_hide = _show_menu;  // but with our menu
	}

}
