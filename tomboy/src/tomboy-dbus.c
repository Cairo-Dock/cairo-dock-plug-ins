#include <string.h>
#include <dbus/dbus-glib.h>
#include <time.h>

#include "tomboy-struct.h"
#include "tomboy-draw.h"
#include "tomboy-dbus.h"

static DBusGProxy *dbus_proxy_tomboy = NULL;

extern struct tm *localtime_r (time_t *timer, struct tm *tp);


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
			///G_TYPE_STRING,
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



static Icon *_cd_tomboy_create_icon_for_note (const gchar *cNoteURI)
{
	Icon *pIcon = g_new0 (Icon, 1);
	pIcon->acName = getNoteTitle (cNoteURI);
	pIcon->fScale = 1.;
	pIcon->fAlpha = 1.;
	pIcon->fWidth = 48;  /// inutile je pense ...
	pIcon->fHeight = 48;
	pIcon->fWidthFactor = 1.;
	pIcon->fHeightFactor = 1.;
	pIcon->acCommand = g_strdup (cNoteURI);  /// avec g_strdup_printf ("tomboy --open-note %s", pNote->name), ca devient un vrai lanceur.
	pIcon->cParentDockName = g_strdup (myIcon->acName);
	pIcon->acFileName = g_strdup (MY_APPLET_SHARE_DATA_DIR"/note.svg");
	if (myConfig.bDrawContent)
		pIcon->cClass = getNoteContent (cNoteURI);
	return pIcon;
}

static Icon *_cd_tomboy_find_note_from_uri (const gchar *cNoteURI)
{
	g_return_val_if_fail (cNoteURI != NULL, NULL);
	return g_hash_table_lookup (myData.hNoteTable, cNoteURI);
}

static void _cd_tomboy_register_note (Icon *pIcon)
{
	g_return_if_fail (pIcon != NULL && pIcon->acCommand != NULL);
	g_hash_table_insert (myData.hNoteTable, pIcon->acCommand, pIcon);
}

static void _cd_tomboy_unregister_note (Icon *pIcon)
{
	g_return_if_fail (pIcon != NULL && pIcon->acCommand != NULL);
	g_hash_table_remove (myData.hNoteTable, pIcon->acCommand);
}


void onDeleteNote(DBusGProxy *proxy, const gchar *note_uri, /**const gchar *note_title, */gpointer data)
{
	cd_message ("%s (%s)", __func__, note_uri);
	Icon *pIcon = _cd_tomboy_find_note_from_uri (note_uri);
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
	
	_cd_tomboy_unregister_note (pIcon);
	update_icon ();
}

void onAddNote(DBusGProxy *proxy, const gchar *note_uri, gpointer data)
{
	cd_message ("%s (%s)", __func__, note_uri);
	
	Icon *pIcon = _cd_tomboy_create_icon_for_note (note_uri);
	GList *pList = (myDock ? (myIcon->pSubDock != NULL ? myIcon->pSubDock->icons : NULL) : myDesklet->icons);
	Icon *pLastIcon = cairo_dock_get_last_icon (pList);
	pIcon->fOrder = (pLastIcon != NULL ? pLastIcon->fOrder + 1 : 0);
	
	if (myDock)
	{
		if (myIcon->pSubDock == NULL)
		{
			CD_APPLET_CREATE_MY_SUBDOCK (NULL, myConfig.cRenderer);
		}
		
		cairo_dock_load_one_icon_from_scratch (pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
		cairo_dock_insert_icon_in_dock_full (pIcon, myIcon->pSubDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, ! CAIRO_DOCK_INSERT_SEPARATOR, NULL);
	}
	else
	{
		myDesklet->icons = g_list_insert_sorted (myDesklet->icons,
			pIcon,
			(GCompareFunc) cairo_dock_compare_icons_order);
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
	}
	
	_cd_tomboy_register_note (pIcon);
	update_icon ();
	
	if (pIcon->cClass != NULL)
	{
		cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
		cd_tomboy_draw_content_on_icon (pIconContext, pIcon, pIcon->cClass);
		cairo_destroy (pIconContext);
		g_free (pIcon->cClass);
		pIcon->cClass = NULL;
	}
}

void onChangeNoteList(DBusGProxy *proxy, const gchar *note_uri, gpointer data)
{
	cd_message ("%s (%s)", __func__, note_uri);
	Icon *pIcon = _cd_tomboy_find_note_from_uri (note_uri);
	g_return_if_fail (pIcon != NULL);
	gchar *cTitle = getNoteTitle(note_uri);
	if (cTitle == NULL || strcmp (cTitle, pIcon->acName) != 0)  // nouveau titre.
	{
		pIcon->acName = cTitle;
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
		cairo_dock_fill_one_text_buffer (pIcon, pCairoContext, &myLabels.iconTextDescription);
		cairo_destroy (pCairoContext);
	}
	else
		g_free (cTitle);
	
	gchar *cNoteContent = getNoteContent (note_uri);
	if (cNoteContent != NULL)
	{
		cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
		cairo_dock_set_icon_surface (pIconContext, myData.pSurfaceNote);  // on efface l'ancien texte.
		cd_tomboy_draw_content_on_icon (pIconContext, pIcon, cNoteContent);
		cairo_destroy (pIconContext);
		g_free (cNoteContent);
	}
}

static gboolean _cd_tomboy_remove_old_notes (gchar *cNoteURI, Icon *pIcon, int iTime)
{
	if (pIcon->iLastCheckTime < iTime)
	{
		cd_message ("cette note (%s) est trop vieille", cNoteURI);
		if (myDock)
		{
			if (myIcon->pSubDock != NULL)
			{
				cairo_dock_detach_icon_from_dock (pIcon, myIcon->pSubDock, FALSE);
			}
		}
		else
		{
			myDesklet->icons = g_list_remove (myDesklet->icons, pIcon);
		}
		
		return TRUE;
	}
	return FALSE;
}
gboolean cd_tomboy_check_deleted_notes (gpointer data)
{
	static int iTime = 0;
	iTime ++;
	cd_message ("");
	gchar **cNotes = NULL;
	if(dbus_g_proxy_call (dbus_proxy_tomboy, "ListAllNotes", NULL,
		G_TYPE_INVALID,
		G_TYPE_STRV,&cNotes,
		G_TYPE_INVALID))
	{
		int i = 0;
		while (cNotes[i] != NULL)
			i ++;
		if (i < g_hash_table_size (myData.hNoteTable))  // il y'a eu suppression.
		{
			cd_message ("tomboy : une note au moins a ete supprimee.");
			
			gchar *cNoteURI;
			Icon *pIcon;
			for (i = 0; cNotes[i] != NULL; i ++)
			{
				cNoteURI = cNotes[i];
				pIcon = _cd_tomboy_find_note_from_uri (cNoteURI);
				if (pIcon != NULL)
					pIcon->iLastCheckTime = iTime;
			}
			
			int iNbRemovedIcons = g_hash_table_foreach_remove (myData.hNoteTable, (GHRFunc) _cd_tomboy_remove_old_notes, GINT_TO_POINTER (iTime));
			if (iNbRemovedIcons != 0)
			{
				cd_message ("%d notes enlevees", iNbRemovedIcons);
				if (myDock)
				{
					if (myIcon->pSubDock != NULL)
					{
						cairo_dock_update_dock_size (myIcon->pSubDock);
					}
				}
				else
				{
					cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
				}
				update_icon ();
			}
		}
		
		g_strfreev (cNotes);
	}
	else
		g_print ("tomboy is not running\n");
	return TRUE;
}


gchar *getNoteTitle (const gchar *note_name)
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
gchar *getNoteContent (const gchar *note_name)
{
	gchar *cNoteContent = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetNoteContents", NULL,
		G_TYPE_STRING, note_name,
		G_TYPE_INVALID,
		G_TYPE_STRING, &cNoteContent,
		G_TYPE_INVALID);
	return cNoteContent;
}

void getAllNotes(void)
{
	cd_message("");
	
	gchar **note_list = NULL;
	if(dbus_g_proxy_call (dbus_proxy_tomboy, "ListAllNotes", NULL,
		G_TYPE_INVALID,
		G_TYPE_STRV,&note_list,
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
	}
	g_strfreev (note_list);
}

gboolean cd_tomboy_load_notes (void)
{
	GList *pList = g_hash_table_get_values (myData.hNoteTable);
	if (myDock)
	{
		if (myIcon->pSubDock == NULL)
		{
			CD_APPLET_CREATE_MY_SUBDOCK (pList, myConfig.cRenderer);
		}
		else
		{
			CD_APPLET_LOAD_ICONS_IN_MY_SUBDOCK (pList);
		}
	}
	else
	{
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
	}
	update_icon ();
	
	Icon *icon;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->cClass != NULL)
		{
			cairo_t *pIconContext = cairo_create (icon->pIconBuffer);
			cd_tomboy_draw_content_on_icon (pIconContext, icon, icon->cClass);
			cairo_destroy (pIconContext);
			g_free (icon->cClass);
			icon->cClass = NULL;
		}
	}
	
	if (myConfig.bNoDeletedSignal && myData.iSidCheckNotes == 0)
		myData.iSidCheckNotes = g_timeout_add_seconds (2, (GSourceFunc) cd_tomboy_check_deleted_notes, (gpointer) NULL);
	return TRUE;
}

void free_all_notes (void)
{
	cd_message ("");
	g_hash_table_remove_all (myData.hNoteTable);
	if (myDock)
	{
		if (myIcon->pSubDock != NULL)
		{
			g_list_free (myIcon->pSubDock->icons);
			myIcon->pSubDock->icons = NULL;
			cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);  // pour le reload.
		}
	}
	else
	{
		g_list_free (myDesklet->icons);
		myDesklet->icons = NULL;
	}
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



gchar **getNoteTags (const gchar *note_name)
{
	gchar **cTags = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetTagsForNote", NULL,
		G_TYPE_STRING, note_name,
		G_TYPE_INVALID,
		G_TYPE_STRV, &cTags,
		G_TYPE_INVALID);
	return cTags;
}



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
			g_print (" %s : %s\n", cNoteName, cContents[i]);
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
	GList *pList = (myDock ? (myIcon->pSubDock ? myIcon->pSubDock->icons : NULL) : myDesklet->icons);
	GList *pMatchList = NULL;
	Icon *icon;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (_cd_tomboy_note_has_contents (icon->acCommand, cContents))
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
	g_print ("epoch_tm.tm_wday : %d\n", epoch_tm.tm_wday);
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
