/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* based on indicator-me.c written by :
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

#include "indicator-applet.h"

#define INDICATOR_APPLET_DBUS_VERSION  1


static void _cd_indicator_make_menu (CDAppletIndicator *pIndicator)
{
	if (pIndicator->pMenu == NULL)
	{
		pIndicator->pMenu = dbusmenu_gtkmenu_new (pIndicator->cBusName, pIndicator->cMenuObject);
		if (pIndicator->pMenu != NULL)
		{
			DbusmenuGtkClient * client = dbusmenu_gtkmenu_get_client (pIndicator->pMenu);
			
			if (pIndicator->add_menu_handler)
				pIndicator->add_menu_handler (client);
		}
	}
}

static gboolean _get_menu_once (CDAppletIndicator *pIndicator)
{
	_cd_indicator_make_menu (pIndicator);
	pIndicator->iSidGetMenuOnce = 0;
	return FALSE;
}

static void
connection_changed (IndicatorServiceManager * sm, gboolean connected, CDAppletIndicator *pIndicator)
{
	CairoDockModuleInstance *myApplet = pIndicator->pApplet;
	if (connected)
	{
		// connect to the service.
		if (pIndicator->pServiceProxy == NULL)
		{
			GError * error = NULL;
			DBusGConnection * sbus = cairo_dock_get_session_connection ();
			pIndicator->pServiceProxy = dbus_g_proxy_new_for_name_owner(sbus,
				pIndicator->cBusName,
				pIndicator->cServiceObject,
				pIndicator->cServiceInterface,
				&error);
			if (error != NULL)
			{
				cd_warning ("'%s' service not found on the bus : %s", pIndicator->cServiceObject, error->message);
				g_error_free(error);
			}
			if (pIndicator->pServiceProxy == NULL)
				return;
			
			if (pIndicator->on_connect)
				pIndicator->on_connect (myApplet);
		}
		
		// query the service to display initial values.
		if (pIndicator->get_initial_values)
			pIndicator->get_initial_values (myApplet);
		
		pIndicator->iSidGetMenuOnce = g_idle_add ((GSourceFunc)_get_menu_once, pIndicator);
		pIndicator->bConnected = TRUE;
	}
	else  // If we're disconnecting, go back to offline.
	{
		if (pIndicator->on_disconnect)
			pIndicator->on_disconnect (myApplet);
		pIndicator->bConnected = FALSE;
	}

	return;
}

CDAppletIndicator *cd_indicator_new (CairoDockModuleInstance *pApplet, const gchar *cBusName, const gchar *cServiceObject, const gchar *cServiceInterface, const gchar *cMenuObject)
{
	CDAppletIndicator *pIndicator = g_new0 (CDAppletIndicator, 1);
	pIndicator->pApplet = pApplet;
	pIndicator->cBusName = cBusName;
	pIndicator->cServiceObject = cServiceObject;
	pIndicator->cServiceInterface = cServiceInterface;
	pIndicator->cMenuObject = cMenuObject;
	
	pIndicator->service = indicator_service_manager_new_version (cBusName, INDICATOR_APPLET_DBUS_VERSION);
	g_signal_connect (G_OBJECT(pIndicator->service), INDICATOR_SERVICE_MANAGER_SIGNAL_CONNECTION_CHANGE, G_CALLBACK(connection_changed), pIndicator);  // on sera appele une fois la connexion etablie.
	
	return pIndicator;
}


void cd_indicator_destroy (CDAppletIndicator *pIndicator)
{
	if (!pIndicator)
		return;
	if (pIndicator->iSidGetMenuOnce != 0)
		g_source_remove (pIndicator->iSidGetMenuOnce);
	if (pIndicator->service)
		g_object_unref (pIndicator->service);
	if (pIndicator->pServiceProxy)
		g_object_unref (pIndicator->pServiceProxy);
	g_print ("destroy menu...\n");
	if (pIndicator->pMenu)
		g_object_ref_sink (pIndicator->pMenu);
	g_print ("done.\n");
	g_free (pIndicator);
}


void cd_indicator_set_icon (CDAppletIndicator *pIndicator, const gchar *cStatusIcon)
{
	CairoDockModuleInstance *myApplet = pIndicator->pApplet;
	if (cStatusIcon != pIndicator->cStatusIcon)
	{
		g_free (pIndicator->cStatusIcon);
		pIndicator->cStatusIcon = g_strdup (cStatusIcon);
	}
	if (cStatusIcon == NULL)
		return;
	
	gchar *cIconPath = cairo_dock_search_icon_s_path (cStatusIcon);
	gchar *cIconPathFallback = NULL;
	if (cIconPath == NULL)  // l'icone ne sera pas trouvee, on met une icone par defaut.
		cIconPathFallback = g_strdup_printf ("%s/%s.svg", myApplet->pModule->pVisitCard->cShareDataDir, cStatusIcon);
	
	g_print ("set %s\n", cIconPathFallback ? cIconPathFallback : cStatusIcon);
	CD_APPLET_SET_IMAGE_ON_MY_ICON (cIconPathFallback ? cIconPathFallback : cStatusIcon);
	
	g_free (cIconPath);
	g_free (cIconPathFallback);
}


void cd_indicator_reload_icon (CDAppletIndicator *pIndicator)
{
	cd_indicator_set_icon (pIndicator, pIndicator->cStatusIcon);
}


gboolean cd_indicator_show_menu (CDAppletIndicator *pIndicator)
{
	_cd_indicator_make_menu (pIndicator);
	if (pIndicator->pMenu != NULL)
	{
		cairo_dock_popup_menu_on_container (GTK_WIDGET (pIndicator->pMenu), myContainer);
		return TRUE;
	}
	return FALSE;
}
