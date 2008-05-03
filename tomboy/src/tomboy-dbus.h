#ifndef __TOMBOY_DBUS__
#define  __TOMBOY_DBUS__

#include <dbus/dbus-glib.h>
#include "tomboy-struct.h"


gboolean dbus_connect_to_bus(void);
void dbus_disconnect_from_bus (void);
void dbus_detect_tomboy(void);

void onDeleteNote(DBusGProxy *proxy,const gchar *note_uri, const gchar *note_title, gpointer data);
void onAddNote(DBusGProxy *proxy,const gchar *note_uri, gpointer data);
void onChangeNoteList(DBusGProxy *proxy,const gchar *note_name, gpointer data);
gboolean cd_tomboy_check_deleted_notes (gpointer data);

void reload_all_notes (void);
gchar *getNoteTitle (const gchar *note_name);
void getAllNotes (void);
void free_all_notes (void);

gchar *addNote(gchar *note_name);
void deleteNote(gchar *note_title);
void showNote(gchar *note_id);

#endif
