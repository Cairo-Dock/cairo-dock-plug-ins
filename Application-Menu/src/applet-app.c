/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* based on indicator-messages.c written by :
*  Ted Gould <ted@canonical.com>
*  Cody Russell <cody.russell@canonical.com>
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "MwmUtil.h"

#include "applet-struct.h"
#include "applet-app.h"

#define CD_APP_MENU_REGISTRAR_ADDR "com.canonical.AppMenu.Registrar"
#define CD_APP_MENU_REGISTRAR_OBJ "/com/canonical/AppMenu/Registrar"
#define CD_APP_MENU_REGISTRAR_IFACE "com.canonical.AppMenu.Registrar"

static DBusGProxyCall *s_pDetectRegistrarCall = NULL;
static DBusGProxyCall *s_pGetMenuCall = NULL;


void cd_app_menu_set_window_border (Window Xid, gboolean bWithBorder)
{
	Display *dpy = cairo_dock_get_Xdisplay();
	MwmHints mwmhints;
	Atom prop;
	memset(&mwmhints, 0, sizeof(mwmhints));
	prop = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
	mwmhints.flags = MWM_HINTS_DECORATIONS;
	mwmhints.decorations = bWithBorder;
	XChangeProperty (dpy, Xid, prop,
		prop, 32, PropModeReplace,
		(unsigned char *) &mwmhints,
		PROP_MWM_HINTS_ELEMENTS);
}

static void _get_window_allowed_actions (Window Xid)
{
	g_return_if_fail (Xid > 0);
	Atom aReturnedType = 0;
	int aReturnedFormat = 0;
	unsigned long iLeftBytes, iBufferNbElements = 0;
	gulong *pXStateBuffer = NULL;
	Display *dpy = cairo_dock_get_Xdisplay ();
	Atom allowedActions = XInternAtom (dpy, "_NET_WM_ALLOWED_ACTIONS", False);
	Atom atomMinimize = XInternAtom (dpy, "_NET_WM_ACTION_MINIMIZE", False);
	Atom atomMaximizeHorz = XInternAtom (dpy, "_NET_WM_ACTION_MAXIMIZE_HORZ", False);
	Atom atomMaximizeVert = XInternAtom (dpy, "_NET_WM_ACTION_MAXIMIZE_VERT", False);
	Atom atomClose = XInternAtom (dpy, "_NET_WM_ACTION_CLOSE", False);
	
	XGetWindowProperty (dpy,
		Xid, allowedActions, 0, G_MAXULONG, False, XA_ATOM,
		&aReturnedType, &aReturnedFormat, &iBufferNbElements, &iLeftBytes, (guchar **)&pXStateBuffer);
	
	gboolean bIsInState = FALSE;
	if (iBufferNbElements > 0)
	{
		myData.bCanMinimize = FALSE;
		myData.bCanMaximize = FALSE;
		myData.bCanClose = FALSE;
		guint i;
		for (i = 0; i < iBufferNbElements; i ++)
		{
			if (pXStateBuffer[i] == atomMinimize)
			{
				myData.bCanMinimize = TRUE;
			}
			else if (pXStateBuffer[i] == atomMaximizeHorz || pXStateBuffer[i] == atomMaximizeVert)
			{
				myData.bCanMaximize = TRUE;
			}
			else if (pXStateBuffer[i] == atomClose)
			{
				myData.bCanClose = TRUE;
			}
		}
	}
	else  // by default, allow all actions.
	{
		cd_warning ("couldn't get allowed actions for the window %u", Xid);
		myData.bCanMinimize = TRUE;
		myData.bCanMaximize = TRUE;
		myData.bCanClose = TRUE;
	}

	XFree (pXStateBuffer);
}

static void cd_app_menu_launch_our_registrar (void)
{
	g_print ("%s\n", CD_PLUGINS_DIR"/appmenu-registrar");
	cairo_dock_launch_command (CD_PLUGINS_DIR"/appmenu-registrar");
	myData.bOwnRegistrar = TRUE;
}

static void _on_registrar_owner_changed (const gchar *cName, gboolean bOwned, gpointer data)
{
	g_print ("=== Registrar is on the bus (%d)\n", bOwned);
	CD_APPLET_ENTER;
	
	if (bOwned)
	{
		// set up a proxy to the Registrar
		myData.pProxyRegistrar = cairo_dock_create_new_session_proxy (
			CD_APP_MENU_REGISTRAR_ADDR,
			CD_APP_MENU_REGISTRAR_OBJ,
			CD_APP_MENU_REGISTRAR_IFACE);  // whenever it appears on the bus, we'll get it.
		
		// get the controls and menu of the current window.
		Window iActiveWindow = cairo_dock_get_active_xwindow ();
		
		cd_app_menu_set_current_window (iActiveWindow);
	}
	else  // no more registrar on the bus.
	{
		g_object_unref (myData.pProxyRegistrar);
		myData.pProxyRegistrar = NULL;
		
		cd_app_menu_launch_our_registrar ();
	}
	CD_APPLET_LEAVE ();
}

static void _on_detect_registrar (gboolean bPresent, gpointer data)
{
	g_print ("=== Registrar is present: %d\n", bPresent);
	CD_APPLET_ENTER;
	s_pDetectRegistrarCall = NULL;
	// if present, set up proxy.
	if (bPresent)
	{
		_on_registrar_owner_changed (CD_APP_MENU_REGISTRAR_ADDR, TRUE, NULL);
	}
	else
	{
		cd_app_menu_launch_our_registrar ();  // when it has been launched, we'll get notified by Dbus.
	}
	
	// watch whenever the Registrar goes up or down.
	cairo_dock_watch_dbus_name_owner (CD_APP_MENU_REGISTRAR_ADDR,
		(CairoDockDbusNameOwnerChangedFunc) _on_registrar_owner_changed,
		NULL);
	CD_APPLET_LEAVE ();
}

void cd_app_detect_registrar (void)
{
	s_pDetectRegistrarCall = cairo_dock_dbus_detect_application_async (CD_APP_MENU_REGISTRAR_ADDR,
		(CairoDockOnAppliPresentOnDbus) _on_detect_registrar,
		NULL);
}


void cd_app_disconnect_from_registrar (void)
{
	cairo_dock_stop_watching_dbus_name_owner (CD_APP_MENU_REGISTRAR_ADDR,
		(CairoDockDbusNameOwnerChangedFunc) _on_registrar_owner_changed);
	
	if (s_pDetectRegistrarCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, s_pDetectRegistrarCall);
		s_pDetectRegistrarCall = NULL;
	}
	
	if (s_pGetMenuCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, s_pGetMenuCall);
		s_pGetMenuCall = NULL;
	}
	
	if (myData.iSidRenderIcon != 0)
		g_source_remove (myData.iSidRenderIcon);
	
	/// TODO: kill the registrar if it's our own one...
	if (myData.bOwnRegistrar)
	{
		
	}
	
	
	/// TODO: set back the window border of maximized windows...
	
}


static void _on_got_menu (DBusGProxy *proxy, DBusGProxyCall *call_id, CairoDockModuleInstance *myApplet)
{
	g_print ("=== %s ()\n", __func__);
	CD_APPLET_ENTER;
	s_pGetMenuCall = NULL;
	
	GError *erreur = NULL;
	gchar *cService = NULL, *cMenuObject = NULL;
	
	gboolean bSuccess = dbus_g_proxy_end_call (proxy,
		call_id,
		&erreur,
		G_TYPE_STRING, &cService,
		DBUS_TYPE_G_OBJECT_PATH, &cMenuObject,
		G_TYPE_INVALID);
	if (erreur)
	{
		cd_warning ("couldn't get the application menu (%s)", erreur->message);
		g_error_free (erreur);
	}
	if (bSuccess)
	{
		g_print (" -> %s\n", cService);
		g_print ("    %s\n", cMenuObject);
		myData.pMenu = dbusmenu_gtkmenu_new (cService, cMenuObject);
	}
	
	g_free (cService);
	CD_APPLET_LEAVE ();
}
static gboolean _render_icon  (gpointer data)
{
	cd_app_menu_render_icon  ();
	myData.iSidRenderIcon = 0;
	return FALSE;
}
void cd_app_menu_set_current_window (Window iActiveWindow)
{
	g_print ("%s (%ld)\n", __func__, iActiveWindow);
	if (iActiveWindow != myData.iCurrentWindow)
	{
		//cd_app_menu_set_window_border (myData.iCurrentWindow, TRUE);
		//cd_app_menu_set_window_border (iActiveWindow, FALSE);
		
		myData.iCurrentWindow = iActiveWindow;
		
		// destroy the current menu
		if (myData.pMenu != NULL)
		{
			gtk_widget_destroy (GTK_WIDGET (myData.pMenu));
			myData.pMenu = NULL;
		}
		
		if (s_pGetMenuCall != NULL)
		{
			DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
			dbus_g_proxy_cancel_call (pProxy, s_pGetMenuCall);
			s_pGetMenuCall = NULL;
		}
		
		// get the new one.
		if (myData.pProxyRegistrar != NULL)
		{
			s_pGetMenuCall = dbus_g_proxy_begin_call (myData.pProxyRegistrar,
				"GetMenuForWindow",
				(DBusGProxyCallNotify)_on_got_menu,
				myApplet,
				(GDestroyNotify) NULL,
				G_TYPE_UINT, iActiveWindow,
				G_TYPE_INVALID);
		}
		
		// get window controls.
		_get_window_allowed_actions (iActiveWindow);
		g_print (" %d/%d/%d, %d\n", myData.bCanMinimize, myData.bCanMaximize, myData.bCanClose, myData.iSidRenderIcon);
		
		// update the icon (do it in idle).
		/// TODO: use a transition ...
		if (myData.iSidRenderIcon == 0)
			myData.iSidRenderIcon = g_idle_add (_render_icon, NULL);
	}
}


void cd_app_menu_render_icon (void)
{
	Icon *pIcon = cairo_dock_get_icon_with_Xid (myData.iCurrentWindow);
	if (pIcon != NULL)
	{
		if (pIcon->iIconTexture != 0 && CD_APPLET_START_DRAWING_MY_ICON)
		{
			int w, h;
			CD_APPLET_GET_MY_ICON_EXTENT (&w, &h);
			
			_cairo_dock_enable_texture ();
			_cairo_dock_set_blend_source ();
			_cairo_dock_apply_texture_at_size (pIcon->iIconTexture, w, h);
			_cairo_dock_disable_texture ();
			
			/// draw the controls ...
			
			CD_APPLET_FINISH_DRAWING_MY_ICON;
		}
		else if (pIcon->pIconBuffer != NULL)
		{
			CD_APPLET_SET_SURFACE_ON_MY_ICON (pIcon->pIconBuffer);
			
			/// draw the controls ...
			
		}
		
		CD_APPLET_REDRAW_MY_ICON;
	}
	else
	{
		CD_APPLET_SET_IMAGE_ON_MY_ICON ("");  /// use the default icon of the core ...
	}
}
