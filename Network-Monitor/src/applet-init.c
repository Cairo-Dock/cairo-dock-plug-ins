/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"
#include "applet-config.h"
#include "applet-connections.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("Network-Monitor"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet shows you a monitor of your active connections\n"
	"Left-click to pop-up some info,"
	"Middle-click to re-check immediately."),
	"Yann SLADEK")

static void _set_data_renderer (CairoDockModuleInstance *myApplet, gboolean bReload)
{
	CairoDataRendererAttribute *pRenderAttr = NULL;  // les attributs du data-renderer global.
	if (myConfig.iDisplayType == CD_WIFI_GAUGE)
	{
		CairoGaugeAttribute attr;  // les attributs de la jauge.
		memset (&attr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "gauge";
		attr.cThemePath = myConfig.cGThemePath;
	}
	else if (myConfig.iDisplayType == CD_WIFI_GRAPH)
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
	else if (myConfig.iDisplayType == CD_WIFI_BAR)
	{
		/// A FAIRE...
	}
	if (pRenderAttr != NULL)
	{
		pRenderAttr->iLatencyTime = myConfig.iCheckInterval * 1000 * myConfig.fSmoothFactor;
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
	if (cairo_dock_dbus_detect_system_application("org.freedesktop.NetworkManager"))
	{	
		cd_debug("Network-Monitor : Dbus Service found, using Dbus connection");
		myData.bDbusConnection = TRUE;
		cd_NetworkMonitor_get_active_connection_info();
		cd_NetworkMonitor_draw_icon (); // Dessin initial (ensuite tout passera au travers des signaux)
		cd_NetworkMonitor_connect_signals();
	}
	else
	{
		cd_debug("Network-Monitor : Dbus Service not found, using rough connection");
		myData.bDbusConnection = FALSE;
		// Initialisation de la tache periodique de mesure.
		//myData.iPreviousQuality = -2;  // force le dessin.
		/*myData.pTask = cairo_dock_new_task (myConfig.iCheckInterval,
			(CairoDockGetDataAsyncFunc) cd_NetworkMonitor_get_data,
			(CairoDockUpdateSyncFunc) cd_NetworkMonitor_update_from_data,
			myApplet);
		if (cairo_dock_is_loading ())
			cairo_dock_launch_task_delayed (myData.pTask, 2000);
		else
			cairo_dock_launch_task (myData.pTask);*/
	}
	
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	if (myData.bWirelessExt)
		dbus_g_proxy_disconnect_signal(myData.dbus_proxy_ActiveAccessPoint, "PropertiesChanged",
			G_CALLBACK(onChangeWirelessProperties), NULL);
			
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
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
		
		if (!myData.bDbusConnection)
		{
			myData.iQuality = -2;  // force le redessin.
			myData.iPercent = -2;
			myData.iSignalLevel = -2;
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			cairo_dock_relaunch_task_immediately (myData.pTask, myConfig.iCheckInterval);
		}
		else
			//CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			cd_NetworkMonitor_draw_icon();
	}
	else  // on redessine juste l'icone.
	{
		//CD_APPLET_RELOAD_MY_DATA_RENDERER (NULL);
		if (myConfig.iDisplayType == CD_WIFI_GRAPH)
			CD_APPLET_SET_MY_DATA_RENDERER_HISTORY_TO_MAX;
		
		if (!myData.bDbusConnection)
		{
			myData.iQuality = -2;  // force le redessin.
			if (! cairo_dock_task_is_running (myData.pTask))
			{
				if (myData.bWirelessExt)
					cd_NetworkMonitor_draw_icon ();
				else
					cd_NetworkMonitor_draw_no_wireless_extension ();
			}
		}
		else
		{
			_set_data_renderer (myApplet, TRUE);
			cd_NetworkMonitor_draw_icon ();
		}
	}
CD_APPLET_RELOAD_END
