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


  ///////////////////////
 /// WINDOW CONTROLS ///
///////////////////////

static void _get_window_allowed_actions (GldiWindowActor *actor)
{
	if (actor == NULL)
	{
		myData.bCanMinimize = FALSE;
		myData.bCanMaximize = FALSE;
		myData.bCanClose = FALSE;
		return;
	}
	gldi_window_can_minimize_maximize_close (actor, &myData.bCanMinimize, &myData.bCanMaximize, &myData.bCanClose);
}

static void _set_border (GldiWindowActor *actor, gboolean bWithBorder)
{
	if (actor->bIsMaximized)
		gldi_window_set_border (actor, bWithBorder);
}
void cd_app_menu_set_windows_borders (gboolean bWithBorder)
{
	gldi_windows_foreach (FALSE, (GFunc)_set_border, GINT_TO_POINTER (bWithBorder));
}


  ////////////////////////
 /// APPLICATION MENU ///
////////////////////////

static void _child_watch (GPid pid, G_GNUC_UNUSED gint wait_status, gpointer ptr)
{
	CD_APPLET_ENTER;
	
	AppChildWatchData *pData = (AppChildWatchData*)ptr;
	if (pData->pApplet)
	{
		pData->pApplet->bOwnRegistrar = FALSE;
		pData->pApplet->pChildWatch = NULL;
	}
	g_free (pData);
	g_spawn_close_pid (pid);
	
	CD_APPLET_LEAVE ();
}

static void _kill_our_registrar (void)
{
	if (myData.bOwnRegistrar)
	{
		myData.pChildWatch->pApplet = NULL; // so that the child watch function will not mess with myData
		myData.pChildWatch = NULL;
		myData.bOwnRegistrar = FALSE;
		kill (myData.pidOwnRegistrar, SIGTERM);
	}
}

static void cd_app_menu_launch_our_registrar (void)
{
	_kill_our_registrar (); // In case it was running, but stopped working
	
	const gchar *args[] = {CD_PLUGINS_DIR"/appmenu-registrar", NULL};
	
	GError *err = NULL;
	gboolean ret = g_spawn_async (NULL, (gchar**)args, NULL,
		// note: posix_spawn (and clone on Linux) will be used if we include the following
		// two flags, but this requires to later add a child watch and also adds a risk of
		// fd leakage (although it has not been an issue before with system() as well)
		G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_LEAVE_DESCRIPTORS_OPEN |
		G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
		NULL, NULL,
		&myData.pidOwnRegistrar, &err);
	if (!ret)
	{
		cd_warning ("couldn't launch this command (%s : %s)", args[0], err->message);
		g_error_free (err);
	}
	else
	{
		myData.bOwnRegistrar = TRUE;
		myData.pChildWatch = g_new (AppChildWatchData, 1);
		myData.pChildWatch->pApplet = &myData;
		g_child_watch_add (myData.pidOwnRegistrar, _child_watch, myData.pChildWatch);
	}
}

static void _registrar_proxy_created (G_GNUC_UNUSED GObject *pObj, GAsyncResult* pRes, G_GNUC_UNUSED gpointer ptr)
{
	CD_APPLET_ENTER;
	
	GError *err = NULL;
	GDBusProxy *pProxy = g_dbus_proxy_new_finish (pRes, &err);
	if (err)
	{
		if (! g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			cd_warning ("Cannot create app registrar DBus proxy: %s", err->message);
		g_error_free (err);
	}
	else
	{
		// note: we can only set it here, since myData might not be valid
		// if the call was canceled
		myData.pProxyRegistrar = pProxy;
		
		// get the controls and menu of the current window.
		GldiWindowActor *actor = gldi_windows_get_active ();
		cd_app_menu_set_current_window (actor);
	}
	
	CD_APPLET_LEAVE ();
}


static void _on_registrar_appeared (GDBusConnection *pConn, G_GNUC_UNUSED const gchar *cName,
	G_GNUC_UNUSED const gchar* cOwner, G_GNUC_UNUSED gpointer ptr)
{
	cd_debug ("Registrar is on the bus");
	CD_APPLET_ENTER;
	
	if (!myData.pCancel) myData.pCancel = g_cancellable_new ();
	g_dbus_proxy_new (pConn,
		G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
		G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS |
		G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
		NULL, // interface info
		CD_APP_MENU_REGISTRAR_ADDR,
		CD_APP_MENU_REGISTRAR_OBJ,
		CD_APP_MENU_REGISTRAR_IFACE,
		myData.pCancel,
		_registrar_proxy_created,
		NULL);
	
	CD_APPLET_LEAVE ();
}


static void _on_registrar_vanished (G_GNUC_UNUSED GDBusConnection *pConn,
	G_GNUC_UNUSED const gchar *cName, G_GNUC_UNUSED gpointer ptr)
{
	cd_debug ("Registrar is not on the bus");
	CD_APPLET_ENTER;
	
	if (myData.pCancel)
	{
		g_cancellable_cancel (myData.pCancel);
		g_object_unref (G_OBJECT (myData.pCancel));
		myData.pCancel = NULL;
	}
	
	if (myData.pProxyRegistrar)
	{
		g_object_unref (myData.pProxyRegistrar);
		myData.pProxyRegistrar = NULL;
	}
	cd_app_menu_launch_our_registrar ();
	
	CD_APPLET_LEAVE ();
}

void cd_app_detect_registrar (void)
{
	if (!myData.uRegWatch)
		myData.uRegWatch = g_bus_watch_name (G_BUS_TYPE_SESSION,
			CD_APP_MENU_REGISTRAR_ADDR,
			G_BUS_NAME_WATCHER_FLAGS_NONE,
			_on_registrar_appeared,
			_on_registrar_vanished,
			NULL,
			NULL);
}


void cd_app_disconnect_from_registrar (void)
{
	if (myData.uRegWatch)
	{
		g_bus_unwatch_name (myData.uRegWatch);
		myData.uRegWatch = 0;
	}
	
	if (myData.pCancel)
	{
		g_cancellable_cancel (myData.pCancel);
		g_object_unref (G_OBJECT (myData.pCancel));
		myData.pCancel = NULL;
	}
	
	if (myData.pCancelMenu)
	{
		g_cancellable_cancel (myData.pCancelMenu);
		g_object_unref (G_OBJECT (myData.pCancelMenu));
		myData.pCancelMenu = NULL;
	}
	
	if (myData.pProxyRegistrar)
	{
		g_object_unref (G_OBJECT (myData.pProxyRegistrar));
		myData.pProxyRegistrar = NULL;
	}
	
	if (myData.pMenu != NULL)
	{
		gtk_widget_destroy (GTK_WIDGET (myData.pMenu));
		myData.pMenu = NULL;
	}
	
/*	if (myData.pTask != NULL)
	{
		gldi_task_discard (myData.pTask);
		myData.pTask = NULL;
	} */
	
	// kill the registrar if it's our own one
	_kill_our_registrar ();
}


typedef struct {
	gchar *cService;
	gchar *cMenuObject;
	DbusmenuGtkMenu *pMenu;
} CDSharedMemory;

static void _on_menu_destroyed (GldiModuleInstance *myApplet, GObject *old_menu_pointer)
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

static void _connect_to_menu (const gchar *cService, const gchar *cMenuObject);
static void _on_got_menu (GObject *pObj, GAsyncResult *pRes, gpointer ptr)
{
	cd_debug ("%s ()", __func__);
	CD_APPLET_ENTER;
	
	GError *erreur = NULL;
	const gchar *cService = NULL, *cMenuObject = NULL;
	GVariant *res = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObj), pRes, &erreur);
	if (erreur)
	{
		if ( !(g_error_matches (erreur, G_IO_ERROR, G_IO_ERROR_CANCELLED) ||
			g_error_matches (erreur, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS)))
		{
			// Note: invalid args error is returned by our registrar if the window is not known.
			// This happens for apps that do not support global menus, so no need to display a warning.
			cd_warning ("couldn't get the application menu (%s)", erreur->message);
		}
		g_error_free (erreur);
	}
	else
	{
		if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(so)")))
			g_variant_get (res, "(&s&o)", &cService, &cMenuObject);
		else if (g_variant_is_of_type (res, G_VARIANT_TYPE ("(ss)"))) // just to be safe
			g_variant_get (res, "(&s&s)", &cService, &cMenuObject);
		else cd_warning ("Unexpected result type: %s", g_variant_get_type_string (res));
		if (cService && cMenuObject) _connect_to_menu (cService, cMenuObject);
		g_variant_unref (res);
	}
	
	CD_APPLET_LEAVE ();
}
		
static void _connect_to_menu (const gchar *cService, const gchar *cMenuObject)
{
	cd_debug (" -> %s", cService);
	cd_debug ("    %s", cMenuObject);
	if (cService && *cService != '\0')
	{
		/*if (myData.pTask != NULL)
		{
			gldi_task_discard (myData.pTask);
			myData.pTask = NULL;
		}
		CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
		pSharedMemory->cService = cService;
		pSharedMemory->cMenuObject = cMenuObject;
		myData.pTask = gldi_task_new_full (0,
			(GldiGetDataAsyncFunc) _get_menu_async,
			(GldiUpdateSyncFunc) _fetch_menu,
			(GFreeFunc) _free_shared_memory,
			pSharedMemory);
		gldi_task_launch_delayed (myData.pTask, 0);*/
		/// TODO: it seems to hang he dock for a second, even with a task :-/
		/// so maybe we need to cache the {window,menu} couples...
		// note: parameters are not const, but not modified (a copy is made)
		myData.pMenu = dbusmenu_gtkmenu_new ((gchar*)cService, (gchar*)cMenuObject);  /// can this object disappear by itself ? it seems to crash with 2 instances of inkscape, when closing one of them...
		if (g_object_is_floating (myData.pMenu))  // claim ownership on the menu.
			g_object_ref_sink (myData.pMenu);
		if (myData.pMenu)
		{
			g_object_weak_ref (G_OBJECT (myData.pMenu),
				(GWeakNotify)_on_menu_destroyed,
				myApplet);
			gldi_menu_init (GTK_WIDGET(myData.pMenu), myIcon);
		}
	}
}
	
static void _get_application_menu (GldiWindowActor *actor)
{
	// destroy the current menu
	if (myData.pMenu != NULL)
	{
		gtk_widget_destroy (GTK_WIDGET (myData.pMenu));
		myData.pMenu = NULL;
	}

	if (myData.pCancelMenu != NULL)
	{
		g_cancellable_cancel (myData.pCancelMenu);
		g_object_unref (G_OBJECT (myData.pCancelMenu));
		myData.pCancelMenu = NULL;
	}
	
/*	if (myData.pTask != NULL)
	{
		gldi_task_discard (myData.pTask);
		myData.pTask = NULL;
	} */
	
	// get the new one.
	if (actor != NULL)
	{
		char *cService = NULL;
		char *cMenuObject = NULL;
		gldi_window_get_menu_address (actor, &cService, &cMenuObject);
		
		if (cService && cMenuObject)
			_connect_to_menu (cService, cMenuObject);
		else if (myData.pProxyRegistrar != NULL)
		{
			if (!myData.pCancelMenu) myData.pCancelMenu = g_cancellable_new ();
			
			guint id = gldi_window_get_id (actor);
			
			g_dbus_proxy_call (myData.pProxyRegistrar,
				"GetMenuForWindow",
				g_variant_new ("(u)", id),
				G_DBUS_CALL_FLAGS_NO_AUTO_START,
				-1,
				myData.pCancelMenu,
				_on_got_menu,
				myApplet);
		}
	}
}


  ////////////////////
 /// START / STOP ///
////////////////////

static gboolean _get_current_window_idle (GldiModuleInstance *myApplet)
{
	// get the controls and menu of the current window.
	GldiWindowActor *actor = gldi_windows_get_active ();
	
	cd_app_menu_set_current_window (actor);
	
	myData.iSidInitIdle = 0;
	return FALSE;
}
static gboolean _remove_windows_borders_idle (GldiModuleInstance *myApplet)
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
		myData.iSidInitIdle2 = g_idle_add ((GSourceFunc)_remove_windows_borders_idle, myApplet);  // in idle, because it's heavy + the applications-manager is started after the plug-ins.
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
	gldi_icon_unset_appli (myIcon);
}


void cd_app_menu_set_current_window (GldiWindowActor *actor)
{
	cd_debug ("%s (%p): %s", __func__, actor, actor ? actor->cName : "");
	if (!actor && gldi_container_is_wayland_backend()) return; // KWin sets the active window to none when clicking on the dock
	if (actor != myData.pCurrentWindow)
	{
		myData.pPreviousWindow = myData.pCurrentWindow;
		myData.pCurrentWindow = actor;
		gldi_icon_set_appli (myIcon, actor);  // set the actor on our icon, so that the dock adds the usual actions in our right-click menu. // this takes a reference on the actor, and remove the ref on the previous one.
		
		if (myConfig.bDisplayMenu)
			_get_application_menu (actor);
		
		if (myConfig.bDisplayControls)
			_get_window_allowed_actions (actor);
		
		// update the icon
		CD_APPLET_SET_NAME_FOR_MY_ICON (actor ? actor->cName : NULL);
		
		cd_app_menu_redraw_icon ();
	}
}
