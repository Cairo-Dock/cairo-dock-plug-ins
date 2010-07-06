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

#include "stdlib.h"

#include "terminal-config.h"
#include "terminal-menu-functions.h"
#include "terminal-struct.h"
#include "terminal-widget.h"
#include "terminal-init.h"


CD_APPLET_DEFINITION ("terminal",
	2, 0, 7,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("Add a terminal to your dock !\n"
	"Left/middle click to show/hide terminal (you can bind a keyboard shortcut for it.)\n"
	"You can drag'n'drop files or text and select an action.\n"
	"You can open many tabs, rename them, change their color (right-click on a tab to interact on it)\n"
	"To open a new tab : type 'CTRL+t' or double-click next to the last tab\n"
	"To close a tab : type 'CTRL+w' or middle-click on it."),
	"Ctaf (Cedric Gestes) &amp; Fabounet (Fabrice Rey)")


CD_APPLET_INIT_BEGIN
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;

	if (myDesklet)  // on cree le terminal pour avoir qqch a afficher dans le desklet.
	{
		terminal_build_and_show_tab ();
		CD_APPLET_SET_STATIC_DESKLET;
	}
	else  // en mode desklet, on n'a pas besoin de l'icone.
	{
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	}
	
	gboolean bKeyBinded = cd_keybinder_bind (myConfig.shortcut, (CDBindkeyHandler)term_on_keybinding_pull, (gpointer)NULL);
	if (! bKeyBinded)  // si le bind n'a pas eu lieu, on s'en souvient. En effet, on a besoin de savoir si on pourra rappeler le desklet lors d'un 'exit' dans le dernier onglet pour cacher ou non le desklet.
	{
		g_free (myConfig.shortcut);
		myConfig.shortcut = NULL;
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
		if (! myData.tab)
		{
			if (myDesklet)  // on cree le terminal pour avoir qqch a afficher dans le desklet.
				terminal_build_and_show_tab ();
		}
		else if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED)
		{
			if (myDesklet)  // il faut passer du dialogue au desklet.
			{
				// on retire le terminal du dialogue.
				myData.tab = cairo_dock_steal_interactive_widget_from_dialog (myData.dialog);
				cairo_dock_dialog_unreference (myData.dialog);
				myData.dialog = NULL;
				
				// on l'insere dans le desklet.
				cairo_dock_add_interactive_widget_to_desklet (myData.tab, myDesklet);
				g_object_unref (myData.tab);  // le 'steal' a rajoute une reference.
				CD_APPLET_SET_DESKLET_RENDERER (NULL);  // pour empecher le clignotement du au double-buffer.
				CD_APPLET_SET_STATIC_DESKLET;
			}
			else  // il faut passer du desklet au dialogue
			{
				// on retire le terminal du desklet.
				myData.tab = cairo_dock_steal_interactive_widget_from_desklet (CAIRO_DESKLET (CD_APPLET_MY_OLD_CONTAINER));
				///myData.tab = cairo_dock_steal_widget_from_its_container (myData.tab);
				myData.dialog = cd_terminal_build_dialog ();
				g_object_unref (myData.tab);  // le 'steal' a rajoute une reference.
				cairo_dock_hide_dialog (myData.dialog);
			}
		}
		
		if (myData.tab)
		{
			term_apply_settings();
		}
		
		if (myDock)  // en mode desklet, on n'a pas besoin de l'icone.
		{
			CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
		}
	}
CD_APPLET_RELOAD_END
