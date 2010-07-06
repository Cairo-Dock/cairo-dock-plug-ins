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

#ifndef __TOMBOY_DBUS__
#define  __TOMBOY_DBUS__

#include <dbus/dbus-glib.h>
#include "tomboy-struct.h"


gboolean dbus_connect_to_bus(void);
void dbus_disconnect_from_bus (void);
void dbus_detect_tomboy(void);
void dbus_detect_tomboy_async (CairoDockModuleInstance *myApplet);

void onDeleteNote(DBusGProxy *proxy,const gchar *note_uri, /*const gchar *note_title, */gpointer data);
void onAddNote(DBusGProxy *proxy,const gchar *note_uri, gpointer data);
void onChangeNoteList(DBusGProxy *proxy,const gchar *note_name, gpointer data);
gboolean cd_tomboy_check_deleted_notes (gpointer data);

gchar *getNoteTitle (const gchar *note_name);
gchar *getNoteContent (const gchar *note_name);
void getAllNotes (void);
gboolean cd_tomboy_load_notes (void);
void free_all_notes (void);

gchar *addNote(gchar *note_name);
void deleteNote(gchar *note_title);
void showNote(gchar *note_id);


GList *cd_tomboy_find_notes_with_tag (gchar *cTag);

GList *cd_tomboy_find_notes_with_contents (gchar **cContents);

GList *cd_tomboy_find_note_for_today (void);
GList *cd_tomboy_find_note_for_this_week (void);
GList *cd_tomboy_find_note_for_next_week (void);

#endif
