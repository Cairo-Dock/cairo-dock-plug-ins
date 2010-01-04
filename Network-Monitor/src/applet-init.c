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


CD_APPLET_PRE_INIT_BEGIN (N_("Network-Monitor"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet allows you to monitor your network connection(s).\n"
	"It can display the download/upload speeds and the wifi signal quality.\n"
	"If you have network-manager running, it can also let you choose the current wifi network.\n"
	"Left-click to pop-up some info,"
	"Scroll on the icon to switch the display between net speed and wifi."),
	"Yann Sladek (Mav), Remy Robertson (ChanGFu), and Fabrice Rey (Fabounet)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	pInterface->load_custom_widget = cd_netmonitor_load_custom_widget;
CD_APPLET_PRE_INIT_END


static void _set_data_renderer (CairoDockModuleInstance *myApplet, gboolean bReload)
{
	CairoDataRendererAttribute *pRenderAttr = NULL;  // les attributs du data-renderer global.
	if (myConfig.iRenderType == CD_EFFECT_GAUGE)
	{
		CairoGaugeAttribute attr;  // les attributs de la jauge.
		memset (&attr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "gauge";
		attr.cThemePath = myConfig.cGThemePath;
	}
	else if (myConfig.iRenderType == CD_EFFECT_GRAPH)
	{
		CairoGraphAttribute attr;  // les attributs du graphe.
		memset (&attr, 0, sizeof (CairoGraphAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "graph";
		pRenderAttr->iMemorySize = (myIcon->fWidth > 1 ? myIcon->fWidth : 32);  // fWidht peut etre <= 1 en mode desklet au chargement.
		attr.iType = myConfig.iGraphType;
		attr.iRadius = 10;
		attr.fHighColor = myConfig.fHigholor;
		attr.fLowColor = myConfig.fLowColor;
		memcpy (attr.fBackGroundColor, myConfig.fBgColor, 4*sizeof (double));
	}
	else if (myConfig.iRenderType == CD_EFFECT_ICON)
	{
		/// A FAIRE...
	}
	if (pRenderAttr != NULL)
	{
		pRenderAttr->iLatencyTime = MIN (myConfig.iWifiCheckInterval, myConfig.iNetspeedCheckInterval) * 1000 * myConfig.fSmoothFactor;
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
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	
	// Initialisation du rendu.
	_set_data_renderer (myApplet, FALSE);
	myData.iPreviousQuality = -1;  // force le dessin.
	
	myData.bDbusConnection = cd_NetworkMonitor_connect_to_bus ();
	if (myData.bDbusConnection)
	{
		cd_debug("Network-Monitor : Dbus Service found, using Dbus connection");
		if (! cd_NetworkMonitor_get_active_connection_info())  // si aucune connexion courante, on parcourt la liste des devices connus, et on choisit dedans.
		{
			cd_NetworkMonitor_get_device ();
		}
		cd_NetworkMonitor_draw_icon ();
	}
	if (myData.cDevice == NULL)  // on passe a la methode manuelle
	{
		cd_debug("Network-Monitor : Dbus service or device not found, using rough connection");
		// Initialisation de la tache periodique de mesure.
		
		if (myConfig.bModeWifi)
			cd_netmonitor_launch_wifi_task (myApplet);
		else
			cd_netmonitor_launch_netspeed_task (myApplet);
	}
	
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet != NULL)
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	
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
		_set_data_renderer (myApplet, TRUE);
		
		myData.iPreviousQuality = -1;  // force le redessin.
		if (!myData.bDbusConnection)
		{
			myData.iPercent = -1;
			myData.iSignalLevel = -1;
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			if (myConfig.bModeWifi)
				cd_netmonitor_launch_wifi_task (myApplet);
			else
				cd_netmonitor_launch_netspeed_task (myApplet);
		}
		else
			//CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			cd_NetworkMonitor_draw_icon();
	}
	else  // on redessine juste l'icone.
	{
		CD_APPLET_RELOAD_MY_DATA_RENDERER (NULL);  // on le recharge aux nouvelles dimensions de l'icone.
		if (myConfig.iRenderType == CD_EFFECT_GRAPH)  // si c'est un graphe, la taille de l'historique depend de la largeur de l'icone, donc on le prend en compte.
			CD_APPLET_SET_MY_DATA_RENDERER_HISTORY_TO_MAX;
		
		CD_APPLET_REFRESH_MY_DATA_RENDERER;  // on rafraichit le dessin.
	}
CD_APPLET_RELOAD_END
