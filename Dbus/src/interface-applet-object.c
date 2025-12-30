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

#include "interface-applet-signals.h"
#include "interface-applet-methods.h"
#include "interface-applet-object.h"
#include "interface-applet-info.h"
#include "interface-sub-applet-info.h"
#include "applet-dbus.h"

static int s_iModuleId = 1;
static unsigned int s_uNbInstances = 0;

DBusAppletData *cd_dbus_get_dbus_applet_from_instance (GldiModuleInstance *pModuleInstance)
{
	return (DBusAppletData*)(pModuleInstance->pData);
}

gboolean cd_dbus_create_remote_applet_object (GldiModuleInstance *pModuleInstance)
{
	g_return_val_if_fail (pModuleInstance != NULL && myData.uRegMainObject != 0, FALSE);
	const gchar *cModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
	g_return_val_if_fail (cModuleName != NULL, FALSE);
	cd_debug ("%s (%s)", __func__, cModuleName);
	
	GError *err = NULL;
	
	//\_____________ initialize a DBus object corresponding to the applet.
	DBusAppletData *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pModuleInstance);
	pDbusApplet->cModuleName = g_strdup (cModuleName);
	pDbusApplet->pModuleInstance = pModuleInstance;
	pDbusApplet->id = s_iModuleId++;
	pDbusApplet->connection = cairo_dock_dbus_get_session_bus (); // should not be NULL if we are here
	g_return_val_if_fail (pDbusApplet->connection != NULL, FALSE);
	
	//\_____________ register it under a unique path.
	gchar *cSuffix = NULL;
	if (pModuleInstance->pModule->pInstancesList != NULL)  // if this is the only instance of the applet, don't add suffix (it's not needed, and it keeps backward compatibility).
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
	
	GDBusInterfaceVTable vtable;
	vtable.method_call = cd_dbus_applet_method_call;
	vtable.get_property = cd_dbus_applet_get_property;
	vtable.set_property = NULL;
	
	// See note about const cast in applet-dbus.c
	pDbusApplet->uRegApplet = g_dbus_connection_register_object (pDbusApplet->connection, pDbusApplet->cBusPath,
		(GDBusInterfaceInfo*)&org_cairodock_cairo_dock_applet_interface, &vtable, pDbusApplet, NULL, &err);
	if (err)
	{
		cd_warning ("Error registering applet DBus object: %s", err->message);
		g_error_free (err);
		return FALSE;
	}
	
	vtable.method_call = cd_dbus_sub_applet_method_call;
	vtable.get_property = cd_dbus_sub_applet_get_property;
	pDbusApplet->cBusPathSub = g_strconcat (pDbusApplet->cBusPath, "/sub_icons", NULL);
	pDbusApplet->uRegSubApplet = g_dbus_connection_register_object (pDbusApplet->connection, pDbusApplet->cBusPathSub,
		(GDBusInterfaceInfo*)&org_cairodock_cairo_dock_subapplet_interface, &vtable, pDbusApplet, NULL, &err);
	if (err)
	{
		cd_warning ("Error registering subapplet DBus object: %s", err->message);
		g_error_free (err);
		g_dbus_connection_unregister_object (pDbusApplet->connection, pDbusApplet->uRegApplet);
		pDbusApplet->uRegApplet = 0;
		return FALSE;
	}
	
	//\_____________ register to the notifications we'll want to propagate on the bus.
	if (!s_uNbInstances)  // 1ere applet Dbus.
	{
		gldi_object_register_notification (&myContainerObjectMgr,
			NOTIFICATION_CLICK_ICON,
			(GldiNotificationFunc) cd_dbus_applet_emit_on_click_icon,
			GLDI_RUN_AFTER,
			NULL);
		gldi_object_register_notification (&myContainerObjectMgr,
			NOTIFICATION_MIDDLE_CLICK_ICON,
			(GldiNotificationFunc) cd_dbus_applet_emit_on_middle_click_icon,
			GLDI_RUN_AFTER,
			NULL);
		gldi_object_register_notification (&myContainerObjectMgr,
			NOTIFICATION_SCROLL_ICON,
			(GldiNotificationFunc) cd_dbus_applet_emit_on_scroll_icon,
			GLDI_RUN_FIRST,
			NULL);
		gldi_object_register_notification (&myContainerObjectMgr,
			NOTIFICATION_BUILD_ICON_MENU,
			(GldiNotificationFunc) cd_dbus_applet_emit_on_build_menu,
			GLDI_RUN_FIRST,
			NULL);
		gldi_object_register_notification (&myWindowObjectMgr,
			NOTIFICATION_WINDOW_ACTIVATED,
			(GldiNotificationFunc) cd_dbus_applet_emit_on_change_focus,
			GLDI_RUN_AFTER,
			NULL);
		myData.pActiveWindow = gldi_windows_get_active ();
	}
	
	s_uNbInstances++;
	
	return TRUE;
}

void cd_dbus_delete_remote_applet_object (DBusAppletData *pDbusApplet)
{
	if (! s_uNbInstances) cd_warning ("Inconsistent applet instance count!");
	else
	{
		s_uNbInstances--;
		if (!s_uNbInstances)  // si plus d'applet dbus, inutile de garder les notifications actives.
			cd_dbus_unregister_notifications ();
	}

	// on enleve les raccourcis clavier de l'applet.
	GList *kb;
	GldiShortkey *pKeyBinding;
	for (kb = pDbusApplet->pShortkeyList; kb != NULL; kb = kb->next)
	{
		pKeyBinding = kb->data;
		gldi_object_unref (GLDI_OBJECT(pKeyBinding));
	}
	g_list_free (pDbusApplet->pShortkeyList);
	pDbusApplet->pShortkeyList = NULL;
	
	// on detruit l'objet.
	if (pDbusApplet->uRegSubApplet)
		g_dbus_connection_unregister_object (pDbusApplet->connection, pDbusApplet->uRegSubApplet);
	if (pDbusApplet->uRegApplet)
		g_dbus_connection_unregister_object (pDbusApplet->connection, pDbusApplet->uRegApplet);
}

void cd_dbus_unregister_notifications (void)
{
	gldi_object_remove_notification (&myContainerObjectMgr,
		NOTIFICATION_CLICK_ICON,
		(GldiNotificationFunc) cd_dbus_applet_emit_on_click_icon,
		NULL);
	gldi_object_remove_notification (&myContainerObjectMgr,
		NOTIFICATION_MIDDLE_CLICK_ICON,
		(GldiNotificationFunc) cd_dbus_applet_emit_on_middle_click_icon,
		NULL);
	gldi_object_remove_notification (&myContainerObjectMgr,
		NOTIFICATION_SCROLL_ICON,
		(GldiNotificationFunc) cd_dbus_applet_emit_on_scroll_icon,
		NULL);
	gldi_object_remove_notification (&myContainerObjectMgr,
		NOTIFICATION_BUILD_ICON_MENU,
		(GldiNotificationFunc) cd_dbus_applet_emit_on_build_menu,
		NULL);
	gldi_object_remove_notification (&myWindowObjectMgr,
		NOTIFICATION_WINDOW_ACTIVATED,
		(GldiNotificationFunc) cd_dbus_applet_emit_on_change_focus,
		NULL);
}


void cd_dbus_launch_applet_process (GldiModuleInstance *pModuleInstance)
{
	const gchar *cModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
	const gchar *cDirPath = pModuleInstance->pModule->pVisitCard->cShareDataDir;
	cd_message ("%s (%s)", __func__, cModuleName);
	
	DBusAppletData *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pModuleInstance);
	
	gchar *cExec = g_strdup_printf ("./%s", cModuleName);
	gchar *cID = g_strdup_printf ("%d", pDbusApplet->id);
	gchar *cPid = g_strdup_printf ("%d", getpid ());
	const gchar * const args[] = {cExec, cID, pDbusApplet->cBusPath,
		pModuleInstance->cConfFilePath, g_cCairoDockDataDir,
		myData.cProgName, cPid, NULL};
	cd_debug ("launching distant applet: %s/%s", cDirPath, cModuleName);
	cd_dbus_launch_subprocess (args, cDirPath);
	g_free (cExec);
	g_free (cID);
	g_free (cPid);
}

