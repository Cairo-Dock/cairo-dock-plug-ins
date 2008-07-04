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
#include <docshell/nsIScrollable.h>

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
