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
#include <string.h>
#ifdef HAVE_X11
#ifdef HAVE_XRANDR
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <gdk/gdkx.h>
#endif
#endif

#include "applet-struct.h"
#include "applet-notifications.h"


static gboolean _cd_allow_minimize (CairoDesklet *pDesklet, gpointer data)
{
	pDesklet->bAllowMinimize = TRUE;
	return FALSE;
}

static void _cd_show_hide_desktop (gboolean bShowDesklets)
{
	if (! myData.bDesktopVisible && ! bShowDesklets)  // on autorise chaque desklet a etre minimise. l'autorisation est annulee lors de leur cachage, donc on n'a pas besoin de faire le contraire apres avoir montre le bureau.
	{
		gldi_desklets_foreach ((GldiDeskletForeachFunc) _cd_allow_minimize, NULL);
	}
	
	gldi_desktop_show_hide (! myData.bDesktopVisible);
}

static void _cd_show_hide_desklet (void)
{
	if (!myData.bDeskletsVisible)
	{
		myData.pLastActiveWindow = gldi_windows_get_active ();
		gldi_object_ref (GLDI_OBJECT(myData.pLastActiveWindow));
		gldi_desklets_set_visible (TRUE);  // TRUE <=> les desklets de la couche widget aussi.
	}
	else
	{
		gldi_desklets_set_visibility_to_default ();
		if (myData.pLastActiveWindow)
		{
			gldi_window_show (myData.pLastActiveWindow);
			gldi_object_unref (GLDI_OBJECT(myData.pLastActiveWindow));
			myData.pLastActiveWindow = NULL;
		}
	}
	myData.bDeskletsVisible = ! myData.bDeskletsVisible;
	
	if (myConfig.cVisibleImage)
	{
		if (myData.bDesktopVisible || myData.bDeskletsVisible)
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cVisibleImage);
		else
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cHiddenImage);
	}
}


static void _cd_show_widget_layer (void)
{
	gldi_desktop_show_widget_layer ();
}

static void _cd_expose (void)
{
	gldi_desktop_present_desktops ();
}
static gboolean _expose_delayed (G_GNUC_UNUSED gpointer data)
{
	_cd_expose ();
	return FALSE;
}
static void _cd_action (CDActionOnClick iAction)
{
	switch (iAction)
	{
		case CD_SHOW_DESKTOP :
			_cd_show_hide_desktop (FALSE);
		break ;
		case CD_SHOW_DESKLETS :
			_cd_show_hide_desklet ();
		break ;
		case CD_SHOW_DESKTOP_AND_DESKLETS :
			_cd_show_hide_desktop (TRUE);  // TRUE <=> show the desklets
		break ;
		case CD_SHOW_WIDGET_LAYER :
			if (gldi_desktop_can_show_widget_layer ())
				_cd_show_widget_layer ();
			else
			{
				cd_warning ("It seems there is no widget layer, we show/hide the desktop");
				_cd_show_hide_desktop (FALSE);
			}
		break ;
		case CD_EXPOSE :
			if (gldi_desktop_can_present_desktops ())
			{
				g_timeout_add (250, _expose_delayed, NULL);  // Gnome-Shell bug here: it doesn't execute the Dbus command right after a click; I suspect the delay to be the interval of a double-click... anyway, it's stupid but it's barely noticeable, so it doesn't hurt that much even when not using GS.
			}
			else
			{
				cd_warning ("It seems we can't present desktops, we show/hide the desktop");
				_cd_show_hide_desktop (FALSE);
			}
		break ;
		default:
		break;
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	_cd_action (myConfig.iActionOnLeftClick);
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
#ifdef HAVE_X11
#ifdef HAVE_XRANDR
static void _on_select_resolution (GtkMenuItem *menu_item, gpointer data)  /// TODO: put that in the core...
{
	CD_APPLET_ENTER;
	int 				    iNumRes = GPOINTER_TO_INT (data);
	Display                 *dpy;
	Window                  root;
	XRRScreenConfiguration  *conf;
	short                   *rates;
	int                     num_rates;
	dpy = gdk_x11_get_default_xdisplay ();
	root = RootWindow(dpy, 0);
	
	conf = XRRGetScreenInfo(dpy, root);
	CD_APPLET_LEAVE_IF_FAIL (conf != NULL);
	//g_return_if_fail (conf != NULL);
	
	rates = XRRRates(dpy, 0, iNumRes, &num_rates);
	CD_APPLET_LEAVE_IF_FAIL (num_rates > 0);
	//g_return_if_fail (num_rates > 0);
	cd_debug ("available rates : from %d to %d Hz", rates[0], rates[num_rates-1]);
	
	XRRSetScreenConfigAndRate(dpy, conf, root, iNumRes, RR_Rotate_0, rates[num_rates-1], CurrentTime);
	XRRFreeScreenConfigInfo (conf);
	
	// restore original conf :  XRRSetScreenConfigAndRate(dpy, conf, root, original_size_id, original_rotation, original_rate, CurrentTime);
	CD_APPLET_LEAVE();
}
#endif
#endif
static void _show_desktop (GtkMenuItem *menu_item, gpointer data)
{
	_cd_show_hide_desktop (FALSE);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	gchar *cLabel;
	
	if (myConfig.iActionOnLeftClick != CD_SHOW_DESKTOP)  // action is not bound to left-click => put it in the menu
	{
		if (myConfig.iActionOnMiddleClick == CD_SHOW_DESKTOP)
			cLabel = g_strdup_printf ("%s (%s)", D_("Show desktop"), D_("middle-click"));
		else
			cLabel = g_strdup (D_("Show desktop"));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
			MY_APPLET_SHARE_DATA_DIR"/../shared-files/images/show-desktop.svg",
			_show_desktop,
			CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
	if (myConfig.iActionOnLeftClick != CD_EXPOSE && gldi_desktop_can_present_desktops ())  // action is not bound to left-click => put it in the menu
	{
		if (myConfig.iActionOnMiddleClick == CD_EXPOSE)
			cLabel = g_strdup_printf ("%s (%s)", D_("Expose all the desktops"), D_("middle-click"));
		else
			cLabel = g_strdup (D_("Expose all the desktops"));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
			MY_APPLET_SHARE_DATA_DIR"/../shared-files/images/expose-desktops.svg",
			_cd_expose,
			CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
	if (myConfig.iActionOnLeftClick != CD_SHOW_WIDGET_LAYER && gldi_desktop_can_show_widget_layer ())  // action is not bound to left-click => put it in the menu
	{
		if (myConfig.iActionOnMiddleClick == CD_SHOW_WIDGET_LAYER)
			cLabel = g_strdup_printf ("%s (%s)", D_("Show the Widget Layer"), D_("middle-click"));
		else
			cLabel = g_strdup (D_("Show the Widget Layer"));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel,
			MY_APPLET_SHARE_DATA_DIR"/../shared-files/images/widget-layer.svg",
		_cd_show_widget_layer,
		CD_APPLET_MY_MENU);
		g_free (cLabel);
	}  // on ne met pas les actions sur les desklets, surement assez peu utilisees.
	
	// Main Menu
	#ifdef HAVE_X11
	#ifdef HAVE_XRANDR
	if (cairo_dock_check_xrandr (1, 1))
	{
		GtkWidget *pResSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Change screen resolution"), CD_APPLET_MY_MENU, GTK_STOCK_FULLSCREEN);
		
		Display                 *dpy;
		Window                  root;
		XRRScreenConfiguration  *conf;
		Rotation                original_rotation;
		SizeID                  original_size_id;
		
		dpy = gdk_x11_get_default_xdisplay ();
		root = RootWindow(dpy, 0);
		
		conf = XRRGetScreenInfo(dpy, root);  // config  courante.
		if (conf != NULL)
		{
			//original_rate = XRRConfigCurrentRate(conf);
			original_size_id = XRRConfigCurrentConfiguration(conf, &original_rotation);
			
			// resolutions possibles.
			int num_sizes = 0;
			XRRScreenSize *xrrs = XRRSizes(dpy, 0, &num_sizes);
			// add to the menu
			GString *pResString = g_string_new ("");
			int i;
			for (i = 0; i < num_sizes; i ++)
			{
				g_string_printf (pResString, "%dx%d", xrrs[i].width, xrrs[i].height);
				if (i == original_size_id)
					CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (pResString->str, GTK_STOCK_APPLY, _on_select_resolution, pResSubMenu, GINT_TO_POINTER (i));
				else
					CD_APPLET_ADD_IN_MENU_WITH_DATA (pResString->str, _on_select_resolution, pResSubMenu, GINT_TO_POINTER (i));
				/*short   *rates;
				int     num_rates;
				rates = XRRRates(dpy, 0, i, &num_rates);
				for(int j = 0; j < num_rates; j ++) {
					possible_frequencies[i][j] = rates[j];
					printf("%4i ", rates[j]); }*/
			}
			g_string_free (pResString, TRUE);
			XRRFreeScreenConfigInfo (conf);
		}
	}
	else
		cd_warning ("Xrandr extension not available.");
	#endif
	#endif
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_cd_action (myConfig.iActionOnMiddleClick);
CD_APPLET_ON_MIDDLE_CLICK_END


static gchar *_get_desktop_path (void)
{
	gchar *cDesktopDir = cairo_dock_launch_command_sync ("xdg-user-dir DESKTOP");
	if (cDesktopDir == NULL)
		cDesktopDir = g_strdup_printf ("%s/Desktop", g_getenv ("HOME"));
	return cDesktopDir;
}
static void _move_to_desktop (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	gchar *cDesktopDir = _get_desktop_path ();
	if (cDesktopDir != NULL)
	{
		cairo_dock_launch_command_printf ("mv \"%s\" \"%s\"", NULL, myData.cPendingFile, cDesktopDir);
		g_free (cDesktopDir);
	}
}
static void _copy_to_desktop (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	gchar *cDesktopDir = _get_desktop_path ();
	if (cDesktopDir != NULL)
	{
		cairo_dock_launch_command_printf ("cp -r \"%s\" \"%s\"", NULL, myData.cPendingFile, cDesktopDir);
		g_free (cDesktopDir);
	}
}
static void _link_to_desktop (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	gchar *cDesktopDir = _get_desktop_path ();
	if (cDesktopDir != NULL)
	{
		cairo_dock_launch_command_printf ("ln -s \"%s\" \"%s\"", NULL, myData.cPendingFile, cDesktopDir);
		g_free (cDesktopDir);
	}
}
static void _make_link_to_desktop (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	gchar *cDesktopDir = _get_desktop_path ();
	if (cDesktopDir != NULL)
	{
		gchar *cName = g_path_get_basename (myData.cPendingFile);
		gchar *cContent = g_strdup_printf ("[Desktop Entry]\n"
			"Encoding=UTF-8\n"
			"Name=%s\n"
			"URL=%s\n"
			"Icon=file\n"
			"Type=Link\n",
			cName, myData.cPendingFile);
		
		gchar *cLinkPath = g_strdup_printf ("%s/Link to %s", cDesktopDir, cName);
		g_file_set_contents (cLinkPath,
			cContent,
			-1,
			NULL);
		
		g_free (cLinkPath);
		g_free (cContent);
		g_free (cName);
		g_free (cDesktopDir);
	}
}
static void _dl_finished (gpointer data)
{
	cd_debug ("DL IS FINISHED");
}
static void _download_to_desktop (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	gchar *cDesktopDir = _get_desktop_path ();
	if (cDesktopDir != NULL)
	{
		cairo_dock_download_file_async (myData.cPendingFile, NULL, (GFunc)_dl_finished, myApplet);
		g_free (cDesktopDir);
	}
}

CD_APPLET_ON_DROP_DATA_BEGIN
	GtkWidget *pMenu = gldi_menu_new (myIcon);
	g_free (myData.cPendingFile);
	myData.cPendingFile = g_strdup (CD_APPLET_RECEIVED_DATA);
	
	if (*CD_APPLET_RECEIVED_DATA == '/' || strncmp (CD_APPLET_RECEIVED_DATA, "file://", 7))  // fichier local
	{
		cairo_dock_add_in_menu_with_stock_and_data (("Move to the Desktop"), GTK_STOCK_CUT, G_CALLBACK (_move_to_desktop), pMenu, myApplet);
		cairo_dock_add_in_menu_with_stock_and_data (("Copy to the Desktop"), GTK_STOCK_COPY, G_CALLBACK (_copy_to_desktop), pMenu, myApplet);
		cairo_dock_add_in_menu_with_stock_and_data (("Link to the Desktop"), GTK_STOCK_JUMP_TO, G_CALLBACK (_link_to_desktop), pMenu, myApplet);
	}
	else  // fichier a telecharger.
	{
		cairo_dock_add_in_menu_with_stock_and_data (("Link to the Desktop"), GTK_STOCK_JUMP_TO, G_CALLBACK (_make_link_to_desktop), pMenu, myApplet);
		cairo_dock_add_in_menu_with_stock_and_data (("Download onto the Desktop"), GTK_STOCK_COPY, G_CALLBACK (_download_to_desktop), pMenu, myApplet);
	}
	CD_APPLET_POPUP_MENU_ON_MY_ICON (pMenu);
	
	
CD_APPLET_ON_DROP_DATA_END


void on_keybinding_pull (const char *keystring, gpointer user_data)
{
	CD_APPLET_ENTER;
	_cd_action (myConfig.iActionOnMiddleClick);
	CD_APPLET_LEAVE();
}


gboolean on_show_desktop (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	myData.bDesktopVisible = gldi_desktop_is_visible ();
	cd_debug ("bDesktopVisible <- %d", myData.bDesktopVisible);
	
	if (myConfig.cVisibleImage)
	{
		if (myData.bDesktopVisible || myData.bDeskletsVisible)
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cVisibleImage);
		else
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cHiddenImage);
		CD_APPLET_REDRAW_MY_ICON;
	}
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
}
