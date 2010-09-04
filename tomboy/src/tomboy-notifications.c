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

#include "tomboy-dbus.h"
#include "tomboy-draw.h"
#include "tomboy-struct.h"
#include "tomboy-notifications.h"


static void _launch_tomboy (void)
{
	cd_debug ("");
	dbus_detect_tomboy();
	if (! myData.bIsRunning)
	{
		const gchar *cName = "";
		switch (myConfig.iAppControlled)
		{
			case CD_NOTES_TOMBOY:
			default:
				cName = "Tomboy";
			break;
			case CD_NOTES_GNOTES:
				cName = "Gnote";
			break;
		}
		cairo_dock_show_temporary_dialog_with_icon_printf ("Launching %s...",
			myIcon, myContainer,
			2000,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			cName);
		cairo_dock_launch_command ("tomboy &");
		dbus_detect_tomboy_async ();
	}
	else
	{
		free_all_notes ();
		getAllNotes_async ();
	}
}

CD_APPLET_ON_CLICK_BEGIN
	if (pClickedIcon != NULL && pClickedIcon != myIcon)
	{
		cd_message("tomboy : %s",pClickedIcon->cCommand);
		showNote (pClickedIcon->cCommand);
	}
	else if (pClickedIcon == myIcon && ! myData.bIsRunning)  // possible si on l'a quitte apres le demarrage de l'applet.
	{
		_launch_tomboy ();
	}
	else
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
CD_APPLET_ON_CLICK_END


static void _cd_tomboy_create_new_note (void)
{
	gchar *note_title;
	if (myConfig.bAutoNaming)
	{
		cd_debug ("on nomme automatiquement cette note");
		note_title = g_new0 (gchar, 50+1);
		time_t epoch = (time_t) time (NULL);
		struct tm currentTime;
		localtime_r (&epoch, &currentTime);
		strftime (note_title, 50, "%a-%d-%b_%r", &currentTime);
	}
	else
	{
		cd_debug ("on demande le nom de la nouvelle note ...");
		note_title = cairo_dock_show_demand_and_wait (D_("Note name : "),
			myIcon,
			myContainer,
			NULL);
		cd_debug ("on a recu '%s'", note_title);
	}
	cd_message ("%s (%s)", __func__, note_title);
	gchar *note_name = addNote(note_title);
	cd_debug (" note_name <- %s", note_name);
	showNote(note_name);
	g_free (note_name);
	g_free (note_title);
}
static void _cd_tomboy_add_note (GtkMenuItem *menu_item, gpointer data)
{
	_cd_tomboy_create_new_note ();
}
static void _cd_tomboy_delete_note (GtkMenuItem *menu_item, Icon *pIcon)
{
	if (pIcon == NULL)
		return ;
	if (myConfig.bAskBeforeDelete)
	{
		gchar *cQuestion = g_strdup_printf ("%s (%s)", D_("Delete this note?"), pIcon->cName);
		int iAnswer = cairo_dock_ask_question_and_wait (cQuestion, pIcon, myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer);
		g_free (cQuestion);
		if (iAnswer != GTK_RESPONSE_YES)
			return ;
	}
	deleteNote (pIcon->cCommand);
}
static void _cd_tomboy_reload_notes (GtkMenuItem *menu_item, gpointer data)
{
	free_all_notes ();
	getAllNotes_async ();
}
static void _cd_tomboy_search_for_content (GtkMenuItem *menu_item, gpointer data)
{
	gchar *cContent = cairo_dock_show_demand_and_wait (D_("Search for:"),
		myIcon,
		myContainer,
		NULL);
	if (cContent != NULL)
	{
		cd_tomboy_reset_icon_marks (FALSE);
		gchar *cContents[2] = {cContent, NULL};
		GList *pList = cd_tomboy_find_notes_with_contents (cContents);
		g_free (cContent);
		cd_tomboy_show_results (pList);
		g_list_free (pList);
	}
}
static void _cd_tomboy_search_for_tag (GtkMenuItem *menu_item, gpointer data)
{
	gchar *cTag = cairo_dock_show_demand_and_wait (D_("Search for tag:"),
		myIcon,
		myContainer,
		NULL);
	if (cTag != NULL)
	{
		cd_tomboy_reset_icon_marks (FALSE);
		GList *pList = cd_tomboy_find_notes_with_tag (cTag);
		g_free (cTag);
		cd_tomboy_show_results (pList);
		g_list_free (pList);
	}
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
	
	// Main Menu
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Add a note"), GTK_STOCK_ADD, _cd_tomboy_add_note, CD_APPLET_MY_MENU);
	
	if (bClickOnNotes && pClickedIcon != NULL)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Delete this note"), GTK_STOCK_REMOVE, _cd_tomboy_delete_note, CD_APPLET_MY_MENU, pClickedIcon);
	}
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Reload notes"), GTK_STOCK_REFRESH, _cd_tomboy_reload_notes, CD_APPLET_MY_MENU);
		
	if (bClickOnNotes)  // on ne le fait pas pour un clic sur myIcon, car le sous-dock gene le dialogue.
	{
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
	}
	CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
	CD_APPLET_ADD_ABOUT_IN_MENU (CD_APPLET_MY_MENU);
	
	if (bClickOnNotes && pClickedIcon != NULL)
		CD_APPLET_LEAVE (CAIRO_DOCK_INTERCEPT_NOTIFICATION);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (pClickedIcon == myIcon && ! myData.bIsRunning)  // possible si on l'a quitte apres le demarrage de l'applet.
	{
		_launch_tomboy ();
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
	
	if (pIcon->bPointed)
	{
		myData.iSidPopupDialog = g_timeout_add (500, (GSourceFunc)_popup_dialog, pIcon);
	}
	
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}
