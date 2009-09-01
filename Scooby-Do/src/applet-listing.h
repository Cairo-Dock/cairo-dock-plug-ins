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


#ifndef __APPLET_LISTING__
#define  __APPLET_LISTING__


#include <cairo-dock.h>
#include "applet-struct.h"


void cd_do_free_entry (CDEntry *pEntry);
void cd_do_free_listing_backup (CDListingBackup *pBackup);

CDListing *cd_do_create_listing (void);
void cd_do_destroy_listing (CDListing *pListing);


gboolean cd_do_update_listing_notification (gpointer pUserData, CDListing *pListing, gboolean *bContinueAnimation);
gboolean cd_do_render_listing_notification (gpointer pUserData, CDListing *pListing, cairo_t *pCairoContext);

void cd_do_show_listing (void);

void cd_do_hide_listing (void);

void cd_do_load_entries_into_listing (GList *pEntries, int iNbEntries);

void cd_do_fill_listing_entries (CDListing *pListing);


void cd_do_select_prev_next_entry_in_listing (gboolean bNext);
void cd_do_select_prev_next_page_in_listing (gboolean bNext);
void cd_do_select_last_first_entry_in_listing (gboolean bLast);
void cd_do_select_nth_entry_in_listing (int iNumEntry);

void cd_do_rewind_current_entry (void);


void cd_do_set_status (const gchar *cStatus);
void cd_do_set_status_printf (const gchar *cStatusFormat, ...);


#endif
