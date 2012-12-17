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

#ifndef __APPLET_NOTES__
#define  __APPLET_NOTES__

#include "tomboy-struct.h"


GList *cd_tomboy_find_notes_with_tag (const gchar *cTag);

GList *cd_tomboy_find_notes_with_contents (const gchar **cContents);

GList *cd_tomboy_find_note_for_today (void);
GList *cd_tomboy_find_note_for_this_week (void);
GList *cd_tomboy_find_note_for_next_week (void);


void cd_notes_show_note (const gchar *cNoteID);

void cd_notes_delete_note (const gchar *cNoteID);

gchar *cd_notes_create_note (const gchar *cTitle);

void cd_notes_run_manager (void);

void cd_notes_start (void);

void cd_notes_stop (void);


void cd_notes_store_load_notes (GList *pNotes);

void cd_notes_store_add_note (CDNote *pNote);

void cd_notes_store_remove_note (const gchar *cNoteID);

void cd_notes_store_update_note (CDNote *pUpdatedNote);

void cd_notes_free_note (CDNote *pNote);


#endif
