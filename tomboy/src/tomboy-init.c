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

#include "applet-notes.h"
#include "applet-backend-tomboy.h"
#include "applet-backend-default.h"
#include "tomboy-draw.h"
#include "tomboy-config.h"
#include "tomboy-notifications.h"
#include "tomboy-struct.h"
#include "tomboy-init.h"


CD_APPLET_DEFINITION (N_("Note-Taking"),
	1, 5, 4,
	CAIRO_DOCK_CATEGORY_APPLET_ACCESSORY,
	N_("Control your Gnote or TomBoy's notes directly in the dock !\n"
	"Click on a note to open it, Escape to close it.\n"
	"Middle-click to instantly create a new note.\n"
	"You can search inside notes and display their content on the icons."),
	"Necropotame (Adrien Pilleboue)")

CD_APPLET_INIT_BEGIN
	myData.hNoteTable = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		NULL,  // l'URI est partage avec l'icone.
		(GDestroyNotify) NULL);  // on detruit les icones nous-memes.
	
	cd_notes_start ();
	
	//Enregistrement des notifications
	gldi_object_register_notification (&myContainerObjectMgr,
		NOTIFICATION_CLICK_ICON,
		(GldiNotificationFunc) CD_APPLET_ON_CLICK_FUNC,
		GLDI_RUN_FIRST, myApplet);  // ici on s'enregistre explicitement avant le dock, pour pas qu'il essaye de lancer nos notes.
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	gldi_object_remove_notification (CD_APPLET_MY_ICONS_LIST_CONTAINER,
		NOTIFICATION_ENTER_ICON,
		(GldiNotificationFunc) cd_tomboy_on_change_icon,
		myApplet);
	gldi_object_remove_notification (CD_APPLET_MY_ICONS_LIST_CONTAINER,
		myDock ? NOTIFICATION_LEAVE_DOCK : NOTIFICATION_LEAVE_DESKLET,
		(GldiNotificationFunc) cd_tomboy_on_leave_container,
		myApplet);
	
	if (myData.iSidResetQuickInfo != 0)
		g_source_remove (myData.iSidResetQuickInfo);
	if (myData.iSidPopupDialog != 0)
		g_source_remove (myData.iSidPopupDialog);
	
	cd_notes_stop ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les parametres qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		myData.iIconState = 0;
		
		if (myData.iSidResetQuickInfo != 0)
		{
			g_source_remove (myData.iSidResetQuickInfo);
			myData.iSidResetQuickInfo = 0;
		}
		if (myData.iSidPopupDialog != 0)
		{
			g_source_remove (myData.iSidPopupDialog);
			myData.iSidPopupDialog = 0;
		}
		
		cd_notes_stop ();
		
		cd_notes_start ();
	}
CD_APPLET_RELOAD_END
