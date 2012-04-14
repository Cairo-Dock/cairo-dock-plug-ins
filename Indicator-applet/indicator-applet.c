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

static gboolean s_bIndicatorIconThemeAdded = FALSE;

static void _on_menu_destroyed (CDAppletIndicator *pIndicator, GObject *old_menu_pointer)
{
	cd_debug ("no more menu (%p / %p)", old_menu_pointer, pIndicator->pMenu);
	if (old_menu_pointer == (GObject*)pIndicator->pMenu)
		pIndicator->pMenu = NULL;
}
static void _on_service_destroyed (CDAppletIndicator *pIndicator, GObject *old_menu_pointer)
{
	cd_debug ("no more service (%p / %p)", old_menu_pointer, pIndicator->service);
	if (old_menu_pointer == (GObject*)pIndicator->service)
		pIndicator->service = NULL;
}

static void _cd_indicator_make_menu (CDAppletIndicator *pIndicator)
{
	if (pIndicator->pMenu == NULL)
	{
		pIndicator->pMenu = dbusmenu_gtkmenu_new ((gchar*)pIndicator->cBusName, (gchar*)pIndicator->cMenuObject);  // the cast is unorthodox, but the function definition is clumsy (it should require 2 const gchar*, since it actually duplicates the strings).
		if (pIndicator->pMenu != NULL)
		{
			g_object_ref_sink (G_OBJECT (pIndicator->pMenu));  // the object is floating -> take the reference
			g_object_weak_ref (G_OBJECT (pIndicator->pMenu),
				(GWeakNotify)_on_menu_destroyed,
				pIndicator);
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
	cd_debug ("%s (%s : %d)", __func__, pIndicator->cBusName, connected);
	CairoDockModuleInstance *myApplet = pIndicator->pApplet;
	if (connected)
	{
		if (!pIndicator->bConnected)
		{
			// connect to the service.
			if (pIndicator->pServiceProxy == NULL)
			{
				/**GError * error = NULL;
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
				}*/
				pIndicator->pServiceProxy = cairo_dock_create_new_session_proxy (
					pIndicator->cBusName,
					pIndicator->cServiceObject,
					pIndicator->cServiceInterface);
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
	}
	else  // If we're disconnecting, go back to offline.
	{
		if (pIndicator->bConnected)
		{
			if (pIndicator->on_disconnect)
				pIndicator->on_disconnect (myApplet);
			if (pIndicator->pMenu)
			{
				cd_debug ("destroy menu...");
				g_object_unref (pIndicator->pMenu);
				cd_debug ("done.");
				pIndicator->pMenu = NULL;
			}
			if (pIndicator->pServiceProxy != NULL)
			{
				g_object_unref (pIndicator->pServiceProxy);  // this removes all the signals connected on the proxy
				pIndicator->pServiceProxy = NULL;
			}
			pIndicator->bConnected = FALSE;
		}
	}

	return;
}

static gboolean _check_indicator (CDAppletIndicator *pIndicator)
{
	CairoDockModuleInstance *myApplet = pIndicator->pApplet;
	if (!pIndicator->bConnected && pIndicator->on_disconnect)
		pIndicator->on_disconnect (myApplet);
		
	pIndicator->iSidCheckIndicator = 0;
	return FALSE;
}

CDAppletIndicator *cd_indicator_new (CairoDockModuleInstance *pApplet, const gchar *cBusName, const gchar *cServiceObject, const gchar *cServiceInterface, const gchar *cMenuObject, int iVersion)
{
	if (!s_bIndicatorIconThemeAdded)
	{
		s_bIndicatorIconThemeAdded = TRUE;
		cairo_dock_add_path_to_icon_theme (INDICATOR_ICONS_DIR);  /// maybe we should add it back when the icons manager is reloaded ?...
	}
	CDAppletIndicator *pIndicator = g_new0 (CDAppletIndicator, 1);
	pIndicator->pApplet = pApplet;
	pIndicator->cBusName = cBusName;
	pIndicator->cServiceObject = cServiceObject;
	pIndicator->cServiceInterface = cServiceInterface;
	pIndicator->cMenuObject = cMenuObject;
	
	pIndicator->service = indicator_service_manager_new_version ((gchar*)cBusName, iVersion);
	g_object_weak_ref (G_OBJECT (pIndicator->service),
		(GWeakNotify)_on_service_destroyed,
		pIndicator);  // most probably useless, but let's be parano, since we're using the dreadful "libindicator" :p
		
	g_signal_connect (G_OBJECT(pIndicator->service), INDICATOR_SERVICE_MANAGER_SIGNAL_CONNECTION_CHANGE, G_CALLBACK(connection_changed), pIndicator);  // on sera appele une fois la connexion etablie.  // pour le cast, cf plus haut.
	
	// indicators don't send the 'connection-change' signal if the connection couldn't be done, so we have to handle this case ourselves.
	pIndicator->iSidCheckIndicator = g_timeout_add_seconds (3, (GSourceFunc)_check_indicator, pIndicator);
	
	return pIndicator;
}


void cd_indicator_destroy (CDAppletIndicator *pIndicator)
{
	if (!pIndicator)
		return;
	if (pIndicator->iSidGetMenuOnce != 0)
		g_source_remove (pIndicator->iSidGetMenuOnce);
	if (pIndicator->iSidCheckIndicator != 0)
		g_source_remove (pIndicator->iSidCheckIndicator);
	pIndicator->bConnected = FALSE;
	pIndicator->on_disconnect = NULL;  // since the indicator has been explicitely destroyed, we don't want to call the callback when we'll disconnect from the service.
	cd_debug ("destroy indicator menu...");
	if (pIndicator->pMenu)
		g_object_unref (pIndicator->pMenu);
	cd_debug ("done.");
	if (pIndicator->pServiceProxy)
	{
		g_object_unref (pIndicator->pServiceProxy);
		pIndicator->pServiceProxy = NULL;
	}
	cd_debug ("destroy service...");
	if (pIndicator->service)
	{
		//g_object_unref (pIndicator->service);  // this causes a crash in libindicator (beacuse they keep the service as parameter of the callback 'service_proxy_name_changed'). so just be sure to disconnect from this object, and forget it.
		g_signal_handlers_disconnect_by_func (G_OBJECT(pIndicator->service), G_CALLBACK(connection_changed), pIndicator);
		g_object_weak_unref (G_OBJECT (pIndicator->service),
			(GWeakNotify)_on_service_destroyed,
			pIndicator);
		pIndicator->service = NULL;
	}
	cd_debug ("done.");
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
	
	const gchar *cIconName = cStatusIcon;
	gchar *tmp_icon_name = NULL;
	gchar *cIconPath = cairo_dock_search_icon_s_path (cIconName, MAX (myIcon->iImageWidth, myIcon->iImageHeight));  // on regarde si l'icone sera trouvee.
	gchar *cIconPathFallback = NULL;
	if (cIconPath == NULL)  // l'icone ne sera pas trouvee, on regarde si ce n'est pas une icone en carton d'Ubuntu.
	{
		gchar *str = g_strstr_len (cIconName, -1, "-panel");
		if (str)
		{
			tmp_icon_name = g_strndup (cIconName, str - cIconName);
			cIconName = tmp_icon_name;
			cIconPath = cairo_dock_search_icon_s_path (cIconName, MAX (myIcon->iImageWidth, myIcon->iImageHeight));
		}
	}
	if (cIconPath == NULL)  // l'icone ne sera pas trouvee, on met une icone par defaut.
	{
		gboolean bAddSuffix = (!g_str_has_suffix (cIconName, ".png") && !g_str_has_suffix (cIconName, ".svg"));
		cIconPathFallback = g_strdup_printf ("%s/%s%s", myApplet->pModule->pVisitCard->cShareDataDir, cIconName, bAddSuffix ? ".svg" : "");
	}
	
	cd_debug ("set %s", cIconPathFallback ? cIconPathFallback : cIconName);
	//CD_APPLET_SET_IMAGE_ON_MY_ICON (cIconPathFallback ? cIconPathFallback : cIconName);
	cairo_dock_set_image_on_icon_with_default (myDrawContext,
		cIconPathFallback ? cIconPathFallback : cIconName,
		myIcon,
		myContainer,
		myApplet->pModule->pVisitCard->cIconFilePath);
	
	g_free (tmp_icon_name);
	g_free (cIconPath);
	g_free (cIconPathFallback);
}


void cd_indicator_reload_icon (CDAppletIndicator *pIndicator)
{
	g_return_if_fail (pIndicator != NULL);
	cd_indicator_set_icon (pIndicator, pIndicator->cStatusIcon);
}


gboolean cd_indicator_show_menu (CDAppletIndicator *pIndicator)
{
	if (pIndicator->bConnected)
	{
		_cd_indicator_make_menu (pIndicator);
		if (pIndicator->pMenu != NULL)
		{
			cairo_dock_popup_menu_on_icon (GTK_WIDGET (pIndicator->pMenu), myIcon, myContainer);
			return TRUE;
		}
	}
	return FALSE;
}
