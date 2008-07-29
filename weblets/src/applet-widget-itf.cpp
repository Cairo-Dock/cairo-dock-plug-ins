/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
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

#include "gtk/gtk.h"

#include "gtkmozembed.h"
#include "gtkmozembed_internal.h"
#include "applet-widget-itf.h"

#include <nsCOMPtr.h>
#include <nsIWebBrowser.h>
#include <nsIWebBrowserChrome.h>
#include <nsIDOMWindow.h>
#include <nsIDOMBarProp.h>
#include <nsIDOMMouseEvent.h>
#include <docshell/nsIScrollable.h>
#include <nsISupports.h>
#include <nsIServiceManager.h>


extern "C" void send_mouse_click_to_cd( GdkEventButton* pButtonEvent );
extern "C" gint dom_mouse_click_cb(GtkMozEmbed *embed, nsIDOMMouseEvent *event, GtkWidget *pGtkMozEmbed);

// hide/show the scrollbars
extern "C" gboolean set_gecko_scrollbars( GtkWidget *moz, gboolean bShowScrollbars, gint scrollX, gint scrollY )
{
  nsIWebBrowser *browser = NULL;
  nsresult res;

	gtk_moz_embed_get_nsIWebBrowser(GTK_MOZ_EMBED(moz), &browser);

	if( browser )
	{
		nsCOMPtr<nsIScrollable> scroller = do_QueryInterface(browser);
		if( scroller )
		{
			scroller->SetDefaultScrollbarPreferences ( nsIScrollable::ScrollOrientation_X , bShowScrollbars?nsIScrollable::Scrollbar_Auto:nsIScrollable::Scrollbar_Never );
			scroller->SetDefaultScrollbarPreferences ( nsIScrollable::ScrollOrientation_Y , bShowScrollbars?nsIScrollable::Scrollbar_Auto:nsIScrollable::Scrollbar_Never );
			scroller->SetCurScrollPosEx ( scrollX , scrollY );
		}
	}

	return TRUE;
}

extern "C" void register_menu_cb( GtkWidget *pGtkMozEmbed )
{
	if( pGtkMozEmbed )
	{
		// initialize HTTPS/SSL, if not yet done.
		nsCOMPtr<nsIServiceManager> spiServManager;
		if( !NS_FAILED(NS_GetServiceManager(getter_AddRefs(spiServManager))) )
		{
			g_print( "retrieved service manager of XULRunner ! Getting PSM..." );
			nsCOMPtr<nsISupports> psm;
			spiServManager->GetServiceByContractID("@mozilla.org/psm;1",
																						 NS_GET_IID(nsISupports),
																						 getter_AddRefs(psm));
			if(psm)
			{
				g_print( "PSM contract retrieved !\n" );
			}
			else
			{
				g_print( "PSM contract failed.\n" );
			}
		}	    
		
		gtk_signal_connect(GTK_OBJECT(pGtkMozEmbed), "dom_mouse_click",
											 GTK_SIGNAL_FUNC(dom_mouse_click_cb), NULL);
	}
}

extern "C" gint dom_mouse_click_cb(GtkMozEmbed *embed, nsIDOMMouseEvent *event, GtkWidget *pGtkMozEmbed)
{
	PRUint16 button = 0;
	if( event )
	{
		event->GetButton(&button);
		if( button == 2 ) // right click
		{
			PRInt32 clientX = 0;
			PRInt32 clientY = 0;
			event->GetClientX(&clientX);
			event->GetClientY(&clientY);

			GdkEvent* pEvent = gdk_event_new(GDK_BUTTON_PRESS);
			GdkEventButton* pButtonEvent = (GdkEventButton*)pEvent;
			if( pButtonEvent )
			{
				pButtonEvent->window = NULL;
				pButtonEvent->type = GDK_BUTTON_PRESS;
				pButtonEvent->button = 3;
				pButtonEvent->x = clientX;
				pButtonEvent->y = clientY;

				send_mouse_click_to_cd( pButtonEvent );
				gdk_event_free( pEvent ); pEvent = NULL; pButtonEvent = NULL;
			}
		}
	}
	return NS_OK;
}

