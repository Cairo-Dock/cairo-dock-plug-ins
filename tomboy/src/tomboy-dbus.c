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

#include <string.h>
#include <dbus/dbus-glib.h>
#include <time.h>

#include <glib-object.h>

#include "tomboy-struct.h"
#include "tomboy-draw.h"
#include "tomboy-notifications.h"
#include "tomboy-dbus.h"

static DBusGProxy *dbus_proxy_tomboy = NULL;

extern struct tm *localtime_r (time_t *timer, struct tm *tp);

#define g_marshal_value_peek_string(v)   (char*) g_value_get_string (v)
#define g_marshal_value_peek_object(v)   g_value_get_object (v)

  /////////////
 /// NOTES ///
/////////////

static Icon *_cd_tomboy_find_note_from_uri (const gchar *cNoteURI)
{
	g_return_val_if_fail (cNoteURI != NULL, NULL);
	return g_hash_table_lookup (myData.hNoteTable, cNoteURI);
}

static void _cd_tomboy_register_note (Icon *pIcon)
{
	g_return_if_fail (pIcon != NULL && pIcon->cCommand != NULL);
	g_hash_table_insert (myData.hNoteTable, pIcon->cCommand, pIcon);
}

static void _cd_tomboy_unregister_note (Icon *pIcon)
{
	g_return_if_fail (pIcon != NULL && pIcon->cCommand != NULL);
	g_hash_table_remove (myData.hNoteTable, pIcon->cCommand);
}


static gchar *getNoteTitle (const gchar *note_name)
{
	cd_debug("%s (%s)", __func__, note_name);
	gchar *note_title = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetNoteTitle", NULL,
		G_TYPE_STRING, note_name,
		G_TYPE_INVALID,
		G_TYPE_STRING,&note_title,
		G_TYPE_INVALID);
	
	return note_title;
}
static gchar *getNoteContent (const gchar *note_name)
{
	gchar *cNoteContent = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetNoteContents", NULL,
		G_TYPE_STRING, note_name,
		G_TYPE_INVALID,
		G_TYPE_STRING, &cNoteContent,
		G_TYPE_INVALID);
	return cNoteContent;
}

static gchar **getNoteTags (const gchar *note_name)
{
	gchar **cTags = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetTagsForNote", NULL,
		G_TYPE_STRING, note_name,
		G_TYPE_INVALID,
		G_TYPE_STRV, &cTags,
		G_TYPE_INVALID);
	return cTags;
}


static void _load_note_image (Icon *pIcon)
{
	CD_APPLET_ENTER;
	int iWidth = pIcon->iImageWidth;
	int iHeight = pIcon->iImageHeight;
	pIcon->pIconBuffer = cairo_dock_create_blank_surface (iWidth, iHeight);
	
	cd_tomboy_load_note_surface (iWidth, iHeight);
	
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	cairo_dock_set_icon_surface (pIconContext, myData.pSurfaceNote);
	cd_tomboy_draw_content_on_icon (pIconContext, pIcon);
	cairo_destroy (pIconContext);
	CD_APPLET_LEAVE ();
}
static Icon *_cd_tomboy_create_icon_for_note (const gchar *cNoteURI)
{
	Icon *pIcon = cairo_dock_create_dummy_launcher (getNoteTitle (cNoteURI),
		(myConfig.cNoteIcon == NULL ?
			g_strdup (MY_APPLET_SHARE_DATA_DIR"/note.svg") :
			g_strdup (myConfig.cNoteIcon)),
		g_strdup (cNoteURI),
		NULL,
		0);  // avec g_strdup_printf ("tomboy --open-note %s", pNote->name), ca pourrait faire un vrai lanceur.
	if (myConfig.bDrawContent)
	{
		pIcon->cClass = getNoteContent (cNoteURI);
		cairo_dock_set_icon_static (pIcon);  // pour la lisibilite, pas d'animation.
		pIcon->iface.load_image = _load_note_image;
	}
	return pIcon;
}


static void onNoteDeleted (DBusGProxy *proxy, const gchar *note_uri, const gchar *note_title, gpointer data)
{
	cd_message ("%s (%s)", __func__, note_uri);
	Icon *pIcon = _cd_tomboy_find_note_from_uri (note_uri);
	g_return_if_fail (pIcon != NULL);
	
	_cd_tomboy_unregister_note (pIcon);
	
	CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pIcon);  // detruit l'icone.
	
	cd_tomboy_update_icon ();
}

static void onNoteAdded (DBusGProxy *proxy, const gchar *note_uri, gpointer data)
{
	cd_message ("%s (%s)", __func__, note_uri);
	
	// on verifie que l'icone n'existe pas deja.
	Icon *pIcon = _cd_tomboy_find_note_from_uri (note_uri);
	if (pIcon != NULL)
		return;
	
	pIcon = _cd_tomboy_create_icon_for_note (note_uri);
	pIcon->fOrder = CAIRO_DOCK_LAST_ORDER;
	
	CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pIcon);
	
	_cd_tomboy_register_note (pIcon);
	cd_tomboy_update_icon ();
}

static void onNoteSaved (DBusGProxy *proxy, const gchar *note_uri, gpointer data)
{
	cd_message ("%s (%s)", __func__, note_uri);
	Icon *pIcon = _cd_tomboy_find_note_from_uri (note_uri);
	g_return_if_fail (pIcon != NULL);
	
	gchar *cTitle = getNoteTitle (note_uri);
	if (cTitle == NULL || strcmp (cTitle, pIcon->cName) != 0)  // nouveau titre.
	{
		cairo_dock_set_icon_name (cTitle, pIcon, CD_APPLET_MY_ICONS_LIST_CONTAINER);
	}
	g_free (cTitle);
	
	if (myConfig.bDrawContent)
	{
		g_free (pIcon->cClass);
		pIcon->cClass = getNoteContent (note_uri);
		if (pIcon->cClass != NULL && pIcon->pIconBuffer)
		{
			cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
			if (myData.pSurfaceNote == NULL)
			{
				int iWidth, iHeight;
				cairo_dock_get_icon_extent (pIcon, CD_APPLET_MY_ICONS_LIST_CONTAINER, &iWidth, &iHeight);
				cd_debug ("on cree la surface a la taille %dx%d\n", iWidth, iHeight);
				myData.pSurfaceNote = cairo_dock_create_surface_from_image_simple (myConfig.cNoteIcon != NULL ? myConfig.cNoteIcon : MY_APPLET_SHARE_DATA_DIR"/note.svg",
					iWidth,
					iHeight);
			}
			cairo_dock_set_icon_surface (pIconContext, myData.pSurfaceNote);  // on efface l'ancien texte.
			cd_tomboy_draw_content_on_icon (pIconContext, pIcon);
			cairo_destroy (pIconContext);
		}
	}
	if (myDesklet)
		cairo_dock_redraw_container (myContainer);
}


static void g_cclosure_marshal_VOID__STRING_STRING (GClosure *closure, GValue *return_value G_GNUC_UNUSED, guint n_param_values, const GValue *param_values, gpointer invocation_hint G_GNUC_UNUSED, gpointer marshal_data)
{
	cd_debug ("marshaller");
	typedef void (*GMarshalFunc_VOID__STRING_STRING) (gpointer data1,
		gchar*   arg_1,
		gchar*   arg_2,
		gpointer data2);
	register GMarshalFunc_VOID__STRING_STRING callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;

	g_return_if_fail (n_param_values == 3);

	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	}
	else
	{
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__STRING_STRING) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
		g_marshal_value_peek_string (param_values + 1),
		g_marshal_value_peek_string (param_values + 2),
		data2);

}
gboolean dbus_connect_to_bus (void)
{
	cd_message ("");
	if (cairo_dock_dbus_is_enabled ())
	{
		dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__STRING_STRING,
		G_TYPE_NONE, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
		
		switch (myConfig.iAppControlled)
		{
			case CD_NOTES_TOMBOY:
			default:
				dbus_proxy_tomboy = cairo_dock_create_new_dbus_proxy (
					"org.gnome.Tomboy",
					"/org/gnome/Tomboy/RemoteControl",
					"org.gnome.Tomboy.RemoteControl");
			break;
			case CD_NOTES_GNOTES:
				dbus_proxy_tomboy = cairo_dock_create_new_dbus_proxy (
					"org.gnome.Gnote",
					"/org/gnome/Gnote/RemoteControl",
					"org.gnome.Gnote.RemoteControl");
			break;
		}
		g_return_val_if_fail (dbus_proxy_tomboy != NULL, FALSE);
		
		dbus_g_proxy_add_signal(dbus_proxy_tomboy, "NoteDeleted",
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
			G_CALLBACK(onNoteDeleted), NULL, NULL);
		dbus_g_proxy_connect_signal(dbus_proxy_tomboy, "NoteAdded",
			G_CALLBACK(onNoteAdded), NULL, NULL);
		dbus_g_proxy_connect_signal(dbus_proxy_tomboy, "NoteSaved",
			G_CALLBACK(onNoteSaved), NULL, NULL);
		return TRUE;
	}
	return FALSE;
}

void dbus_disconnect_from_bus (void)
{
	cd_message ("");
	if (dbus_proxy_tomboy == NULL)
		return ;
	
	if (myData.pDetectTomboyCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, myData.pDetectTomboyCall);
		myData.pDetectTomboyCall = NULL;
	}
	if (myData.pGetNotesCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, myData.pGetNotesCall);
		myData.pGetNotesCall = NULL;
	}
	
	dbus_g_proxy_disconnect_signal(dbus_proxy_tomboy, "NoteDeleted",
		G_CALLBACK(onNoteDeleted), NULL);
	dbus_g_proxy_disconnect_signal(dbus_proxy_tomboy, "NoteAdded",
		G_CALLBACK(onNoteAdded), NULL);
	dbus_g_proxy_disconnect_signal(dbus_proxy_tomboy, "NoteSaved",
		G_CALLBACK(onNoteSaved), NULL);
	
	g_object_unref (dbus_proxy_tomboy);
	dbus_proxy_tomboy = NULL;
	
}

void dbus_detect_tomboy (void)
{
	cd_message ("");
	const gchar *cService = "";
	switch (myConfig.iAppControlled)
	{
		case CD_NOTES_TOMBOY:
		default:
			cService = "org.gnome.Tomboy";
		break;
		case CD_NOTES_GNOTES:
			cService = "org.gnome.Gnote";
		break;
	}
	myData.bIsRunning = cairo_dock_dbus_detect_application (cService);
}

static void _on_detect_tomboy (gboolean bPresent, gpointer data)
{
	CD_APPLET_ENTER;
	myData.pDetectTomboyCall = NULL;
	myData.bIsRunning = bPresent;
	getAllNotes_async ();
	CD_APPLET_LEAVE ();
}
void dbus_detect_tomboy_async (void)
{
	myData.bIsRunning = FALSE;
	const gchar *cService = "";
	switch (myConfig.iAppControlled)
	{
		case CD_NOTES_TOMBOY:
		default:
			cService = "org.gnome.Tomboy";
		break;
		case CD_NOTES_GNOTES:
			cService = "org.gnome.Gnote";
		break;
	}
	
	if (myData.pDetectTomboyCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, myData.pDetectTomboyCall);
	}
	myData.pDetectTomboyCall = cairo_dock_dbus_detect_application_async (cService, (CairoDockOnAppliPresentOnDbus) _on_detect_tomboy, NULL);
}


static void _load_notes (void)
{
	GList *pList = g_hash_table_get_values (myData.hNoteTable);
	CD_APPLET_LOAD_MY_ICONS_LIST (pList, myConfig.cRenderer, "Slide", NULL);  // pList desormais appartient au container de l'applet, donc on ne la libere pas.
	
	cairo_dock_remove_notification_func_on_object (CD_APPLET_MY_ICONS_LIST_CONTAINER,
		NOTIFICATION_ENTER_ICON,
		(CairoDockNotificationFunc) cd_tomboy_on_change_icon,
		myApplet);  // le sous-dock n'est pas forcement detruit.
	if (myConfig.bPopupContent)
		cairo_dock_register_notification_on_object (CD_APPLET_MY_ICONS_LIST_CONTAINER,
			NOTIFICATION_ENTER_ICON,
			(CairoDockNotificationFunc) cd_tomboy_on_change_icon,
			CAIRO_DOCK_RUN_AFTER, myApplet);
	
	cd_tomboy_update_icon ();
}
static void _on_get_all_notes (DBusGProxy *proxy, DBusGProxyCall *call_id, gpointer data)
{
	CD_APPLET_ENTER;
	myData.pGetNotesCall = NULL;
	gchar **note_list = NULL;
	if (dbus_g_proxy_end_call (proxy,
		call_id,
		NULL,
		G_TYPE_STRV,
		&note_list,
		G_TYPE_INVALID))
	{
		cd_message ("tomboy : Liste des notes...");
		gchar *cNoteURI;
		int i;
		for (i = 0; note_list[i] != NULL; i ++)
		{
			cNoteURI = note_list[i];
			Icon *pIcon = _cd_tomboy_create_icon_for_note (cNoteURI);
			pIcon->fOrder = i;  /// recuperer la date ...
			_cd_tomboy_register_note (pIcon);
		}
		g_strfreev (note_list);
	}
	
	_load_notes ();
	
	CD_APPLET_LEAVE ();
}
void getAllNotes_async (void)
{
	if (myData.pGetNotesCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, myData.pGetNotesCall);
	}
	myData.pGetNotesCall = dbus_g_proxy_begin_call (dbus_proxy_tomboy, "ListAllNotes",
		(DBusGProxyCallNotify)_on_get_all_notes,
		NULL,  // data
		(GDestroyNotify) NULL,
		G_TYPE_INVALID);
}

void free_all_notes (void)
{
	cd_message ("");
	g_hash_table_remove_all (myData.hNoteTable);
	cairo_dock_remove_notification_func_on_object (CD_APPLET_MY_ICONS_LIST_CONTAINER,
		NOTIFICATION_ENTER_ICON,
		(CairoDockNotificationFunc) cd_tomboy_on_change_icon,
		myApplet);
	CD_APPLET_DELETE_MY_ICONS_LIST;
}


  ///////////////
 /// ACTIONS ///
///////////////

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


  ////////////
 /// FIND ///
////////////

static gchar **_cd_tomboy_get_note_names_with_tag (gchar *cTag)
{
	gchar **cNoteNames = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetAllNotesWithTag", NULL,
		G_TYPE_STRING, cTag,
		G_TYPE_INVALID,
		G_TYPE_STRV, &cNoteNames,
		G_TYPE_INVALID);
	return cNoteNames;
}
GList *cd_tomboy_find_notes_with_tag (gchar *cTag)
{
	gchar **cNoteNames = _cd_tomboy_get_note_names_with_tag (cTag);
	if (cNoteNames == NULL)
		return NULL;
	
	GList *pList = (myDock ? (myIcon->pSubDock ? myIcon->pSubDock->icons : NULL) : myDesklet->icons);
	GList *pMatchList = NULL;
	Icon *pIcon;
	int i=0;
	while (cNoteNames[i] != NULL)
	{
		pIcon = _cd_tomboy_find_note_from_uri (cNoteNames[i]);
		if (pIcon != NULL)
			pMatchList = g_list_prepend (pMatchList, pMatchList);
		i ++;
	}
	return pMatchList;
}


static gboolean _cd_tomboy_note_has_contents (gchar *cNoteName, gchar **cContents)
{
	gchar *cNoteContent = NULL;
	if (dbus_g_proxy_call (dbus_proxy_tomboy, "GetNoteContents", NULL,
		G_TYPE_STRING, cNoteName,
		G_TYPE_INVALID,
		G_TYPE_STRING, &cNoteContent,
		G_TYPE_INVALID))
	{
		int i = 0;
		while (cContents[i] != NULL)
		{
			cd_debug (" %s : %s\n", cNoteName, cContents[i]);
			if (g_strstr_len (cNoteContent, strlen (cNoteContent), cContents[i]) != NULL)
			{
				g_free (cNoteContent);
				return TRUE;
			}
			i ++;
		}
	}
	g_free (cNoteContent);
	return FALSE;
}
GList *cd_tomboy_find_notes_with_contents (gchar **cContents)
{
	g_return_val_if_fail (cContents != NULL, NULL);
	GList *pList = CD_APPLET_MY_ICONS_LIST;
	GList *pMatchList = NULL;
	Icon *icon;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (_cd_tomboy_note_has_contents (icon->cCommand, cContents))
		{
			pMatchList = g_list_prepend (pMatchList, icon);
		}
	}
	return pMatchList;
}


#define CD_TOMBOY_DATE_BUFFER_LENGTH 50
GList *cd_tomboy_find_note_for_today (void)
{
	static char s_cDateBuffer[CD_TOMBOY_DATE_BUFFER_LENGTH+1];
	static struct tm epoch_tm;
	time_t epoch = (time_t) time (NULL);
	localtime_r (&epoch, &epoch_tm);
	strftime (s_cDateBuffer, CD_TOMBOY_DATE_BUFFER_LENGTH, myConfig.cDateFormat, &epoch_tm);
	
	gchar *cContents[2] = {s_cDateBuffer, NULL};
	return cd_tomboy_find_notes_with_contents (cContents);
}

GList *cd_tomboy_find_note_for_this_week (void)
{
	static char s_cDateBuffer[CD_TOMBOY_DATE_BUFFER_LENGTH+1];
	static struct tm epoch_tm;
	time_t epoch = (time_t) time (NULL);
	localtime_r (&epoch, &epoch_tm);
	cd_debug ("epoch_tm.tm_wday : %d\n", epoch_tm.tm_wday);
	int i, iNbDays = (8 - epoch_tm.tm_wday) % 7;  // samedi <=> 6, dimanche <=> 0.
	
	gchar **cDays = g_new0 (gchar *, iNbDays + 1);
	for (i = 0; i < iNbDays; i ++)
	{
		epoch = (time_t) time (NULL) + i * 86400;
		localtime_r (&epoch, &epoch_tm);
		strftime (s_cDateBuffer, CD_TOMBOY_DATE_BUFFER_LENGTH, myConfig.cDateFormat, &epoch_tm);
		cDays[i] = g_strdup (s_cDateBuffer);
	}
	
	GList *pList = cd_tomboy_find_notes_with_contents (cDays);
	g_free (cDays);
	return pList;
}

GList *cd_tomboy_find_note_for_next_week (void)
{
	static char s_cDateBuffer[CD_TOMBOY_DATE_BUFFER_LENGTH+1];
	static struct tm epoch_tm;
	time_t epoch = (time_t) time (NULL);
	localtime_r (&epoch, &epoch_tm);
	int i, iDaysOffset = (8 - epoch_tm.tm_wday) % 7;
	
	gchar **cDays = g_new0 (gchar *, 8);
	for (i = 0; i < 7; i ++)
	{
		epoch = (time_t) time (NULL) + (i+iDaysOffset) * 86400;
		localtime_r (&epoch, &epoch_tm);
		strftime (s_cDateBuffer, CD_TOMBOY_DATE_BUFFER_LENGTH, myConfig.cDateFormat, &epoch_tm);
		cDays[i] = g_strdup (s_cDateBuffer);
	}
	
	GList *pList = cd_tomboy_find_notes_with_contents (cDays);
	g_free (cDays);
	return pList;
}
