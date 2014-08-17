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
		GldiWindowActor *actor = gldi_windows_get_active ();
		
		cd_app_menu_set_current_window (actor);
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
		gldi_task_discard (myData.pTask);
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

static void _on_got_menu (DBusGProxy *proxy, DBusGProxyCall *call_id, GldiModuleInstance *myApplet)
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
			myData.pMenu = dbusmenu_gtkmenu_new (cService, cMenuObject);  /// can this object disappear by itself ? it seems to crash with 2 instances of inkscape, when closing one of them... 
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
	
	///g_free (cService);
	///g_free (cMenuObject);
	CD_APPLET_LEAVE ();
}
static void _get_application_menu (GldiWindowActor *actor)
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
		gldi_task_discard (myData.pTask);
		myData.pTask = NULL;
	}
	
	// get the new one.
	if (actor != NULL)
	{
		if (myData.pProxyRegistrar != NULL)
		{
			guint id = gldi_window_get_id (actor);
			
			s_pGetMenuCall = dbus_g_proxy_begin_call (myData.pProxyRegistrar,
				"GetMenuForWindow",
				(DBusGProxyCallNotify)_on_got_menu,
				myApplet,
				(GDestroyNotify) NULL,
				G_TYPE_UINT, id,
				G_TYPE_INVALID);
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
	cd_debug ("%s (%p)", __func__, actor);
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
