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


#ifndef __APPLET_SEARCH__
#define  __APPLET_SEARCH__


#include <cairo-dock.h>
#include "applet-struct.h"


gboolean cd_do_fill_default_entry (CDEntry *pEntry);


void cd_do_launch_backend (CDBackend *pBackend);
void cd_do_launch_all_backends (void);

void cd_do_stop_backend (CDBackend *pBackend);
void cd_do_stop_all_backends (void);

void cd_do_free_backend (CDBackend *pBackend);
void cd_do_free_all_backends (void);


void cd_do_append_entries_to_listing (GList *pEntries, gint iNbEntries);

void cd_do_remove_entries_from_listing (CDBackend *pBackend);

int cd_do_filter_entries (GList *pEntries, gint iNbEntries);



void cd_do_activate_filter_option (int iNumOption);


GList* cd_do_list_main_sub_entry (CDEntry *pEntry, int *iNbSubEntries);

void cd_do_show_current_sub_listing (void);

void cd_do_show_previous_listing (void);


void cd_do_filter_current_listing (void);


#endif
