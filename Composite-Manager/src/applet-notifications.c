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

#define _BSD_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-composite-manager.h"

static void _on_answer (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // ok or Enter
	{
		cd_toggle_composite ();
	}
}

CD_APPLET_ON_CLICK_BEGIN
	if (myConfig.bAskBeforeSwitching)
	{
		cairo_dock_show_dialog_full (D_("Toggle composite?"), myIcon, myContainer, 0, "same icon", NULL, (CairoDockActionOnAnswerFunc)_on_answer, NULL, NULL);
	}
	else
	{
		cd_toggle_composite ();
	}
CD_APPLET_ON_CLICK_END

		
static void _cd_show_desktop (void)
{
	gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
	cairo_dock_show_hide_desktop (! bDesktopIsVisible);
}
static void _cd_expose_windows (void)
{
	cairo_dock_wm_present_windows ();
}
static gboolean _cd_expose_windows_idle (gpointer data)
{
	_cd_expose_windows ();
	return FALSE;
}
static void _cd_expose_desktops (void)
{
	cairo_dock_wm_present_desktops ();
}
static void _cd_show_widget_layer (void)
{
	cairo_dock_wm_show_widget_layer ();
}

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	switch (myConfig.iActionOnMiddleClick)
	{
		
		case CD_EDIT_CONFIG:
		{
			 cd_open_wm_config();
		}
		break;
		case CD_SHOW_DESKTOP:
		{
			_cd_show_desktop ();
		}
		break;
		case CD_EXPOSE_DESKTOPS:
		{
			_cd_expose_desktops ();
		}
		break;
		case CD_EXPOSE_WINDOWS:
		{
			// ok this is just crazy: if you call the Scale dbus method of Compiz before the middle button is released, it doesn't work.
			g_timeout_add (300, _cd_expose_windows_idle, NULL);
		}
		break;
		case CD_SHOW_WIDGET_LAYER:
		{
			_cd_show_widget_layer ();
		}
		break;
		default:
		break;
	}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	gchar *cLabel;
	
	cLabel = (myConfig.iActionOnMiddleClick == CD_EDIT_CONFIG ? g_strdup_printf ("%s (%s)", D_("Edit Window-Manager settings"), D_("middle-click")) : g_strdup (D_("Edit Window-Manager settings")));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
		GTK_STOCK_EDIT,
		cd_open_wm_config,
		CD_APPLET_MY_MENU);
	g_free (cLabel);
	
	cLabel = (myConfig.iActionOnMiddleClick == CD_SHOW_DESKTOP ? g_strdup_printf ("%s (%s)", D_("Show desktop"), D_("middle-click")) : g_strdup (D_("Show desktop")));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
		MY_APPLET_SHARE_DATA_DIR"/../shared-images/show-desktop.svg",
		_cd_show_desktop,
		CD_APPLET_MY_MENU);
	g_free (cLabel);
	
	if (cairo_dock_wm_can_present_desktops ())
	{
		cLabel = (myConfig.iActionOnMiddleClick == CD_EXPOSE_DESKTOPS ? g_strdup_printf ("%s (%s)", D_("Expose all the desktops"), D_("middle-click")) : g_strdup (D_("Expose all the desktops")));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
			MY_APPLET_SHARE_DATA_DIR"/../shared-images/expose-desktops.svg",
			_cd_expose_desktops,
			CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
	if (cairo_dock_wm_can_present_windows ())
	{
		cLabel = (myConfig.iActionOnMiddleClick == CD_EXPOSE_WINDOWS ? g_strdup_printf ("%s (%s)", D_("Expose all the windows"), D_("middle-click")) : g_strdup (D_("Expose all the windows")));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
			MY_APPLET_SHARE_DATA_DIR"/../shared-images/expose-windows.svg",
			_cd_expose_windows,
			CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
	if (cairo_dock_wm_can_show_widget_layer ())
	{
		cLabel = (myConfig.iActionOnMiddleClick == CD_SHOW_WIDGET_LAYER ? g_strdup_printf ("%s (%s)", D_("Show the Widget Layer"), D_("middle-click")) : g_strdup (D_("Show the Widget Layer")));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
			GTK_STOCK_LEAVE_FULLSCREEN,
			_cd_show_widget_layer,
			CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
CD_APPLET_ON_BUILD_MENU_END
