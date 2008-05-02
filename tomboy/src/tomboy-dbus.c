#include <string.h>
#include <dbus/dbus-glib.h>

#include "tomboy-struct.h"
#include "tomboy-draw.h"
#include "tomboy-dbus.h"

static DBusGProxy *dbus_proxy_tomboy = NULL;


CD_APPLET_INCLUDE_MY_VARS


gboolean dbus_connect_to_bus (void)
{
	cd_message ("");
	if (cairo_dock_bdus_is_enabled ())
	{
		dbus_proxy_tomboy = cairo_dock_create_new_dbus_proxy (
			"org.gnome.Tomboy",
			"/org/gnome/Tomboy/RemoteControl",
			"org.gnome.Tomboy.RemoteControl"
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
		
		dbus_g_proxy_connect_signal(dbus_proxy_tomboy, "NoteDeleted",
			G_CALLBACK(onDeleteNote), NULL, NULL);
		dbus_g_proxy_connect_signal(dbus_proxy_tomboy, "NoteAdded",
			G_CALLBACK(onAddNote), NULL, NULL);
		dbus_g_proxy_connect_signal(dbus_proxy_tomboy, "NoteSaved",
			G_CALLBACK(onChangeNoteList), NULL, NULL);
		return TRUE;
	}
	return FALSE;
}

void dbus_disconnect_from_bus (void)
{
	cd_message ("");
	if (dbus_proxy_tomboy == NULL)
		return ;
	
	dbus_g_proxy_disconnect_signal(dbus_proxy_tomboy, "NoteDeleted",
		G_CALLBACK(onDeleteNote), NULL);
	dbus_g_proxy_disconnect_signal(dbus_proxy_tomboy, "NoteAdded",
		G_CALLBACK(onAddNote), NULL);
	dbus_g_proxy_disconnect_signal(dbus_proxy_tomboy, "NoteSaved",
		G_CALLBACK(onChangeNoteList), NULL);
	
	g_object_unref (dbus_proxy_tomboy);
	dbus_proxy_tomboy = NULL;
	
}

void dbus_detect_tomboy(void)
{
	cd_message ("");
	myData.opening = cairo_dock_dbus_detect_application ("org.gnome.Tomboy");
}


static Icon *_cd_tomboy_find_note_from_name (const gchar *cNoteURI)
{
	Icon *pIcon = NULL;
	GList *pList = (myDock ? (myIcon->pSubDock != NULL ? myIcon->pSubDock->icons : NULL) : myDesklet->icons);
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (strcmp (cNoteURI, pIcon->acCommand) == 0)
			return pIcon;
	}
	
	return NULL;
}

void onDeleteNote(DBusGProxy *proxy,const gchar *note_uri, const gchar *note_title, gpointer data)
{
	cd_message ("%s (%s)", __func__, note_uri);
	//reload_all_notes ();
	Icon *pIcon = _cd_tomboy_find_note_from_name (note_uri);
	g_return_if_fail (pIcon != NULL);
	
	if (myDock)
	{
		if (myIcon->pSubDock != NULL)
		{
			cairo_dock_detach_icon_from_dock (pIcon, myIcon->pSubDock, FALSE);
			cairo_dock_update_dock_size (myIcon->pSubDock);
		}
	}
	else
	{
		myDesklet->icons = g_list_remove (myDesklet->icons, pIcon);
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
	}
	
	cairo_dock_free_icon (pIcon);
	myData.countNotes --;
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d",myData.countNotes)
}

void onAddNote(DBusGProxy *proxy,const gchar *note_uri, gpointer data)
{
	cd_message ("%s (%s)", __func__, note_uri);
	//registerNote(note_uri);
	//update_icon();
	
	Icon *pIcon = g_new0 (Icon, 1);
	pIcon->acName = getNoteTitle(note_uri);
	pIcon->fScale = 1.;
	pIcon->fAlpha = 1.;
	pIcon->fWidth = 48;  /// inutile je pense ...
	pIcon->fHeight = 48;
	pIcon->fWidthFactor = 1.;
	pIcon->fHeightFactor = 1.;
	pIcon->acCommand = g_strdup (note_uri);  /// avec g_strdup_printf ("tomboy --open-note %s", pNote->name), ca devient un vrai lanceur.
	pIcon->cParentDockName = g_strdup (myIcon->acName);
	pIcon->acFileName = g_strdup_printf ("%s/note.svg",MY_APPLET_SHARE_DATA_DIR);
	
	GList *pList = (myDock ? (myIcon->pSubDock != NULL ? myIcon->pSubDock->icons : NULL) : myDesklet->icons);
	Icon *pLastIcon = cairo_dock_get_last_icon (pList);
	pIcon->fOrder = (pLastIcon != NULL ? pLastIcon->fOrder + 1 : 0);
	
	if (myDock)
	{
		if (myIcon->pSubDock == NULL)
		{
			myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (NULL, myIcon->acName);
			cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
		}
		
		cairo_dock_load_one_icon_from_scratch (pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
		cairo_dock_insert_icon_in_dock (pIcon, myIcon->pSubDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, FALSE);
	}
	else
	{
		myDesklet->icons = g_list_insert_sorted (myDesklet->icons,
			pIcon,
			(GCompareFunc) cairo_dock_compare_icons_order);
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
	}
	
	myData.countNotes ++;
	CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d",myData.countNotes)
}

void onChangeNoteList(DBusGProxy *proxy,const gchar *note_uri, gpointer data)
{
	cd_message ("%s (%s)", __func__, note_uri);
	Icon *pIcon = _cd_tomboy_find_note_from_name (note_uri);
	g_return_if_fail (pIcon != NULL);
	gchar *cTitle = getNoteTitle(note_uri);
	if (cTitle == NULL || strcmp (cTitle, pIcon->acName) != 0)  // nouveau titre.
	{
		pIcon->acName = cTitle;
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
		cairo_dock_fill_one_text_buffer (pIcon, pCairoContext, g_iLabelSize, g_cLabelPolice, (g_bTextAlwaysHorizontal ? CAIRO_DOCK_HORIZONTAL : myContainer->bIsHorizontal), myContainer->bDirectionUp);
		cairo_destroy (pCairoContext);
	}
	else
		g_free (cTitle);
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

gboolean cd_tomboy_check_deleted_notes (gpointer data)
{
	gchar **cNotes = NULL;
	if(dbus_g_proxy_call (dbus_proxy_tomboy, "ListAllNotes", NULL,
		G_TYPE_INVALID,
		G_TYPE_STRV,&cNotes,
		G_TYPE_INVALID))
	{
		int i = 0;
		while (cNotes[i] != NULL)
			i ++;
		if (i < myData.countNotes)  // il y'a eu suppression.
		{
			cd_message ("tomboy : une note au moins a ete supprimee.");
			free_all_notes ();
			for (i = 0; cNotes[i] != NULL; i ++)
			{
				registerNote(cNotes[i]);
			}
			update_icon();
		}
		g_strfreev (cNotes);
	}
	return TRUE;
}
