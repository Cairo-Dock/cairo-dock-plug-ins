#ifndef __TOMBOY_DBUS__
#define  __TOMBOY_DBUS__

#include <dbus/dbus-glib.h>
#include "tomboy-struct.h"


gboolean dbus_connect_to_bus(void);
void dbus_disconnect_from_bus (void);
void dbus_detect_tomboy(void);

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
