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
#include "applet-notes.h"
#include "applet-backend-tomboy.h"
#include "applet-backend-default.h"

static DBusGProxy *dbus_proxy_tomboy = NULL;

extern struct tm *localtime_r (const time_t *timer, struct tm *tp);

#define g_marshal_value_peek_string(v)   (char*) g_value_get_string (v)
#define g_marshal_value_peek_object(v)   g_value_get_object (v)

// start -> get notes {ID+title+content+date} -> load list into icons/treeview
// click main -> show sub-dock/dialog
// middle-click main -> dialog -> title -> create_note
// click note -> show_note
// menu note -> delete -> delete_note
//           -> show -> show_note
//           -> search -> dialog -> search in icons title and content -> popup menu -> select -> show_note
// menu main -> add new note (middle-click)
//           -> search -> dialog -> search in icons title and content -> popup menu -> select -> show_note
//           -> reload notes
//
// Slide -> buttons/combo "by title" "by date" "by tags" + filter entry
// Dialog -> filter + tag-filter + treeview (title,date,last change,tag) + menu (delete/add/show)

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


static void _load_note_image (Icon *pIcon)
{
	int iWidth = cairo_dock_icon_get_allocated_width (pIcon);
	int iHeight = cairo_dock_icon_get_allocated_width (pIcon);
	
	cd_tomboy_load_note_surface (iWidth, iHeight);
	
	cairo_surface_t *pSurface = cairo_dock_duplicate_surface (myData.pSurfaceNote, iWidth, iHeight, iWidth, iHeight);
	cairo_dock_load_image_buffer_from_surface (&pIcon->image, pSurface, iWidth, iHeight);
	
	if (pIcon->image.pSurface)
	{
		cairo_t *pIconContext = cairo_create (pIcon->image.pSurface);
		cd_tomboy_draw_content_on_icon (pIconContext, pIcon);
		cairo_destroy (pIconContext);
	}
}
Icon *cd_notes_create_icon_for_note (CDNote *pNote)
{
	gchar *cTitle;
	if (pNote->cTitle == NULL)
		cTitle = g_strdup (D_("No title"));
	else if (*pNote->cTitle == '\0')
	{
		cTitle = g_strdup (D_("No title"));
		g_free (pNote->cTitle);
	}
	else
		cTitle = pNote->cTitle;

	Icon *pIcon = cairo_dock_create_dummy_launcher (cTitle,
		(myConfig.cNoteIcon == NULL ?
			g_strdup (MY_APPLET_SHARE_DATA_DIR"/note.svg") :
			g_strdup (myConfig.cNoteIcon)),
		pNote->cID,
		NULL,
		0);
	pNote->cTitle = NULL;
	pNote->cID = NULL;

	if (myConfig.bDrawContent)
	{
		pIcon->cClass = pNote->cContent;
		pNote->cContent = NULL;
		cairo_dock_set_icon_static (pIcon, TRUE);  // pour la lisibilite, pas d'animation.
		pIcon->iface.load_image = _load_note_image;
	}
	return pIcon;
}


static void free_all_notes (void)
{
	cd_debug ("");
	g_hash_table_remove_all (myData.hNoteTable);
	gldi_object_remove_notification (CD_APPLET_MY_ICONS_LIST_CONTAINER,
		NOTIFICATION_ENTER_ICON,
		(GldiNotificationFunc) cd_tomboy_on_change_icon,
		myApplet);
	CD_APPLET_DELETE_MY_ICONS_LIST;
}


  ////////////
 /// FIND ///
////////////

static gchar **_cd_tomboy_get_note_names_with_tag (const gchar *cTag)
{
	gchar **cNoteNames = NULL;
	dbus_g_proxy_call (dbus_proxy_tomboy, "GetAllNotesWithTag", NULL,
		G_TYPE_STRING, cTag,
		G_TYPE_INVALID,
		G_TYPE_STRV, &cNoteNames,
		G_TYPE_INVALID);
	return cNoteNames;
}
GList *cd_tomboy_find_notes_with_tag (const gchar *cTag)
{
	gchar **cNoteNames = _cd_tomboy_get_note_names_with_tag (cTag);
	if (cNoteNames == NULL)
		return NULL;
	
	// GList *pList = (myDock ? (myIcon->pSubDock ? myIcon->pSubDock->icons : NULL) : myDesklet->icons);
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


static gboolean _cd_tomboy_note_has_contents (const gchar *cNoteName, const gchar **cContents)
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
			cd_debug (" %s : %s", cNoteName, cContents[i]);
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
GList *cd_tomboy_find_notes_with_contents (const gchar **cContents)
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
	
	const gchar *cContents[2] = {s_cDateBuffer, NULL};
	return cd_tomboy_find_notes_with_contents (cContents);
}

GList *cd_tomboy_find_note_for_this_week (void)
{
	static char s_cDateBuffer[CD_TOMBOY_DATE_BUFFER_LENGTH+1];
	static struct tm epoch_tm;
	time_t epoch = (time_t) time (NULL);
	localtime_r (&epoch, &epoch_tm);
	cd_debug ("epoch_tm.tm_wday : %d", epoch_tm.tm_wday);
	int i, iNbDays = (8 - epoch_tm.tm_wday) % 7;  // samedi <=> 6, dimanche <=> 0.
	
	gchar **cDays = g_new0 (gchar *, iNbDays + 1);
	for (i = 0; i < iNbDays; i ++)
	{
		epoch = (time_t) time (NULL) + i * 86400;
		localtime_r (&epoch, &epoch_tm);
		strftime (s_cDateBuffer, CD_TOMBOY_DATE_BUFFER_LENGTH, myConfig.cDateFormat, &epoch_tm);
		cDays[i] = g_strdup (s_cDateBuffer);
	}
	
	GList *pList = cd_tomboy_find_notes_with_contents ((const gchar **)cDays);
	g_strfreev (cDays);
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
	
	GList *pList = cd_tomboy_find_notes_with_contents ((const gchar **)cDays);
	g_strfreev (cDays);
	return pList;
}



void cd_notes_show_note (const gchar *cNoteID)
{
	if (myData.backend.show_note)
		myData.backend.show_note (cNoteID);
}

void cd_notes_delete_note (const gchar *cNoteID)
{
	if (myData.backend.delete_note)
		myData.backend.delete_note (cNoteID);
}

gchar *cd_notes_create_note (const gchar *cTitle)
{
	if (myData.backend.create_note)
		return myData.backend.create_note (cTitle);
	return NULL;
}

void cd_notes_run_manager (void)
{
	if (myData.backend.run_manager)
		myData.backend.run_manager ();
}

void cd_notes_start (void)
{
	myData.iIconState = -1;
	// set the backend
	switch (myConfig.iAppControlled)
	{
		case CD_NOTES_TOMBOY:
		case CD_NOTES_GNOTES:
			cd_notes_register_tomboy_backend ();
		break;
		default:
			cd_notes_register_default_backend ();
		break;
	}
	// start it
	myData.backend.start ();
}

void cd_notes_stop (void)
{
	if (myData.backend.stop)
		myData.backend.stop ();
	
	gldi_task_discard (myData.pTask);
	myData.pTask = NULL;
	
	free_all_notes ();  // detruit aussi la liste des icones.
}



static void _load_notes (void)
{
	GList *pList = g_hash_table_get_values (myData.hNoteTable);
	CD_APPLET_LOAD_MY_ICONS_LIST (pList, myConfig.cRenderer, "Slide", NULL);  // pList desormais appartient au container de l'applet, donc on ne la libere pas.
	
	gldi_object_remove_notification (CD_APPLET_MY_ICONS_LIST_CONTAINER,
		NOTIFICATION_ENTER_ICON,
		(GldiNotificationFunc) cd_tomboy_on_change_icon,
		myApplet);  // le sous-dock n'est pas forcement detruit.
	gldi_object_remove_notification (CD_APPLET_MY_ICONS_LIST_CONTAINER,
		myDock ? NOTIFICATION_LEAVE_DOCK : NOTIFICATION_LEAVE_DESKLET,
		(GldiNotificationFunc) cd_tomboy_on_leave_container,
		myApplet);  // le sous-dock n'est pas forcement detruit.
	if (myConfig.bPopupContent)
	{
		gldi_object_register_notification (CD_APPLET_MY_ICONS_LIST_CONTAINER,
			NOTIFICATION_ENTER_ICON,
			(GldiNotificationFunc) cd_tomboy_on_change_icon,
			GLDI_RUN_AFTER, myApplet);
		gldi_object_register_notification (CD_APPLET_MY_ICONS_LIST_CONTAINER,
			myDock ? NOTIFICATION_LEAVE_DOCK : NOTIFICATION_LEAVE_DESKLET,  // a bit unfortunate
			(GldiNotificationFunc) cd_tomboy_on_leave_container,
			GLDI_RUN_AFTER, myApplet);
	}
	cd_tomboy_update_icon ();
}
void cd_notes_store_load_notes (GList *pNotes)
{
	int i = 0;
	CDNote *pNote;
	GList *n;
	for (n = pNotes; n != NULL; n = n->next)
	{
		pNote = n->data;
		Icon *pIcon = cd_notes_create_icon_for_note (pNote);
		pIcon->fOrder = i++;
		_cd_tomboy_register_note (pIcon);
	}
	
	_load_notes ();
}

void cd_notes_store_add_note (CDNote *pNote)
{
	// on verifie que l'icone n'existe pas deja.
	Icon *pIcon = _cd_tomboy_find_note_from_uri (pNote->cID);
	if (pIcon != NULL)
		return;
	
	pIcon = cd_notes_create_icon_for_note (pNote);
	pIcon->fOrder = CAIRO_DOCK_LAST_ORDER;
	
	CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pIcon);
	
	_cd_tomboy_register_note (pIcon);
	cd_tomboy_update_icon ();
}

void cd_notes_store_remove_note (const gchar *cNoteID)
{
	Icon *pIcon = _cd_tomboy_find_note_from_uri (cNoteID);
	g_return_if_fail (pIcon != NULL);
	
	_cd_tomboy_unregister_note (pIcon);
	
	CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pIcon);  // detruit l'icone.
	
	cd_tomboy_update_icon ();
}

static const gchar * _get_display_title (const gchar *cTitle)
{
	return cTitle && *cTitle != '\0' ? cTitle : D_("No title");
}

void cd_notes_store_update_note (CDNote *pUpdatedNote)
{
	Icon *pIcon = _cd_tomboy_find_note_from_uri (pUpdatedNote->cID);
	g_return_if_fail (pIcon != NULL);
	
	cd_debug ("  %s -> %s", pUpdatedNote->cTitle, pIcon->cName);
	if (g_strcmp0 (pUpdatedNote->cTitle, pIcon->cName) != 0)  // nouveau titre.
	{
		gldi_icon_set_name (pIcon, _get_display_title (pUpdatedNote->cTitle));
	}
	
	if (myConfig.bDrawContent)
	{
		cd_debug ("  %s -> %s", pIcon->cClass, pUpdatedNote->cContent);
		if (g_strcmp0 (pIcon->cClass, pUpdatedNote->cContent) != 0)
		{
			g_free (pIcon->cClass);
			pIcon->cClass = pUpdatedNote->cContent;
			pUpdatedNote->cContent = NULL;
			if (pIcon->image.pSurface)
			{
				cairo_t *pIconContext = cairo_dock_begin_draw_icon_cairo (pIcon, 0, NULL);
				g_return_if_fail (pIconContext != NULL);
				if (myData.pSurfaceNote == NULL)
				{
					int iWidth, iHeight;
					cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
					cd_tomboy_load_note_surface (iWidth, iHeight);
				}
				cairo_dock_set_icon_surface (pIconContext, myData.pSurfaceNote, pIcon);  // on efface l'ancien texte.
				cd_tomboy_draw_content_on_icon (pIconContext, pIcon);
				cairo_dock_end_draw_icon_cairo (pIcon);
				cairo_destroy (pIconContext);
			}
		}
	}
	if (myDesklet)
		cairo_dock_redraw_container (myContainer);
}

void cd_notes_free_note (CDNote *pNote)
{
	if (!pNote)
		return;
	g_free (pNote->cID);
	g_free (pNote->cTitle);
	g_free (pNote->cContent);
	g_free (pNote->cTags);
	g_free (pNote);
}
