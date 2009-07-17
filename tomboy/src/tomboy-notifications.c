#include <stdlib.h>
#include <glib/gi18n.h>
#define __USE_POSIX
#include <time.h>

#include "tomboy-dbus.h"
#include "tomboy-draw.h"
#include "tomboy-struct.h"
#include "tomboy-notifications.h"


CD_APPLET_ON_CLICK_BEGIN
	if (pClickedIcon != NULL && pClickedIcon != myIcon)
	{
		cd_message("tomboy : %s",pClickedIcon->acCommand);
		showNote(pClickedIcon->acCommand);
	}
	else if (pClickedIcon == myIcon && ! myData.opening)  // possible si on l'a quitte apres le demarrage de l'applet.
	{
		dbus_detect_tomboy();
		if (! myData.opening)
		{
			dbus_detect_tomboy();
			free_all_notes ();
			getAllNotes();
			cd_tomboy_load_notes();
		}
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_CLICK_END


static void _cd_tomboy_create_new_note (void)
{
	gchar *note_title;
	if (myConfig.bAutoNaming)
	{
		g_print ("on nomme automatiquement cette note\n");
		note_title = g_new0 (gchar, 50+1);
		time_t epoch = (time_t) time (NULL);
		struct tm currentTime;
		localtime_r (&epoch, &currentTime);
		strftime (note_title, 50, "%a-%d-%b_%r", &currentTime);
	}
	else
	{
		g_print ("on demande le nom de la nouvelle note ...\n");
		note_title = cairo_dock_show_demand_and_wait (D_("Note name : "),
			myIcon,
			myContainer,
			NULL);
		g_print ("on a recu '%s'\n", note_title);
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
		gchar *cQuestion = g_strdup_printf ("%s (%s)", D_("Delete this note ?"), pIcon->acName);
		int iAnswer = cairo_dock_ask_question_and_wait (cQuestion, pIcon, myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer);
		g_free (cQuestion);
		if (iAnswer != GTK_RESPONSE_YES)
			return ;
	}
	deleteNote (pIcon->acCommand);
}
static void _cd_tomboy_reload_notes (GtkMenuItem *menu_item, gpointer data)
{
	free_all_notes ();
	getAllNotes();
	cd_tomboy_load_notes();
}
static void _cd_tomboy_search_for_content (GtkMenuItem *menu_item, gpointer data)
{
	gchar *cContent = cairo_dock_show_demand_and_wait (D_("Search for :"),
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
	gchar *cTag = cairo_dock_show_demand_and_wait (D_("Search for tag :"),
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
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Reload notes"), GTK_STOCK_REFRESH, _cd_tomboy_reload_notes, CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Add a note"), GTK_STOCK_ADD, _cd_tomboy_add_note, CD_APPLET_MY_MENU);
	
	if (pClickedContainer == CAIRO_CONTAINER (myIcon->pSubDock) || myDesklet)
	{
		if (pClickedIcon != NULL && pClickedIcon !=  myIcon)
		{
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Delete this note"), GTK_STOCK_REMOVE, _cd_tomboy_delete_note, CD_APPLET_MY_MENU, pClickedIcon);
		}
		
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
				CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Clear marks"), GTK_STOCK_CLEAR, _cd_tomboy_reset_marks, CD_APPLET_MY_MENU);
				break ;
			}
		}
		if (pClickedIcon != NULL)
			return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (CD_APPLET_MY_MENU);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (pClickedIcon == myIcon && ! myData.opening)  // possible si on l'a quitte apres le demarrage de l'applet.
	{
		dbus_detect_tomboy();
		free_all_notes ();
		getAllNotes();
		cd_tomboy_load_notes();
	}
	else
		_cd_tomboy_create_new_note ();
CD_APPLET_ON_MIDDLE_CLICK_END




gboolean cd_tomboy_on_change_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation)
{
	GList *pList = CD_APPLET_MY_ICONS_LIST;
	Icon *icon;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		cairo_dock_remove_dialog_if_any (icon);
	}
	
	if (pIcon->bPointed)
		cairo_dock_show_temporary_dialog_with_icon (pIcon->cClass, pIcon, CD_APPLET_MY_ICONS_LIST_CONTAINER, 8000, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
