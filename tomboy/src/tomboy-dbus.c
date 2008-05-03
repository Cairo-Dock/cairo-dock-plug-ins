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



static Icon *_cd_tomboy_create_icon_for_note (gchar *cNoteURI, gchar *cNoteTitle)
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
	pIcon->acFileName = g_strdup_printf ("%s/note.svg",MY_APPLET_SHARE_DATA_DIR);
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

void onDeleteNote(DBusGProxy *proxy,const gchar *note_uri, const gchar *note_title, gpointer data)
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

void onAddNote(DBusGProxy *proxy,const gchar *note_uri, gpointer data)
{
	cd_message ("%s (%s)", __func__, note_uri);
	
	Icon *pIcon = _cd_tomboy_create_icon_for_note (note_uri, getNoteTitle(note_uri));
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
	
	_cd_tomboy_register_note (pIcon);
	update_icon ();
}

void onChangeNoteList(DBusGProxy *proxy,const gchar *note_uri, gpointer data)
{
	cd_message ("%s (%s)", __func__, note_uri);
	Icon *pIcon = _cd_tomboy_find_note_from_uri (note_uri);
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

static gboolean _cd_tomboy_remove_old_notes (gchar *cNoteURI, Icon *pIcon, double *fTime)
{
	if (pIcon->fLastCheckTime < *fTime)
	{
		cd_message ("cette note (%s) est trop vieille\n", cNoteURI);
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
			GTimeVal time_val;
			g_get_current_time (&time_val);
			double fTime = time_val.tv_sec + time_val.tv_usec * 1e-6;
			for (i = 0; cNotes[i] != NULL; i ++)
			{
				cNoteURI = cNotes[i];
				pIcon = _cd_tomboy_find_note_from_uri (cNoteURI);
				if (pIcon != NULL)
					pIcon->fLastCheckTime = fTime;
			}
			
			int iNbRemovedIcons = g_hash_table_foreach_remove (myData.hNoteTable, (GHRFunc) _cd_tomboy_remove_old_notes, &fTime);
			if (iNbRemovedIcons != 0)
			{
				cd_message ("%d notes enlevees\n", iNbRemovedIcons);
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
	return TRUE;
}



void reload_all_notes (void)
{
	cd_message ("");
	getAllNotes();
	update_icon();
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

void getAllNotes(void)
{
	cd_message("tomboy : getAllNotes");
	
	free_all_notes ();
	GList *pList = NULL;
	
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
			Icon *pIcon = _cd_tomboy_create_icon_for_note (cNoteURI, getNoteTitle(cNoteURI));
			pIcon->fOrder = i;
			pList = g_list_append (pList, pIcon);
			_cd_tomboy_register_note (pIcon);
		}
	}
	g_strfreev (note_list);
	
	if (myDock)
	{
		if (myIcon->pSubDock == NULL)
		{
			myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pList, myIcon->acName);
			cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
		}
		else
		{
			myIcon->pSubDock->icons = pList;
			cairo_dock_load_buffers_in_one_dock (myIcon->pSubDock);
		}
	}
	else
	{
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Tree", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
	}
}

void free_all_notes (void)
{
	cd_message (""); 
	g_hash_table_remove_all (myData.hNoteTable);
	if (myDock)
	{
		if (myIcon->pSubDock != NULL)
		{
			myIcon->pSubDock->icons = NULL;
		}
	}
	else
	{
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
