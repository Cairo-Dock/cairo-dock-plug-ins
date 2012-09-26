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
#include "applet-draw.h"
#include "applet-app.h"

#define CD_APP_MENU_REGISTRAR_ADDR "com.canonical.AppMenu.Registrar"
#define CD_APP_MENU_REGISTRAR_OBJ "/com/canonical/AppMenu/Registrar"
#define CD_APP_MENU_REGISTRAR_IFACE "com.canonical.AppMenu.Registrar"

static DBusGProxyCall *s_pDetectRegistrarCall = NULL;
static DBusGProxyCall *s_pGetMenuCall = NULL;


  ///////////////////////
 /// WINDOW CONTROLS ///
///////////////////////

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
	if (Xid == 0)
	{
		myData.bCanMinimize = FALSE;
		myData.bCanMaximize = FALSE;
		myData.bCanClose = FALSE;
		return;
	}
	
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

static void _set_border (Icon *icon, CairoContainer *pContainer, gboolean bWithBorder)
{
	if (icon->bIsMaximized)
		cd_app_menu_set_window_border (icon->Xid, bWithBorder);
}
void cd_app_menu_set_windows_borders (gboolean bWithBorder)
{
	cairo_dock_foreach_applis ((CairoDockForeachIconFunc)_set_border, FALSE, GINT_TO_POINTER (bWithBorder));
}


  ////////////////////////
 /// APPLICATION MENU ///
////////////////////////

static void cd_app_menu_launch_our_registrar (void)
{
	cairo_dock_launch_command (CD_PLUGINS_DIR"/appmenu-registrar");
	myData.bOwnRegistrar = TRUE;
}

static void _on_registrar_owner_changed (const gchar *cName, gboolean bOwned, gpointer data)
{
	cd_debug ("Registrar is on the bus (%d)", bOwned);
	CD_APPLET_ENTER;
	
	if (bOwned)
	{
		// set up a proxy to the Registrar
		myData.pProxyRegistrar = cairo_dock_create_new_session_proxy (
			CD_APP_MENU_REGISTRAR_ADDR,
			CD_APP_MENU_REGISTRAR_OBJ,
			CD_APP_MENU_REGISTRAR_IFACE);  // whenever it appears on the bus, we'll get it.
		
		// get the controls and menu of the current window.
		Window iActiveWindow = cairo_dock_get_current_active_window ();
		
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
	cd_debug ("Registrar is present: %d", bPresent);
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
	if (s_pDetectRegistrarCall == NULL)
		s_pDetectRegistrarCall = cairo_dock_dbus_detect_application_async (CD_APP_MENU_REGISTRAR_ADDR,
			(CairoDockOnAppliPresentOnDbus) _on_detect_registrar,
			NULL);
}


void cd_app_disconnect_from_registrar (void)
{
	// stop detecting/watching the registrar
	cairo_dock_stop_watching_dbus_name_owner (CD_APP_MENU_REGISTRAR_ADDR,
		(CairoDockDbusNameOwnerChangedFunc) _on_registrar_owner_changed);
	
	if (s_pDetectRegistrarCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, s_pDetectRegistrarCall);
		s_pDetectRegistrarCall = NULL;
	}
	
	// discard the menu
	if (s_pGetMenuCall != NULL)
	{
		DBusGProxy *pProxy = cairo_dock_get_main_proxy ();
		dbus_g_proxy_cancel_call (pProxy, s_pGetMenuCall);
		s_pGetMenuCall = NULL;
	}
	
	if (myData.pMenu != NULL)
	{
		gtk_widget_destroy (GTK_WIDGET (myData.pMenu));
		myData.pMenu = NULL;
	}
	
	if (myData.pTask != NULL)
	{
		cairo_dock_discard_task (myData.pTask);
		myData.pTask = NULL;
	}
	
	// kill the registrar if it's our own one
	if (myData.bOwnRegistrar)
	{
		int r = system ("pkill appmenu-registr");  // 15 chars limit; 'pkill -f' doesn't work :-/ this is not very clean, we should get the PID when we spawn it, and use it.
		if (r < 0)
			cd_warning ("Not able to launch this command: pkill");
		myData.bOwnRegistrar = FALSE;
	}
}


typedef struct {
	gchar *cService;
	gchar *cMenuObject;
	DbusmenuGtkMenu *pMenu;
} CDSharedMemory;

static void _on_menu_destroyed (CairoDockModuleInstance *myApplet, GObject *old_menu_pointer)
{
	if (old_menu_pointer == (GObject*)myData.pMenu)
		myData.pMenu = NULL;
}

/*static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	g_free (pSharedMemory->cService);
	g_free (pSharedMemory->cMenuObject);
	if (pSharedMemory->pMenu)
		gtk_widget_destroy (GTK_WIDGET (pSharedMemory->pMenu));
	g_free (pSharedMemory);
}
static void _get_menu_async (CDSharedMemory *pSharedMemory)
{
	cd_debug ("%s()", __func__);
	pSharedMemory->pMenu = dbusmenu_gtkmenu_new (pSharedMemory->cService, pSharedMemory->cMenuObject);  /// can this object disappear by itself ? it seems to crash with 2 instances of inkscape, when closing one of them... 
	cd_debug ("menu built");
}
static gboolean _fetch_menu (CDSharedMemory *pSharedMemory)
{
	cd_debug ("%s()", __func__);
	CD_APPLET_ENTER;
	myData.pMenu = pSharedMemory->pMenu;
	pSharedMemory->pMenu = NULL;
	
	g_object_weak_ref (G_OBJECT (myData.pMenu),
		(GWeakNotify)_on_menu_destroyed,
		myApplet);
	CD_APPLET_LEAVE (TRUE);
}*/

static void _on_got_menu (DBusGProxy *proxy, DBusGProxyCall *call_id, CairoDockModuleInstance *myApplet)
{
	cd_debug ("%s ()", __func__);
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
		cd_debug (" -> %s", cService);
		cd_debug ("    %s", cMenuObject);
		if (cService && *cService != '\0')
		{
			/*if (myData.pTask != NULL)
			{
				cairo_dock_discard_task (myData.pTask);
				myData.pTask = NULL;
			}
			CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
			pSharedMemory->cService = cService;
			pSharedMemory->cMenuObject = cMenuObject;
			myData.pTask = cairo_dock_new_task_full (0,
				(CairoDockGetDataAsyncFunc) _get_menu_async,
				(CairoDockUpdateSyncFunc) _fetch_menu,
				(GFreeFunc) _free_shared_memory,
				pSharedMemory);
			cairo_dock_launch_task_delayed (myData.pTask, 0);*/
			/// TODO: it seems to hang he dock for a second, even with a task :-/
			/// so maybe we need to cache the {window,menu} couples...
			myData.pMenu = dbusmenu_gtkmenu_new (cService, cMenuObject);  /// can this object disappear by itself ? it seems to crash with 2 instances of inkscape, when closing one of them... 
			if (myData.pMenu)
				g_object_weak_ref (G_OBJECT (myData.pMenu),
					(GWeakNotify)_on_menu_destroyed,
					myApplet);
		}
	}
	
	///g_free (cService);
	///g_free (cMenuObject);
	CD_APPLET_LEAVE ();
}
static void _get_application_menu (Window Xid)
{
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
	
	if (myData.pTask != NULL)
	{
		cairo_dock_discard_task (myData.pTask);
		myData.pTask = NULL;
	}
	
	// get the new one.
	if (Xid != 0)
	{
		if (myData.pProxyRegistrar != NULL)
		{
			s_pGetMenuCall = dbus_g_proxy_begin_call (myData.pProxyRegistrar,
				"GetMenuForWindow",
				(DBusGProxyCallNotify)_on_got_menu,
				myApplet,
				(GDestroyNotify) NULL,
				G_TYPE_UINT, Xid,
				G_TYPE_INVALID);
		}
	}
}


  ////////////////////
 /// START / STOP ///
////////////////////

static gboolean _get_current_window_idle (CairoDockModuleInstance *myApplet)
{
	// get the controls and menu of the current window.
	Window iActiveWindow = cairo_dock_get_current_active_window ();
	
	cd_app_menu_set_current_window (iActiveWindow);
	
	myData.iSidInitIdle = 0;
	return FALSE;
}
static gboolean _remove_windows_borders (CairoDockModuleInstance *myApplet)
{
	cd_app_menu_set_windows_borders (FALSE);
	
	myData.iSidInitIdle2 = 0;
	return FALSE;
}
void cd_app_menu_start (void)
{
	// connect to the registrar or directly get the current window.
	if (myConfig.bDisplayMenu)
	{
		cd_app_detect_registrar ();  // -> will get the current window once connected to the registrar
	}
	else
	{
		myData.iSidInitIdle = g_idle_add ((GSourceFunc)_get_current_window_idle, myApplet);  // in idle, because it's heavy + the applications-manager is started after the plug-ins.
	}
	
	// remove borders from all maximised windows
	if (myConfig.bDisplayControls)
	{
		myData.iSidInitIdle2 = g_idle_add ((GSourceFunc)_remove_windows_borders, myApplet);  // in idle, because it's heavy + the applications-manager is started after the plug-ins.
	}
	
	if (myConfig.bDisplayControls)
	{
		cd_app_menu_resize ();
	}
}


void cd_app_menu_stop (void)
{
	// disconnect from the registrar.
	if (myConfig.bDisplayMenu)
	{
		cd_app_disconnect_from_registrar ();
	}
	
	// set back the window border of maximized windows.
	if (myConfig.bDisplayControls)
	{	
		cd_app_menu_set_windows_borders (TRUE);
	}
	
	if (myData.iSidInitIdle != 0)
		g_source_remove (myData.iSidInitIdle);
	if (myData.iSidInitIdle2 != 0)
		g_source_remove (myData.iSidInitIdle2);
}


void cd_app_menu_set_current_window (Window iActiveWindow)
{
	cd_debug ("%s (%ld)", __func__, iActiveWindow);
	if (iActiveWindow != myData.iCurrentWindow)
	{
		myData.iPreviousWindow = myData.iCurrentWindow;
		myData.iCurrentWindow = iActiveWindow;
		myIcon->Xid = iActiveWindow;  // set the Xid on our icon, so that the dock adds the usual actions in our right-click menu.
		
		if (myConfig.bDisplayMenu)
			_get_application_menu (iActiveWindow);
		
		if (myConfig.bDisplayControls)
			_get_window_allowed_actions (iActiveWindow);
		
		// update the icon
		Icon *icon = cairo_dock_get_icon_with_Xid (iActiveWindow);
		CD_APPLET_SET_NAME_FOR_MY_ICON (icon ? icon->cName : NULL);
		
		cd_app_menu_redraw_icon ();
	}
}
