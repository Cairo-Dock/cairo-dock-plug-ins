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


CD_APPLET_DEFINE_BEGIN (N_("systray"),
	2, 2, 0,
	CAIRO_DOCK_CATEGORY_APPLET_DESKTOP,
	N_("Add a systray to your dock.\n"
	"Left-click to show the systray in a dialog (you can bind a keyboard shortcut for it.)\n"
	"Middle-click to close the dalog.\n"
	"But the best way to use it id to detach it from the dock, and place it somewhere, above other windows."),
	"Ctaf (Cedric Gestes)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_SET_UNRESIZABLE_DESKLET
CD_APPLET_DEFINE_END


CD_APPLET_INIT_BEGIN

	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	
	cd_systray_check_running ();  // petit message si l'utilisateur active le systray alors qu'un autre systray existe.
	
	cd_systray_build_systray (); // on construit le systray immediatement pour les applis qui ne peuvent exister sans.
	
	if (myDesklet)  // le desklet ne tourne pas.
	{
		CD_APPLET_SET_STATIC_DESKLET;
	}
	else  // en mode desklet on n'a pas besoin de l'icone.
	{
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
	}

CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN

	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;

CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN

	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (! myData.tray)  // ne devrait pas arriver.
		{
			cd_systray_build_systray ();
		}
		else
		{
			// changement de l'orientation.
			cd_systray_set_orientation (myConfig.iIconPacking == 0 ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);
			
			// on remet le raccourci.
			systray_set_shortcut();
			
			// changement de container.
			if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED)
			{
				if (myDesklet != NULL)  // il faut passer du dialogue au desklet.
				{
					cairo_dock_steal_interactive_widget_from_dialog (myData.dialog);
					cairo_dock_dialog_unreference (myData.dialog);
					myData.dialog = NULL;
					cairo_dock_add_interactive_widget_to_desklet (GTK_WIDGET (myData.tray), myDesklet);
					CD_APPLET_SET_DESKLET_RENDERER (NULL);  // pour empecher le clignotement du au double-buffer.
					CD_APPLET_SET_STATIC_DESKLET;
				}
				else  // il faut passer du desklet au dialogue
				{
					CairoDesklet *pDesklet = CAIRO_DESKLET (CD_APPLET_MY_OLD_CONTAINER);
					cairo_dock_steal_interactive_widget_from_desklet (pDesklet);
					cd_systray_build_dialog ();
				}
				g_object_unref (G_OBJECT (myData.tray));  // le 'steal' a ajoute une reference, et l'insertion dans le container aussi.
			}
		}
	}
	
	if (myDesklet)  // on cloue le desklet.
	{
		GdkGravity iGravity;
		if (myContainer->iWindowPositionX < g_desktopGeometry.iScreenWidth[CAIRO_DOCK_HORIZONTAL]/2)  // on prend la taille de l'ecran ou il est place plutot que la taille totale, car en general on n'utilise pas un dual-screen comme un seul grand ecran maisplutot comme 2 bureaux.
			iGravity = GDK_GRAVITY_NORTH_WEST;
		else
			iGravity = GDK_GRAVITY_NORTH_EAST;
		gtk_window_set_gravity (GTK_WINDOW (myContainer->pWidget), iGravity);
	}

CD_APPLET_RELOAD_END
