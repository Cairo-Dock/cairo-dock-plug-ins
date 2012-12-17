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

#include <stdlib.h>
#define __USE_POSIX
#include <time.h>
#include <signal.h>

#include "applet-notes.h"
#include "tomboy-draw.h"
#include "tomboy-struct.h"
#include "tomboy-notifications.h"



CD_APPLET_ON_CLICK_BEGIN
	if (pClickedIcon != NULL && pClickedIcon != myIcon)
	{
		cd_message("tomboy : %s",pClickedIcon->cCommand);
		cd_notes_show_note (pClickedIcon->cCommand);
		
		if (myData.iSidPopupDialog != 0)
		{
			g_source_remove (myData.iSidPopupDialog);
			myData.iSidPopupDialog = 0;
		}
		cairo_dock_remove_dialog_if_any (pClickedIcon);
	}
	else if (pClickedIcon == myIcon && ! myData.bIsRunning)  // possible si on l'a quitte apres le demarrage de l'applet.
	{
		cd_notes_run_manager ();
	}
	else
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
CD_APPLET_ON_CLICK_END


static void _add_note_and_show (const gchar *note_title)
{
	gchar *cNoteId = cd_notes_create_note (note_title);
	cd_debug (" %s -> %s", note_title, cNoteId);
	cd_notes_show_note (cNoteId);
	g_free (cNoteId);
}
static void _on_got_name (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	CD_APPLET_ENTER;
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		const gchar *note_title = gtk_entry_get_text (GTK_ENTRY (pInteractiveWidget));
		if (note_title != NULL)
			_add_note_and_show (note_title);
	}
	CD_APPLET_LEAVE ();
}
static void _cd_tomboy_create_new_note (void)
{
	if (myConfig.bAutoNaming)
	{
		gchar *note_title = g_new0 (gchar, 50+1);
		time_t epoch = (time_t) time (NULL);
		struct tm currentTime;
		localtime_r (&epoch, &currentTime);
		strftime (note_title, 50, "%a-%d-%b_%r", &currentTime);
		
		_add_note_and_show (note_title);
		g_free (note_title);
	}
	else
	{
		cairo_dock_show_dialog_with_entry (D_("Note name : "),
			myIcon, myContainer,
			"same icon",
			NULL,
			(CairoDockActionOnAnswerFunc)_on_got_name,
			NULL, (GFreeFunc)NULL);
	}
}
static void _cd_tomboy_add_note (GtkMenuItem *menu_item, gpointer data)
{
	_cd_tomboy_create_new_note ();
}

static void _on_answer_delete (int iClickedButton, GtkWidget *pInteractiveWidget, const gchar *cCommand, CairoDialog *pDialog)
{
	CD_APPLET_ENTER;
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		cd_notes_delete_note (cCommand);
	}
	CD_APPLET_LEAVE ();
}
static void _cd_tomboy_delete_note (GtkMenuItem *menu_item, Icon *pIcon)
{
	g_return_if_fail (pIcon != NULL);
	if (myConfig.bAskBeforeDelete)
	{
		gchar *cQuestion = g_strdup_printf ("%s (%s)", D_("Delete this note?"), pIcon->cName);
		cairo_dock_show_dialog_with_question (cQuestion,
			pIcon, myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer,
			"same icon",
			(CairoDockActionOnAnswerFunc) _on_answer_delete,
			g_strdup (pIcon->cCommand), (GFreeFunc)g_free);
		g_free (cQuestion);
	}
	else
	{
		cd_notes_delete_note (pIcon->cCommand);
	}
}

static void _cd_tomboy_reload_notes (GtkMenuItem *menu_item, gpointer data)
{
	cd_notes_stop ();
	
	cd_notes_start ();
}

static void _on_active_search (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	CD_APPLET_ENTER;
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		const gchar *cContent = gtk_entry_get_text (GTK_ENTRY (pInteractiveWidget));
		if (cContent != NULL)
		{
			cd_tomboy_reset_icon_marks (FALSE);
			const gchar *cContents[2] = {cContent, NULL};
			GList *pList = cd_tomboy_find_notes_with_contents (cContents);
			cd_tomboy_show_results (pList);
			g_list_free (pList);
		}
	}
	CD_APPLET_LEAVE ();
}
static void _cd_tomboy_search_for_content (GtkMenuItem *menu_item, gpointer data)
{
	cairo_dock_show_dialog_with_entry (D_("Search for:"),
		myIcon,	myContainer,
		"same icon",
		NULL,
		(CairoDockActionOnAnswerFunc) _on_active_search, NULL, (GFreeFunc) NULL);
}

static void _on_active_search_tag (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	CD_APPLET_ENTER;
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		const gchar *cContent = gtk_entry_get_text (GTK_ENTRY (pInteractiveWidget));
		if (cContent != NULL)
		{
			cd_tomboy_reset_icon_marks (FALSE);
			GList *pList = cd_tomboy_find_notes_with_tag (cContent);
			cd_tomboy_show_results (pList);
			g_list_free (pList);
		}
	}
	CD_APPLET_LEAVE ();
}
static void _cd_tomboy_search_for_tag (GtkMenuItem *menu_item, gpointer data)
{
	cairo_dock_show_dialog_with_entry (D_("Search for tag:"),
		myIcon,	myContainer,
		"same icon",
		NULL,
		(CairoDockActionOnAnswerFunc) _on_active_search_tag, NULL, (GFreeFunc) NULL);
}

static void _cd_tomboy_search_for_today (GtkMenuItem *menu_item, gpointer data)
{
	cd_tomboy_reset_icon_marks (FALSE);
	GList *pList = cd_tomboy_find_note_for_today ();
	cd_tomboy_show_results (pList);
	g_list_free (pList);
}

static void _cd_tomboy_search_for_this_week (GtkMenuItem *menu_item, gpointer data)
{
	cd_tomboy_reset_icon_marks (FALSE);
	GList *pList = cd_tomboy_find_note_for_this_week ();
	cd_tomboy_show_results (pList);
	g_list_free (pList);
}

static void _cd_tomboy_search_for_next_week (GtkMenuItem *menu_item, gpointer data)
{
	cd_tomboy_reset_icon_marks (FALSE);
	GList *pList = cd_tomboy_find_note_for_next_week ();
	cd_tomboy_show_results (pList);
	g_list_free (pList);
}

static void _cd_tomboy_reset_marks (GtkMenuItem *menu_item, gpointer data)
{
	cd_tomboy_reset_icon_marks (TRUE);
}

CD_APPLET_ON_BUILD_MENU_BEGIN
	gboolean bClickOnNotes = (pClickedIcon !=  myIcon);
	
	gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Add a note"), D_("middle-click"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GTK_STOCK_ADD, _cd_tomboy_add_note, CD_APPLET_MY_MENU);
	g_free (cLabel);
	
	if (bClickOnNotes && pClickedIcon != NULL)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Delete this note"), GTK_STOCK_REMOVE, _cd_tomboy_delete_note, CD_APPLET_MY_MENU, pClickedIcon);
	}
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Reload notes"), GTK_STOCK_REFRESH, _cd_tomboy_reload_notes, CD_APPLET_MY_MENU);
	
	CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK  (D_("Search"), GTK_STOCK_FIND, _cd_tomboy_search_for_content, CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU(D_("Search for tag"), _cd_tomboy_search_for_tag, CD_APPLET_MY_MENU);
	
	CD_APPLET_ADD_IN_MENU(D_("Search for today's note"), _cd_tomboy_search_for_today, CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU(D_("Search for this week's note"), _cd_tomboy_search_for_this_week, CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU(D_("Search for next week's note"), _cd_tomboy_search_for_next_week, CD_APPLET_MY_MENU);
	
	GList *pList = (myDock ? (myIcon->pSubDock ? myIcon->pSubDock->icons : NULL) : myDesklet->icons);
	Icon *icon;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (icon->bHasIndicator)
		{
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Reset marks"), GTK_STOCK_CLEAR, _cd_tomboy_reset_marks, CD_APPLET_MY_MENU);
			break ;
		}
	}
	
	if (bClickOnNotes && pClickedIcon != NULL)
		CD_APPLET_LEAVE (CAIRO_DOCK_INTERCEPT_NOTIFICATION);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (pClickedIcon == myIcon && ! myData.bIsRunning)  // possible si on l'a quitte apres le demarrage de l'applet.
	{
		cd_notes_run_manager ();
	}
	else
	{
		_cd_tomboy_create_new_note ();
	}
CD_APPLET_ON_MIDDLE_CLICK_END



static gboolean _popup_dialog (Icon *pIcon)
{
	CD_APPLET_ENTER;
	CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	if (pContainer->bInside)
	{
		if (g_list_find (CD_APPLET_MY_ICONS_LIST, pIcon))  // on verifie que l'icone ne s'est pas fait effacee entre-temps.
			cairo_dock_show_temporary_dialog_with_icon (pIcon->cClass,
				pIcon,
				CD_APPLET_MY_ICONS_LIST_CONTAINER,
				myConfig.iDialogDuration,
				myConfig.cIconDefault != NULL ? myConfig.cIconDefault : MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
	myData.iSidPopupDialog = 0;
	CD_APPLET_LEAVE (FALSE);
}
gboolean cd_tomboy_on_change_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation)
{
	CD_APPLET_ENTER;
	if (myData.iSidPopupDialog != 0)
	{
		g_source_remove (myData.iSidPopupDialog);
		myData.iSidPopupDialog = 0;
	}
	
	GList *pList = CD_APPLET_MY_ICONS_LIST;
	Icon *icon;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cairo_dock_remove_dialog_if_any (icon);
	}
	
	if (pIcon && pIcon->bPointed)
	{
		myData.iSidPopupDialog = g_timeout_add (500, (GSourceFunc)_popup_dialog, pIcon);
	}
	
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}
