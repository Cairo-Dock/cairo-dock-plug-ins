#ifndef __TOMBOY_DBUS__
#define  __TOMBOY_DBUS__

#include <dbus/dbus-glib.h>
#include "tomboy-struct.h"


gboolean dbus_get_dbus (void);
void dbus_connect_to_bus(void);
void dbus_disconnect_from_bus (void);
void dbus_detect_tomboy(void);

void onDeleteNote(DBusGProxy *proxy,const gchar *note_uri, const gchar *note_title, gpointer data);
void onAddNote(DBusGProxy *proxy,const gchar *note_uri, gpointer data);
void onChangeNoteList(DBusGProxy *proxy,const gchar *note_name, gpointer data);

void registerNote(gchar *uri);
void getAllNotes(void);
gchar *getNoteTitle(gchar *note_name);
gchar *addNote(gchar *note_name);
void showNote(gchar *note_id);

void free_note (TomBoyNote *pNote);
void free_all_notes (void);
#endif
