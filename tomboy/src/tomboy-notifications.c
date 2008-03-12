#include <stdlib.h>
#include <glib/gi18n.h>

#include "tomboy-dbus.h"
#include "tomboy-draw.h"
#include "tomboy-struct.h"
#include "tomboy-notifications.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;

CD_APPLET_ABOUT (_D("Applet by Necropotame (Adrien Pilleboue)"))

CD_APPLET_ON_CLICK_BEGIN
	if (myDock != NULL && myIcon->pSubDock != NULL && pClickedContainer == CAIRO_DOCK_CONTAINER (myIcon->pSubDock))
	{
		cd_message("tomboy : %s\n",pClickedIcon->cBaseURI);
		showNote(pClickedIcon->cBaseURI);
	}
	else return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_CLICK_END

CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_ABOUT_IN_MENU (CD_APPLET_MY_MENU)
CD_APPLET_ON_BUILD_MENU_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	gchar *note_title = cairo_dock_show_demand_and_wait (_("Note name : "),
		myIcon,
		myContainer,
		NULL);
	gchar *note_name = addNote(note_title);
	showNote(note_name);
CD_APPLET_ON_MIDDLE_CLICK_END
