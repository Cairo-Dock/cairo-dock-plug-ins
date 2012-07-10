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

#include "stdlib.h"
#include "applet-config.h"
#include "applet-connections.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-netspeed.h"
#include "applet-wifi.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN (N_("Network-Monitor"),
	2, 1, 4,
	CAIRO_DOCK_CATEGORY_APPLET_INTERNET,
	N_("This applet allows you to monitor your network connection(s).\n"
	"It can display the download/upload speeds and the wifi signal quality.\n"
	"If you have network-manager running, it can also let you choose the current wifi network.\n"
	"Left-click to pop-up some info,"
	"Scroll on the icon to switch the display between net speed and wifi."),
	"Yann Sladek (Mav), Remy Robertson (ChanGFu), and Fabrice Rey (Fabounet)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	pInterface->load_custom_widget = cd_netmonitor_load_custom_widget;
CD_APPLET_DEFINE_END


static CairoDataRendererAttribute *make_data_renderer_attribute (Icon *myIcon, CDRenderer *pRendererParams)
{
	if (pRendererParams->iRenderType == CD_EFFECT_ICON)
		return NULL; /// TODO

	CairoDataRendererAttribute *pRenderAttr = NULL;  // attributes for the global data-renderer.
	CairoGaugeAttribute aGaugeAttr;  // gauge attributes.
	CairoGraphAttribute aGraphAttr;  // graph attributes.
	if (pRendererParams->iRenderType == CD_EFFECT_GAUGE)
	{
		memset (&aGaugeAttr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aGaugeAttr);
		pRenderAttr->cModelName = "gauge";
		aGaugeAttr.cThemePath = pRendererParams->cGThemePath;
	}
	else if (pRendererParams->iRenderType == CD_EFFECT_GRAPH)
	{
		memset (&aGraphAttr, 0, sizeof (CairoGraphAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aGraphAttr);
		pRenderAttr->cModelName = "graph";
		pRenderAttr->iMemorySize = (myIcon->fWidth > 1 ? myIcon->fWidth : 32);  // fWidht peut etre <= 1 en mode desklet au chargement.
		aGraphAttr.iType = pRendererParams->iGraphType;
		aGraphAttr.iRadius = 10;
		aGraphAttr.fHighColor = pRendererParams->fHigholor;
		aGraphAttr.fLowColor = pRendererParams->fLowColor;
		memcpy (aGraphAttr.fBackGroundColor, pRendererParams->fBgColor, 4*sizeof (double));
	}

	return pRenderAttr;
}

static void _set_data_renderer (CairoDockModuleInstance *myApplet, gboolean bReload)
{
	CairoDataRendererAttribute *pRenderAttr = make_data_renderer_attribute (myIcon, (myConfig.bModeWifi ? &myConfig.wifiRenderer : &myConfig.netSpeedRenderer));  // les attributs du data-renderer global.
	
	if (pRenderAttr != NULL)
	{
		pRenderAttr->iLatencyTime = (myConfig.bModeWifi ? myConfig.iWifiCheckInterval : myConfig.iNetspeedCheckInterval) * 1000 * myConfig.fSmoothFactor;
		//pRenderAttr->bWriteValues = TRUE;
		if (! bReload)
			CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON (pRenderAttr);
		else
			CD_APPLET_RELOAD_MY_DATA_RENDERER (pRenderAttr);
	}
}


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
		CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
	}
	// Initialisation du rendu.
	_set_data_renderer (myApplet, FALSE);
	myData.iPreviousQuality = -1;  // force le dessin.
	
	//\___________ Avant toute chose on se connecte a NM sur le bus (signaux Wifi actif et reseau actif)
	myData.bDbusConnection = cd_NetworkMonitor_connect_to_bus ();
	
	if (myData.bDbusConnection)
	{
		// on recupere la connection active sur l'interface souhait�e, ou la 1ere connexion active si aucune interface n'est specifiee.
		if (! cd_NetworkMonitor_get_active_connection_info())  // si aucune connexion courante, on parcourt la liste des devices connus, et on choisit dedans.
		{
			cd_NetworkMonitor_get_device ();
		}
	}
	
	if (myData.cDevice == NULL)  // soit NM n'est pas present, soit on a specifie une interface qui n'existe pas.
	{
		if (myConfig.cInterface != NULL)  // on a specifie une interface.
		{
			// on verifie que cette interface existe.
			int iType = cd_netmonitor_check_interface (myConfig.cInterface);
			if (iType == 0)  // l'interface n'existe pas.
			{
				cd_NetworkMonitor_draw_no_wireless_extension ();
				
				/// rajouter une embleme de warning ...
				
			}
			else  // l'interface existe.
			{
				if (iType == 2 && myConfig.bModeWifi)  // sans fil.
				{
					cd_netmonitor_launch_wifi_task (myApplet);
				}
				else
				{
					cd_netmonitor_launch_netspeed_task (myApplet);
				}
			}
		}
		else  // aucune interface n'est specifiee.
		{
			
		}
	}
	else
	{
		if (myData.bWiredExt || ! myConfig.bModeWifi)  // NM ne nous donne pas la vitesse de la connexion.
			cd_netmonitor_launch_netspeed_task (myApplet);
		else
			cd_NetworkMonitor_draw_icon ();
	}
	
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN

	// TO CHECK... other objects?
	if (myData.dbus_proxy_ActiveAccessPoint)
	{
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged",
			NULL, NULL);
		g_object_unref (myData.dbus_proxy_ActiveAccessPoint);
	}
	if (myData.dbus_proxy_ActiveAccessPoint_prop)
	{
		g_object_unref (myData.dbus_proxy_ActiveAccessPoint_prop);
	}
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	int i; // reset surfaces utilisateurs.
	for (i = 0; i < CONNECTION_NB_QUALITY; i ++) {
		if (myData.pSurfaces[i] != NULL) {
			cairo_surface_destroy (myData.pSurfaces[i]);
			myData.pSurfaces[i] = NULL;
		}
	}
	
	//\_______________ On relance avec la nouvelle config ou on redessine.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
			CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
		}
		
		cd_netmonitor_free_netspeed_task (myApplet);
		cd_netmonitor_free_wifi_task (myApplet);
		
		_set_data_renderer (myApplet, TRUE);
		
		myData.iPreviousQuality = -1;  // force le redessin.
		myData.iPercent = WIFI_QUALITY_NO_SIGNAL;
		myData.iSignalLevel = WIFI_QUALITY_NO_SIGNAL;
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		
		if (myConfig.bModeWifi)
		{
			if (myData.cDevice == NULL)  // on passe a la methode manuelle
				cd_netmonitor_launch_wifi_task (myApplet);  // sinon les signaux sont connectes.
			else
				cd_NetworkMonitor_draw_icon();
		}
		else
		{
			cd_netmonitor_launch_netspeed_task (myApplet);  // NM ne nous donne pas la vitesse de la connexion.
		}
	}
	else  // on redessine juste l'icone.
	{
		CD_APPLET_RELOAD_MY_DATA_RENDERER (NULL);  // on le recharge aux nouvelles dimensions de l'icone.
		CDRenderType iRenderType = (myConfig.bModeWifi ? myConfig.wifiRenderer.iRenderType : myConfig.netSpeedRenderer.iRenderType);
		if (iRenderType == CD_EFFECT_GRAPH)  // si c'est un graphe, la taille de l'historique depend de la largeur de l'icone, donc on le prend en compte.
			CD_APPLET_SET_MY_DATA_RENDERER_HISTORY_TO_MAX;
		
		CD_APPLET_REFRESH_MY_DATA_RENDERER;  // on rafraichit le dessin.
	}
CD_APPLET_RELOAD_END
