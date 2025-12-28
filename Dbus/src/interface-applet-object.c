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
#include "applet-dbus.h"

static int s_iModuleId = 1;
static unsigned int s_uNbInstances = 0;

static const char *s_cAppletXml;
static const char *s_cSubAppletXml;

static GDBusNodeInfo *s_pAppletNodeInfo = NULL;
static GDBusNodeInfo *s_pSubAppletNodeInfo = NULL;

DBusAppletData *cd_dbus_get_dbus_applet_from_instance (GldiModuleInstance *pModuleInstance)
{
	return (DBusAppletData*)(pModuleInstance->pData);
}

static void _parse_interface_xml (void)
{
	GError *err = NULL;
	
	if (!s_pAppletNodeInfo)
	{
		s_pAppletNodeInfo = g_dbus_node_info_new_for_xml (s_cAppletXml, &err);
		if (err)
		{
			cd_warning ("Cannot parse DBus interface XML: %s", err->message);
			g_error_free (err);
			return;
		}
	}
	
	if (!s_pSubAppletNodeInfo)
	{
		s_pSubAppletNodeInfo = g_dbus_node_info_new_for_xml (s_cSubAppletXml, &err);
		if (err)
		{
			cd_warning ("Cannot parse DBus interface XML: %s", err->message);
			g_error_free (err);
			return;
		}
	}
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
	pDbusApplet->connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL); // should be immediate and should not fail (we already connected)
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
	
	_parse_interface_xml ();
	if (! (s_pAppletNodeInfo && s_pSubAppletNodeInfo)) return FALSE;
	
	// note: return value is owned by pNodeInfo
	GDBusInterfaceInfo *pInfo = g_dbus_node_info_lookup_interface (s_pAppletNodeInfo, "org.cairodock.CairoDock.applet");
	
	GDBusInterfaceVTable vtable;
	vtable.method_call = cd_dbus_applet_method_call;
	vtable.get_property = cd_dbus_applet_get_property;
	vtable.set_property = NULL;
	
	pDbusApplet->uRegApplet = g_dbus_connection_register_object (pDbusApplet->connection, pDbusApplet->cBusPath, pInfo, &vtable, pDbusApplet, NULL, &err);
	if (err)
	{
		cd_warning ("Error registering applet DBus object: %s", err->message);
		g_error_free (err);
		return FALSE;
	}
	
	pInfo = g_dbus_node_info_lookup_interface (s_pSubAppletNodeInfo, "org.cairodock.CairoDock.subapplet");
	vtable.method_call = cd_dbus_sub_applet_method_call;
	vtable.get_property = cd_dbus_sub_applet_get_property;
	pDbusApplet->cBusPathSub = g_strconcat (pDbusApplet->cBusPath, "/sub_icons", NULL);
	pDbusApplet->uRegSubApplet = g_dbus_connection_register_object (pDbusApplet->connection, pDbusApplet->cBusPathSub, pInfo, &vtable, pDbusApplet, NULL, &err);
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



static const char *s_cAppletXml = 
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\""
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">"
"<node name=\"/org/cairodock/CairoDock\">"
"	<interface name=\"org.cairodock.CairoDock.applet\">"
"		<method name=\"Get\">"
"			<arg name=\"cProperty\" direction=\"in\" type=\"s\"/>"
"			<arg name=\"value\" direction=\"out\" type=\"v\"/>"
"		</method>"
"		<method name=\"GetAll\">"
"			<arg name=\"hProperties\" direction=\"out\" type=\"a{sv}\"/>"
"		</method>"
"		"
"		<method name=\"SetQuickInfo\">"
"			<arg name=\"cQuickInfo\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"SetLabel\">"
"			<arg name=\"cLabel\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"SetIcon\">"
"			<arg name=\"cImage\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"SetEmblem\">"
"			<arg name=\"cImage\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"iPosition\" type=\"i\" direction=\"in\"/>"
"		</method>"
"		<method name=\"Animate\">"
"			<arg name=\"cAnimation\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"iNbRounds\" type=\"i\" direction=\"in\"/>"
"		</method>"
"		<method name=\"DemandsAttention\">"
"			<arg name=\"bStart\" type=\"b\" direction=\"in\"/>"
"			<arg name=\"cAnimation\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"ShowDialog\">"
"			<arg name=\"cMessage\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"iDuration\" type=\"i\" direction=\"in\"/>"
"		</method>"
"		<method name=\"PopupDialog\">"
"			<arg name=\"hDialogAttributes\" direction=\"in\" type=\"a{sv}\"/>"
"			<arg name=\"hWidgetAttributes\" direction=\"in\" type=\"a{sv}\"/>"
"		</method>"
"		<method name=\"AddDataRenderer\">"
"			<arg name=\"cType\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"iNbValues\" type=\"i\" direction=\"in\"/>"
"			<arg name=\"cTheme\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"RenderValues\">"
"			<arg name=\"pValues\" type=\"ad\" direction=\"in\"/>"
"		</method>"
"		<method name=\"ControlAppli\">"
"			<arg name=\"cApplicationClass\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"ActOnAppli\">"
"			<arg name=\"cAction\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"AddMenuItems\">"
"			<arg name=\"pItems\" type=\"aa{sv}\" direction=\"in\"/>"
"		</method>"
"		<method name=\"BindShortkey\">"
"			<arg name=\"cShortkeys\" type=\"as\" direction=\"in\"/>"
"		</method>"
"		<!-- Deprecated since 3.0 -->"
"		<method name=\"AskQuestion\">"
"			<arg name=\"cMessage\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"AskValue\">"
"			<arg name=\"cMessage\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"fInitialValue\" type=\"d\" direction=\"in\"/>"
"			<arg name=\"fMaxlValue\" type=\"d\" direction=\"in\"/>"
"		</method>"
"		<method name=\"AskText\">"
"			<arg name=\"cMessage\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cInitialText\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"ShowAppli\">"
"			<arg name=\"bShow\" type=\"b\" direction=\"in\"/>"
"		</method>"
"		<method name=\"PopulateMenu\">"
"			<arg name=\"pLabels\" type=\"as\" direction=\"in\"/>"
"		</method>"
"		<!-- End of deprecated -->"
"		"
"		<signal name=\"on_click\">"
"			<arg name=\"iButtonState\" type=\"i\"/>"
"		</signal>"
"		<signal name=\"on_middle_click\">"
"		</signal>"
"		<signal name=\"on_scroll\">"
"			<arg name=\"bDirectionUp\" type=\"b\"/>"
"		</signal>"
"		<signal name=\"on_build_menu\">"
"		</signal>"
"		<signal name=\"on_menu_select\">"
"			<arg name=\"iNumEntry\" type=\"i\"/>"
"		</signal>"
"		<signal name=\"on_drop_data\">"
"			<arg name=\"cReceivedData\" type=\"s\"/>"
"		</signal>"
"		<signal name=\"on_change_focus\">"
"			<arg name=\"is_active\" type=\"b\"/>"
"		</signal>"
"		<signal name=\"on_answer\">"
"			<arg name=\"answer\" type=\"v\"/>"
"		</signal>"
"		<signal name=\"on_answer_dialog\">"
"			<arg name=\"iClickedButton\" type=\"i\"/>"
"			<arg name=\"answer\" type=\"v\"/>"
"		</signal>"
"		<signal name=\"on_shortkey\">"
"			<arg name=\"cShortkey\" type=\"s\"/>"
"		</signal>"
"		"
"		<signal name=\"on_stop_module\">"
"		</signal>"
"		<signal name=\"on_reload_module\">"
"			<arg name=\"bConfigHasChanged\" type=\"b\"/>"
"		</signal>"
"		"
"		<!-- Properties; NOTE: to limit the number of messages, the"
"		org.freedesktop.DBus.Properties.PropertiesChanged signal will NOT"
"		be emitted for these! You will need to read the property values"
"		directly with the org.freedesktop.DBus.Properties.Get or"
"		org.cairodock.CairoDock.applet.Get methods. -->"
"		<property type=\"i\" name=\"x\" access=\"read\" />"
"		<property type=\"i\" name=\"y\" access=\"read\" />"
"		<property type=\"i\" name=\"width\" access=\"read\" />"
"		<property type=\"i\" name=\"height\" access=\"read\" />"
"		<property type=\"u\" name=\"orientation\" access=\"read\" />"
"		<property type=\"u\" name=\"container\" access=\"read\" />"
"		<property type=\"t\" name=\"Xid\" access=\"read\" />"
"		<property type=\"b\" name=\"has_focus\" access=\"read\" />"
"		"
"	</interface>"
"</node>";

static const char *s_cSubAppletXml = 
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\""
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">"
"<node name=\"/org/cairodock/CairoDock\">"
"	<interface name=\"org.cairodock.CairoDock.subapplet\">"
"		<method name=\"SetQuickInfo\">"
"			<arg name=\"cQuickInfo\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cIconID\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"SetLabel\">"
"			<arg name=\"cLabel\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cIconID\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"SetIcon\">"
"			<arg name=\"cImage\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cIconID\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"SetEmblem\">"
"			<arg name=\"cImage\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"iPosition\" type=\"i\" direction=\"in\"/>"
"			<arg name=\"cIconID\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"Animate\">"
"			<arg name=\"cAnimation\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"iNbRounds\" type=\"i\" direction=\"in\"/>"
"			<arg name=\"cIconID\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"ShowDialog\">"
"			<arg name=\"message\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"iDuration\" type=\"i\" direction=\"in\"/>"
"			<arg name=\"cIconID\" type=\"s\" direction=\"in\"/>"
"		</method>>"
"		<!-- Deprecated since 3.0 -->"
"		<method name=\"AskQuestion\">"
"			<arg name=\"cMessage\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cIconID\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"AskValue\">"
"			<arg name=\"cMessage\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"fInitialValue\" type=\"d\" direction=\"in\"/>"
"			<arg name=\"fMaxlValue\" type=\"d\" direction=\"in\"/>"
"			<arg name=\"cIconID\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"AskText\">"
"			<arg name=\"cMessage\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cInitialText\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cIconID\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<!-- End of deprecated -->"
"		"
"		<method name=\"PopupDialog\">"
"			<arg name=\"hDialogAttributes\" direction=\"in\" type=\"a{sv}\"/>"
"			<arg name=\"hWidgetAttributes\" direction=\"in\" type=\"a{sv}\"/>"
"			<arg name=\"cIconID\" direction=\"in\" type=\"s\" />"
"		</method>"
"		"
"		"
"		<method name=\"AddSubIcons\">"
"			<arg name=\"pIconFields\" type=\"as\" direction=\"in\"/>"
"		</method>"
"		<method name=\"RemoveSubIcon\">"
"			<arg name=\"cIconID\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		"
"		<signal name=\"on_click_sub_icon\">"
"			<arg name=\"iButtonState\" type=\"i\"/>"
"			<arg name=\"cIconID\" type=\"s\"/>"
"		</signal>"
"		<signal name=\"on_middle_click_sub_icon\">"
"			<arg name=\"cIconID\" type=\"s\"/>"
"		</signal>"
"		<signal name=\"on_scroll_sub_icon\">"
"			<arg name=\"bDirectionUp\" type=\"b\"/>"
"			<arg name=\"cIconID\" type=\"s\"/>"
"		</signal>"
"		<signal name=\"on_build_menu_sub_icon\">"
"			<arg name=\"cIconID\" type=\"s\"/>"
"		</signal>"
"		<signal name=\"on_drop_data_sub_icon\">"
"			<arg name=\"cReceivedData\" type=\"s\"/>"
"			<arg name=\"cIconID\" type=\"s\"/>"
"		</signal>"
"	</interface>"
"</node>";

