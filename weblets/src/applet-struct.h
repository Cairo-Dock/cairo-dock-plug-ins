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


#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <gtk/gtk.h>
#include <cairo-dock.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <webkit2/webkit2.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	   gchar *cURI_to_load;
	   gboolean bShowScrollbars;
	   gboolean bIsTransparent;
	   gint iPosScrollX;
	   gint iPosScrollY;
	   guint iReloadTimeout;
	   gchar **cListURI;
	   gint iRightMargin;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
      CairoDialog *dialog;
	    GtkWidget *pGtkMozEmbed;
	    WebKitWebView *pWebKitView;
	    GldiTask *pRefreshTimer;
	} ;


#endif
