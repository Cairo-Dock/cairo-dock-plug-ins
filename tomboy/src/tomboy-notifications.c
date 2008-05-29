#include <stdlib.h>
#include <glib/gi18n.h>

#include "tomboy-dbus.h"
#include "tomboy-draw.h"
#include "tomboy-struct.h"
#include "tomboy-notifications.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("Applet by Necropotame (Adrien Pilleboue)"))


CD_APPLET_ON_CLICK_BEGIN
	if (pClickedIcon != NULL && pClickedIcon != myIcon)
	{
		cd_message("tomboy : %s",pClickedIcon->acCommand);
		showNote(pClickedIcon->acCommand);
		///return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	else if (pClickedIcon == myIcon && ! myData.opening)  // possible si on l'a quitte apres le demarrage de l'applet.
	{
		dbus_detect_tomboy();
		if (! myData.opening)
		{
			dbus_detect_tomboy();
			getAllNotes();
			cd_tomboy_load_notes();
		}
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_CLICK_END


static void _cd_tomboy_create_new_note (Icon *pIcon)
{
	gchar *note_title = cairo_dock_show_demand_and_wait (D_("Note name : "),
		(pIcon != NULL ? pIcon : myIcon),
		(pIcon != NULL && myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer),
		NULL);
	gchar *note_name = addNote(note_title);
	showNote(note_name);
	g_free (note_name);
}
static void _cd_tomboy_add_note (GtkMenuItem *menu_item, Icon *pIcon)
{
	_cd_tomboy_create_new_note (pIcon);
}
static void _cd_tomboy_delete_note (GtkMenuItem *menu_item, Icon *pIcon)
{
	deleteNote (pIcon->acCommand);
}
static void _cd_tomboy_reload_notes (GtkMenuItem *menu_item, Icon *pIcon)
{
	getAllNotes();
	cd_tomboy_load_notes();
}
static void _cd_tomboy_search_for_content (GtkMenuItem *menu_item, Icon *pIcon)
{
	gchar *cContent = cairo_dock_show_demand_and_wait (D_("Search for :"),
		(pIcon != NULL ? pIcon : myIcon),
		(pIcon != NULL && myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer),
		NULL);
	if (cContent != NULL)
	{
		cd_tomboy_reset_icon_marks (FALSE);
		gchar *cContents[2] = {cContent, NULL};
		GList *pList = cd_tomboy_find_notes_with_contents (cContents);
		g_free (cContent);
		if (pList != NULL)
		{
			cd_tomboy_mark_icons (pList, TRUE);
			g_list_free (pList);
			if (myDock)
				cairo_dock_show_dock_at_mouse (myIcon->pSubDock);
		}
	}
}
static void _cd_tomboy_search_for_tag (GtkMenuItem *menu_item, Icon *pIcon)
{
	gchar *cTag = cairo_dock_show_demand_and_wait (D_("Search for tag :"),
		(pIcon != NULL ? pIcon : myIcon),
		(pIcon != NULL && myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer),
		NULL);
	if (cTag != NULL)
	{
		cd_tomboy_reset_icon_marks (FALSE);
		GList *pList = cd_tomboy_find_notes_with_tag (cTag);
		g_free (cTag);
		if (pList != NULL)
		{
			cd_tomboy_mark_icons (pList, TRUE);
			g_list_free (pList);
			if (myDock)
				cairo_dock_show_dock_at_mouse (myIcon->pSubDock);
		}
	}
}
static void _cd_tomboy_search_for_today (GtkMenuItem *menu_item, Icon *pIcon)
{
	GList *pList = cd_tomboy_find_note_for_today ();
	if (pList != NULL)
	{
		cd_tomboy_mark_icons (pList, TRUE);
		g_list_free (pList);
		if (myDock)
			cairo_dock_show_dock_at_mouse (myIcon->pSubDock);
	}
}
static void _cd_tomboy_search_for_this_week (GtkMenuItem *menu_item, Icon *pIcon)
{
	GList *pList = cd_tomboy_find_note_for_this_week ();
	if (pList != NULL)
	{
		cd_tomboy_mark_icons (pList, TRUE);
		g_list_free (pList);
		if (myDock)
			cairo_dock_show_dock_at_mouse (myIcon->pSubDock);
	}
}
static void _cd_tomboy_search_for_next_week (GtkMenuItem *menu_item, Icon *pIcon)
{
	GList *pList = cd_tomboy_find_note_for_next_week ();
	if (pList != NULL)
	{
		cd_tomboy_mark_icons (pList, TRUE);
		g_list_free (pList);
		if (myDock)
			cairo_dock_show_dock_at_mouse (myIcon->pSubDock);
	}
}
static void _cd_tomboy_reset_marks (GtkMenuItem *menu_item, Icon *pIcon)
{
	cd_tomboy_reset_icon_marks (TRUE);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_IN_MENU(D_("Reload notes"), _cd_tomboy_reload_notes, CD_APPLET_MY_MENU)
	if (myDock && (myIcon->pSubDock != NULL && pClickedContainer == CAIRO_CONTAINER (myIcon->pSubDock)) || (myIcon->pSubDock == NULL && pClickedContainer == myContainer))
	{
		CD_APPLET_ADD_IN_MENU(D_("Add a note"), _cd_tomboy_add_note, CD_APPLET_MY_MENU)
		
		if (pClickedIcon != NULL && pClickedIcon !=  myIcon)
		{
			CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("Delete this note"), _cd_tomboy_delete_note, CD_APPLET_MY_MENU, pClickedIcon) 
		}
		
		CD_APPLET_ADD_IN_MENU(D_("Search"), _cd_tomboy_search_for_content, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU(D_("Searh for tag"), _cd_tomboy_search_for_tag, CD_APPLET_MY_MENU)
		
		CD_APPLET_ADD_IN_MENU(D_("Search for today's note"), _cd_tomboy_search_for_today, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU(D_("Search for this week's note"), _cd_tomboy_search_for_this_week, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU(D_("Search for next week's note"), _cd_tomboy_search_for_next_week, CD_APPLET_MY_MENU)
		
		GList *pList = (myDock ? (myIcon->pSubDock ? myIcon->pSubDock->icons : NULL) : myDesklet->icons);
		Icon *icon;
		GList *ic;
		for (ic = pList; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->bHasIndicator)
			{
				CD_APPLET_ADD_IN_MENU(D_("Reset marks"), _cd_tomboy_reset_marks, CD_APPLET_MY_MENU)
				break ;
			}
		}
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (CD_APPLET_MY_MENU)
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (pClickedIcon == myIcon && ! myData.opening)  // possible si on l'a quitte apres le demarrage de l'applet.
	{
		dbus_detect_tomboy();
		getAllNotes();
		cd_tomboy_load_notes();
	}
	else
		_cd_tomboy_create_new_note (pClickedIcon);
CD_APPLET_ON_MIDDLE_CLICK_END
