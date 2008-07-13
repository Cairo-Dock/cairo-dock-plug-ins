/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
** Login : <chris.chapuis@gmail.com>
** Started on  Thu Apr 03 18:21:35 2008 CHAPUIS Christophe
** $Id$
**
** Author(s):
**  - Christophe CHAPUIS <chris.chapuis@gmail.com>
**
** Copyright (C) 2008 CHAPUIS Christophe
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

#include "applet-struct.h"
#include "applet-widget.h"

#include "gtkmozembed.h"
#include "applet-widget-itf.h"

CD_APPLET_INCLUDE_MY_VARS

void load_started_cb(GtkMozEmbed *embed, void *data);
void load_finished_cb(GtkMozEmbed *embed, void *data);

void weblet_build_and_show(void)
{
	myData.pGtkMozEmbed = gtk_moz_embed_new();
	gtk_widget_show(myData.pGtkMozEmbed);
	gtk_signal_connect(GTK_OBJECT(myData.pGtkMozEmbed), "net_start",
					 					 GTK_SIGNAL_FUNC(load_started_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(myData.pGtkMozEmbed), "net_stop",
					 					 GTK_SIGNAL_FUNC(load_finished_cb), NULL);
	// register for right-click on the moz-embed widget
	register_menu_cb( myData.pGtkMozEmbed );

	if (myDock)
	{
		myData.dialog = cairo_dock_build_dialog (D_("Terminal"), myIcon, myContainer, NULL, myData.pGtkMozEmbed, GTK_BUTTONS_NONE, NULL, NULL, NULL);
	}
	else
	{
		cairo_dock_add_interactive_widget_to_desklet (myData.pGtkMozEmbed, myDesklet);
		
		cairo_dock_set_desklet_renderer_by_name (myDesklet, NULL, NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
	}
}

gboolean cd_weblets_refresh_page (void)
{
		// load the page
		if(myData.pGtkMozEmbed)
		{
			gtk_moz_embed_load_url(GTK_MOZ_EMBED(myData.pGtkMozEmbed), myConfig.cURI_to_load?myConfig.cURI_to_load:"http://www.google.com");
		}

		return TRUE;
}

void load_started_cb(GtkMozEmbed *embed, void *data)
{
	// update scrollbars status
	show_hide_scrollbars();
}

void load_finished_cb(GtkMozEmbed *embed, void *data)
{
	// update scrollbars status
	show_hide_scrollbars();
}

// hide/show the scrollbars
void show_hide_scrollbars()
{
    set_gecko_scrollbars( myData.pGtkMozEmbed, myConfig.bShowScrollbars, myConfig.iPosScrollX , myConfig.iPosScrollY );
}

void send_mouse_click_to_cd( GdkEventButton* pButtonEvent )
{
	if( myDock )
	{
		on_button_press2(myData.pGtkMozEmbed, pButtonEvent, myDock );
	}
	else if( myDesklet )
	{
		Icon *pClickedIcon = cairo_dock_find_clicked_icon_in_desklet (myDesklet);
		GtkWidget *menu = cairo_dock_build_menu (pClickedIcon, CAIRO_CONTAINER (myDesklet));  // genere un CAIRO_DOCK_BUILD_MENU.
		gtk_widget_show_all (menu);
		gtk_menu_popup (GTK_MENU (menu),
			NULL,
			NULL,
			NULL,
			NULL,
			1,
			gtk_get_current_event_time ());
		myDesklet->bInside = FALSE;
		myDesklet->iGradationCount = 0;  // on force le fond a redevenir transparent.
		gtk_widget_queue_draw (myDesklet->pWidget);
	}
}
