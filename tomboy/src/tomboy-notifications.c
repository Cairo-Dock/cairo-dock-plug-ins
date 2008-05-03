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
			update_icon();
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
	reload_all_notes ();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_IN_MENU(D_("Add a note"), _cd_tomboy_add_note, CD_APPLET_MY_MENU)
	CD_APPLET_ADD_IN_MENU(D_("Reload notes"), _cd_tomboy_reload_notes, CD_APPLET_MY_MENU)
	if (myDock != NULL && myIcon->pSubDock != NULL && pClickedContainer == CAIRO_CONTAINER (myIcon->pSubDock))
	{
		if (pClickedIcon != NULL && pClickedIcon !=  myIcon)
		{
			CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("Delete this note"), _cd_tomboy_delete_note, CD_APPLET_MY_MENU, pClickedIcon) 
		}
		CD_APPLET_ADD_ABOUT_IN_MENU (CD_APPLET_MY_MENU)
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (CD_APPLET_MY_MENU)
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (pClickedIcon == myIcon && ! myData.opening)  // possible si on l'a quitte apres le demarrage de l'applet.
	{
		dbus_detect_tomboy();
		getAllNotes();
		update_icon();
	}
	else
		_cd_tomboy_create_new_note (pClickedIcon);
CD_APPLET_ON_MIDDLE_CLICK_END
