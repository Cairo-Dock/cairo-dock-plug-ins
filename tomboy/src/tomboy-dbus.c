#include <string.h>
#include <dbus/dbus-glib.h>

#include "tomboy-struct.h"
#include "tomboy-draw.h"
#include "tomboy-struct.h"
#include "tomboy-dbus.h"

static DBusGConnection *dbus_connexion;
static DBusGProxy *dbus_proxy_dbus;
static DBusGProxy *dbus_proxy_tomboy;


CD_APPLET_INCLUDE_MY_VARS

gboolean dbus_get_dbus (void)
{
	cd_message ("Connexion au bus ... ");
	dbus_connexion = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	
	if(!dbus_connexion)
	{
		cd_message (" echouee");
		return FALSE;
	}
	else
	{
		cd_message (" reussie");
		
		dbus_proxy_tomboy = dbus_g_proxy_new_for_name (
			dbus_connexion,
			"org.gnome.Tomboy",
			"/org/gnome/Tomboy/RemoteControl",
			"org.gnome.Tomboy.RemoteControl"
		);
		
		dbus_proxy_dbus = dbus_g_proxy_new_for_name (
			dbus_connexion,
			"org.freedesktop.DBus",
			"/",
			"org.freedesktop.DBus"
		);
		
		
		dbus_g_proxy_add_signal(dbus_proxy_tomboy, "NoteDeleted",  // aie, ce signal n'a pas l'air d'exister dans la version Gutsy de tomboy (No marshaller for signature of signal 'NoteDeleted') :-(
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(dbus_proxy_tomboy, "NoteAdded",
			G_TYPE_STRING,
			G_TYPE_INVALID);
		dbus_g_proxy_add_signal(dbus_proxy_tomboy, "NoteSaved",
			G_TYPE_STRING,
			G_TYPE_INVALID);
		
		return TRUE;
	}
}


void dbus_connect_to_bus (void)
{
	cd_message ("");
	
	dbus_g_proxy_connect_signal(dbus_proxy_tomboy, "NoteDeleted",
		G_CALLBACK(onDeleteNote), NULL, NULL);
	dbus_g_proxy_connect_signal(dbus_proxy_tomboy, "NoteAdded",
		G_CALLBACK(onAddNote), NULL, NULL);
	dbus_g_proxy_connect_signal(dbus_proxy_tomboy, "NoteSaved",
		G_CALLBACK(onChangeNoteList), NULL, NULL);
	
}

void dbus_disconnect_from_bus (void)
{
	cd_message ("");
	
	dbus_g_proxy_disconnect_signal(dbus_proxy_tomboy, "NoteDeleted",
		G_CALLBACK(onDeleteNote), NULL);
	dbus_g_proxy_disconnect_signal(dbus_proxy_tomboy, "NoteAdded",
		G_CALLBACK(onAddNote), NULL);
	dbus_g_proxy_disconnect_signal(dbus_proxy_tomboy, "NoteSaved",
		G_CALLBACK(onChangeNoteList), NULL);
	
}

void dbus_detect_tomboy(void)
{
	cd_message ("");
	gchar **name_list = NULL;
	
	myData.opening = FALSE;
	if(dbus_g_proxy_call (dbus_proxy_dbus, "ListNames", NULL,
		G_TYPE_INVALID,
		G_TYPE_STRV,
		&name_list,
		G_TYPE_INVALID))
	{
		cd_message ("tomboy : detection du service...");
		int i;
		for (i = 0; name_list[i] != NULL; i ++)
		{
			if (strcmp (name_list[i], "org.gnome.Tomboy") == 0)
			{
				myData.opening = TRUE;
				break;
			}
		}
	}
	g_strfreev (name_list);
}

void onDeleteNote(DBusGProxy *proxy,const gchar *note_uri, const gchar *note_title, gpointer data)
{
	cd_message ("");
	reload_all_notes ();
}

void onAddNote(DBusGProxy *proxy,const gchar *note_uri, gpointer data)
{
	cd_message ("");
	registerNote(note_uri);
	update_icon();
}

void onChangeNoteList(DBusGProxy *proxy,const gchar *note_name, gpointer data)
{
	cd_message ("");
	reload_all_notes ();
}

void reload_all_notes (void)
{
	cd_message ("");
	getAllNotes();
	update_icon();
}

gchar *getNoteTitle(gchar *note_name)
{
	cd_debug("tomboy : Chargement du titre : %s",note_name);
	gchar *note_title = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetNoteTitle", NULL,
		G_TYPE_STRING, note_name,
		G_TYPE_INVALID,
		G_TYPE_STRING,&note_title,
		G_TYPE_INVALID);
	
	return note_title;
}

void registerNote(gchar *uri)
{
	myData.countNotes++;
	TomBoyNote *pNote = g_new0 (TomBoyNote, 1);
	pNote->name = g_strdup (uri);
	cd_message("tomboy : Enregistrement de la note %s",pNote->name);
	pNote->title = getNoteTitle(uri);
	myData.noteList = g_list_prepend (myData.noteList, pNote);
}

void getAllNotes(void)
{
	cd_message("tomboy : getAllNotes");
	
	free_all_notes ();
	
	gchar **note_list = NULL;
	myData.countNotes = 0;
	if(dbus_g_proxy_call (dbus_proxy_tomboy, "ListAllNotes", NULL,
		G_TYPE_INVALID,
		G_TYPE_STRV,&note_list,
		G_TYPE_INVALID))
	{
		cd_message ("tomboy : Liste des notes...");
		int i;
		for (i = 0; note_list[i] != NULL; i ++)
		{
			registerNote(note_list[i]);
		}
	}
	g_strfreev (note_list);
}

gchar *addNote(gchar *note_title)
{
	cd_debug("tomboy : Nouvelle note : %s",note_title);
	gchar *note_name = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "CreateNamedNote", NULL,
		G_TYPE_STRING, note_title,
		G_TYPE_INVALID,
		G_TYPE_STRING,&note_name,
		G_TYPE_INVALID);
	return note_name;
}

void deleteNote(gchar *note_title)
{
	cd_debug("tomboy : Suppression note : %s",note_title);
	gboolean bDelete = TRUE;
	dbus_g_proxy_call (dbus_proxy_tomboy, "DeleteNote", NULL,
		G_TYPE_STRING, note_title,
		G_TYPE_INVALID,
		G_TYPE_BOOLEAN,&bDelete,
		G_TYPE_INVALID);
}

void showNote(gchar *note_name)
{
	cd_debug("tomboy : Afficher une note : %s",note_name);
	dbus_g_proxy_call (dbus_proxy_tomboy, "DisplayNote", NULL,
		G_TYPE_STRING, note_name,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
}

void free_note (TomBoyNote *pNote)
{
	if (pNote == NULL)
		return ;
	g_free (pNote->name);
	g_free (pNote->title);
	g_free (pNote);
}

void free_all_notes (void)
{
	TomBoyNote *pNote;
	GList *pElement;
	for (pElement = myData.noteList; pElement != NULL; pElement = pElement->next)
	{
		pNote = pElement->data;
		free_note(pNote);
	}
	g_list_free (myData.noteList);
	myData.noteList = NULL;
	myData.countNotes = 0;
}
