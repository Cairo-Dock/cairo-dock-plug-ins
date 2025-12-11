/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
** Login : <ctaf42@gmail.com>
** Started on  Fri Nov 30 05:31:31 2007 GESTES Cedric
** $Id$
**
** Copyright (C) 2007 GESTES Cedric
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <stdlib.h>

#include "systray-config.h"
#include "systray-interface.h"
#include "systray-notifications.h"
#include "systray-struct.h"
#include "systray-init.h"


CD_APPLET_DEFINE2_BEGIN ("systray",
	CAIRO_DOCK_MODULE_SUPPORTS_X11,
	CAIRO_DOCK_CATEGORY_APPLET_DESKTOP,
	N_("Add a systray to your dock.\n"
	"Left-click to show/hide the systray in a dialog (you can bind a keyboard shortcut for it.)\n"
	"But the best way to use it is to detach it from the dock, and place it somewhere, above other windows."),
	"Ctaf (Cedric Gestes)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_SET_UNRESIZABLE_DESKLET
	CD_APPLET_REDEFINE_TITLE (N_("Notification Area Old"))
CD_APPLET_DEFINE2_END


CD_APPLET_INIT_BEGIN

	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	// CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	
	cd_systray_build_systray (); // on construit le systray immediatement pour les applis qui ne peuvent exister sans.
	
	if (myDesklet)  // le desklet ne tourne pas.
	{
		CD_APPLET_SET_STATIC_DESKLET;
	}
	else  // en mode desklet on n'a pas besoin de l'icone.
	{
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
	}
	
	myData.cKeyBinding = CD_APPLET_BIND_KEY (myConfig.shortcut,
		D_("Show/hide the systray"),
		"Configuration", "shortkey",
		(CDBindkeyHandler) systray_on_keybinding_pull);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN

	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	// CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	gldi_object_unref (GLDI_OBJECT(myData.cKeyBinding));
	
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN

	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		// changement de l'orientation.
		if (myData.tray)
			cd_systray_set_orientation (myConfig.iIconPacking == 0 ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);
		
		// changement de container.
		if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED)
		{
			if (myDesklet != NULL)  // il faut passer du dialogue au desklet.
			{
				if (myData.tray) gldi_dialog_steal_interactive_widget (myData.dialog);
				gldi_object_unref (GLDI_OBJECT(myData.dialog));
				myData.dialog = NULL;
				cd_systray_build_desklet ();
			}
			else  // il faut passer du desklet au dialogue
			{
				CairoDesklet *pDesklet = CAIRO_DESKLET (CD_APPLET_MY_OLD_CONTAINER);
				if (myData.tray) gldi_desklet_steal_interactive_widget (pDesklet);
				cd_systray_build_dialog ();
			}
			if (myData.tray) g_object_unref (G_OBJECT (myData.tray));  // le 'steal' a ajoute une reference, et l'insertion dans le container aussi.
		}
		
		if (myDock)
			CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
		
		gldi_shortkey_rebind (myData.cKeyBinding, myConfig.shortcut, NULL);
	}
	
/*	if (myDesklet)  // on cloue le desklet. -- does not work properly with how desklets are positioned
	{
		GdkGravity iGravity;
		if (myContainer->iWindowPositionX < g_desktopGeometry.Xscreen.width/2)  // we don't know if the container is set on a given screen or not, so take the X screen.
			iGravity = GDK_GRAVITY_NORTH_WEST;
		else
			iGravity = GDK_GRAVITY_NORTH_EAST;
		gtk_window_set_gravity (GTK_WINDOW (myContainer->pWidget), iGravity);
	} */

CD_APPLET_RELOAD_END
