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

#include <unistd.h>  // getpid
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "interface-applet-signals.h"
#include "interface-applet-methods.h"
#include "dbus-applet-spec.h"
#include "dbus-sub-applet-spec.h"
#include "interface-applet-object.h"

static int s_iModuleId = 1;
static GList *s_pAppletList = NULL;

static void cd_dbus_applet_dispose (GObject *object);
static void cd_dbus_applet_finalize (GObject *object);

G_DEFINE_TYPE(dbusApplet, cd_dbus_applet, G_TYPE_OBJECT);

G_DEFINE_TYPE(dbusSubApplet, cd_dbus_sub_applet, G_TYPE_OBJECT);

static void cd_dbus_applet_class_init(dbusAppletClass *klass)
{
	cd_debug("");
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->dispose = cd_dbus_applet_dispose;
	object_class->finalize = cd_dbus_applet_finalize;
	
	cd_dbus_applet_init_signals_once (klass);
	
	dbus_g_object_type_install_info (cd_dbus_applet_get_type(), &dbus_glib_cd_dbus_applet_object_info);
}
static void cd_dbus_applet_init (dbusApplet *pDbusApplet)
{
	cd_debug("");
	
	pDbusApplet->connection = cairo_dock_get_session_connection ();
	pDbusApplet->proxy = cairo_dock_get_main_proxy ();
	pDbusApplet->pSubApplet = g_object_new (cd_dbus_sub_applet_get_type(), NULL);
	pDbusApplet->pSubApplet->pApplet = pDbusApplet;
}

static void cd_dbus_applet_dispose (GObject *object)
{
	dbusApplet *pDbusApplet = (dbusApplet*)object;
	if (pDbusApplet->pSubApplet != NULL)
	{
		g_object_unref (pDbusApplet->pSubApplet);
		pDbusApplet->pSubApplet = NULL;
	}
}

static void cd_dbus_applet_finalize (GObject *object)
{
	dbusApplet *pDbusApplet = (dbusApplet*)object;
	g_free (pDbusApplet->cBusPath);
	pDbusApplet->cBusPath = NULL;
}

static void cd_dbus_sub_applet_class_init(dbusSubAppletClass *klass)
{
	cd_debug("");
	
	cd_dbus_sub_applet_init_signals_once (klass);
	
	dbus_g_object_type_install_info (cd_dbus_sub_applet_get_type(), &dbus_glib_cd_dbus_sub_applet_object_info);
}
static void cd_dbus_sub_applet_init (dbusSubApplet *pDbusSubApplet)
{
	cd_debug("");
}


dbusApplet * cd_dbus_get_dbus_applet_from_instance (CairoDockModuleInstance *pModuleInstance)
{
	dbusApplet *pDbusApplet = NULL;
	GList *a;
	for (a = s_pAppletList; a != NULL; a = a->next)
	{
		pDbusApplet = a->data;
		if (pDbusApplet->pModuleInstance == pModuleInstance)
			return pDbusApplet;
	}
	return NULL;
}


#define _applet_list_is_empty() (s_pAppletList == NULL)

dbusApplet *cd_dbus_create_remote_applet_object (CairoDockModuleInstance *pModuleInstance)
{
	g_return_val_if_fail (pModuleInstance != NULL && myData.pMainObject != NULL, NULL);
	const gchar *cModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
	g_return_val_if_fail (cModuleName != NULL, NULL);
	cd_debug ("%s (%s)", __func__, cModuleName);
	
	//\_____________ unicity check.
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pModuleInstance);
	if (pDbusApplet != NULL)  // shouldn't arrive, but let's be cautious.
	{
		cd_warning ("this applet (%s) already has a remote object on the bus", cModuleName);
		return pDbusApplet;
	}
	
	//\_____________ create a DBus object corresponding to the applet.
	pDbusApplet = g_object_new (cd_dbus_applet_get_type(), NULL);  // appelle cd_dbus_applet_class_init() et cd_dbus_applet_init().
	pDbusApplet->cModuleName = g_strdup (cModuleName);
	pDbusApplet->pModuleInstance = pModuleInstance;
	pDbusApplet->id = s_iModuleId++;
	
	//\_____________ register it under a unique path.
	gchar *cSuffix = NULL;
	if (pModuleInstance->pModule->pInstancesList->next != NULL)  // if this is the only instance of the applet, don't add suffix (it's not needed, and it keeps backward compatibility).
		cSuffix = g_strdup_printf ("_%d", pDbusApplet->id);
	
	gchar *cNameWithoutHyphen = NULL;
	if (strchr (cModuleName, '-') != NULL)
	{
		cNameWithoutHyphen = g_strdup (cModuleName);
		int i;
		for (i = 0; cNameWithoutHyphen[i] != '\0'; i ++)
			if (cNameWithoutHyphen[i] == '-' || cNameWithoutHyphen[i] == ' ')
				cNameWithoutHyphen[i] = ' ';
		
	}
	pDbusApplet->cBusPath = g_strconcat (myData.cBasePath, "/", cNameWithoutHyphen ? cNameWithoutHyphen : cModuleName, cSuffix, NULL);
	g_free (cNameWithoutHyphen);
	g_free (cSuffix);
	
	dbus_g_connection_register_g_object (pDbusApplet->connection, pDbusApplet->cBusPath, G_OBJECT(pDbusApplet));
	
	gchar *cSubPath = g_strconcat (pDbusApplet->cBusPath, "/sub_icons", NULL);
	dbus_g_connection_register_g_object (pDbusApplet->connection, cSubPath, G_OBJECT(pDbusApplet->pSubApplet));
	g_free (cSubPath);
	
	//\_____________ register to the notifications we'll want to propagate on the bus.
	if (pDbusApplet->proxy != NULL && _applet_list_is_empty ())  // 1ere applet Dbus.
	{
		cairo_dock_register_notification_on_object (&myContainersMgr,
			NOTIFICATION_CLICK_ICON,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_click_icon,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
		cairo_dock_register_notification_on_object (&myContainersMgr,
			NOTIFICATION_MIDDLE_CLICK_ICON,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_middle_click_icon,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
		cairo_dock_register_notification_on_object (&myContainersMgr,
			NOTIFICATION_SCROLL_ICON,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_scroll_icon,
			CAIRO_DOCK_RUN_FIRST,
			NULL);
		cairo_dock_register_notification_on_object (&myContainersMgr,
			NOTIFICATION_BUILD_ICON_MENU,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_build_menu,
			CAIRO_DOCK_RUN_FIRST,
			NULL);
		cairo_dock_register_notification_on_object (&myDesktopMgr,
			NOTIFICATION_WINDOW_ACTIVATED,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_change_focus,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
		myData.xActiveWindow = cairo_dock_get_current_active_window ();
	}
	
	s_pAppletList = g_list_prepend (s_pAppletList, pDbusApplet);
	return pDbusApplet;
}

void cd_dbus_delete_remote_applet_object (dbusApplet *pDbusApplet)
{
	s_pAppletList = g_list_remove (s_pAppletList, pDbusApplet);
	
	if (_applet_list_is_empty ())  // si plus d'applet dbus, inutile de garder les notifications actives.
	{
		cd_dbus_unregister_notifications ();
	}
	
	if (pDbusApplet != NULL)
	{
		// on enleve les raccourcis clavier de l'applet.
		GList *kb;
		CairoKeyBinding *pKeyBinding;
		for (kb = pDbusApplet->pShortkeyList; kb != NULL; kb = kb->next)
		{
			pKeyBinding = kb->data;
			cd_keybinder_unbind (pKeyBinding);
		}
		g_list_free (pDbusApplet->pShortkeyList);
		pDbusApplet->pShortkeyList = NULL;
		
		// on detruit l'objet.
		g_object_unref (pDbusApplet);
	}
}

void cd_dbus_unregister_notifications (void)
{
	cairo_dock_remove_notification_func_on_object (&myContainersMgr,
		NOTIFICATION_CLICK_ICON,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_click_icon,
		NULL);
	cairo_dock_remove_notification_func_on_object (&myContainersMgr,
		NOTIFICATION_MIDDLE_CLICK_ICON,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_middle_click_icon,
		NULL);
	cairo_dock_remove_notification_func_on_object (&myContainersMgr,
		NOTIFICATION_SCROLL_ICON,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_scroll_icon,
		NULL);
	cairo_dock_remove_notification_func_on_object (&myContainersMgr,
		NOTIFICATION_BUILD_ICON_MENU,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_build_menu,
		NULL);
	cairo_dock_remove_notification_func_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_ACTIVATED,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_change_focus,
		NULL);
}


void cd_dbus_launch_applet_process (CairoDockModuleInstance *pModuleInstance, dbusApplet *pDbusApplet)
{
	const gchar *cModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
	const gchar *cDirPath = pModuleInstance->pModule->pVisitCard->cShareDataDir;
	cd_message ("%s (%s)", __func__, cModuleName);
	
 	gchar *cCommand = g_strdup_printf ("cd \"%s\" && ./\"%s\" %d \"%s\" \"%s\" \"%s\" %s %d",
 		cDirPath,
 		cModuleName,
 		pDbusApplet->id,
 		pDbusApplet->cBusPath,
 		pModuleInstance->cConfFilePath,
		g_cCairoDockDataDir,
 		myData.cProgName,
 		getpid());
	cd_debug ("launching distant applet with: '%s'", cCommand);
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);
}
