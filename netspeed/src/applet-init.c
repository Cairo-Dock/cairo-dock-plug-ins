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
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-netspeed.h"


CD_APPLET_DEFINITION (N_("netspeed"),
	2, 0, 5,
	CAIRO_DOCK_CATEGORY_APPLET_INTERNET,
	N_("This applet shows you the bit rate of your internet connection and some stats about it.\n"
	"Left-click on the icon to have the total amount of data transferred\n"
	"Middle-click to (de)activate the network (needs NetworManager)"),
	"parAdOxxx_ZeRo");

static void _set_data_renderer (GldiModuleInstance *myApplet, gboolean bReload)
{
	if (myConfig.iDisplayType == CD_NETSPEED_BAR)
		return; /// TODO

	CairoDataRendererAttribute *pRenderAttr = NULL;  // attributes for the global data-renderer.
	CairoGaugeAttribute aGaugeAttr;  // gauge attributes.
	CairoGraphAttribute aGraphAttr;  // graph attributes.
	double fHighColor[CD_NETSPEED_NB_MAX_VALUES*3]; // attributes for the graph
	double fLowColor[CD_NETSPEED_NB_MAX_VALUES*3];  // (but have to be declared here to force GCC to keep data)
	if (myConfig.iDisplayType == CD_NETSPEED_GAUGE)
	{
		memset (&aGaugeAttr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aGaugeAttr);
		pRenderAttr->cModelName = "gauge";
		pRenderAttr->iRotateTheme = myConfig.iRotateTheme;
		aGaugeAttr.cThemePath = myConfig.cGThemePath;
	}
	else if (myConfig.iDisplayType == CD_NETSPEED_GRAPH)
	{
		memset (&aGraphAttr, 0, sizeof (CairoGraphAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aGraphAttr);
		pRenderAttr->cModelName = "graph";
		int w, h;
		CD_APPLET_GET_MY_ICON_EXTENT (&w, &h);
		pRenderAttr->iMemorySize = (w > 1 ? w : 32);
		aGraphAttr.iType = myConfig.iGraphType;
		aGraphAttr.bMixGraphs = myConfig.bMixGraph;
		int i = 0;
		memcpy (&fHighColor[3*i], myConfig.fHigholor, 3*sizeof (double));
		memcpy (&fLowColor[3*i], myConfig.fLowColor, 3*sizeof (double));
		i ++;
		memcpy (&fHighColor[3*i], myConfig.fHigholor, 3*sizeof (double));
		memcpy (&fLowColor[3*i], myConfig.fLowColor, 3*sizeof (double));
		aGraphAttr.fHighColor = fHighColor;
		aGraphAttr.fLowColor = fLowColor;
		memcpy (aGraphAttr.fBackGroundColor, myConfig.fBgColor, 4*sizeof (double));
	}

	pRenderAttr->iLatencyTime = myConfig.iCheckInterval * 1000 * myConfig.fSmoothFactor;
	pRenderAttr->iNbValues = 2;
	pRenderAttr->bUpdateMinMax = TRUE;
	if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
	{
		pRenderAttr->bWriteValues = TRUE;
		pRenderAttr->format_value = (CairoDataRendererFormatValueFunc)cd_netspeed_format_value;
		pRenderAttr->pFormatData = myApplet;
	}
	const gchar *labels[2] = {"DOWN", "UP"};
	pRenderAttr->cLabels = (gchar **)labels;
	CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON (pRenderAttr);
}

CD_APPLET_INIT_BEGIN
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
	}
	
	// Initialisation du rendu.
	_set_data_renderer (myApplet, FALSE);
	
	// Initialisation de la tache periodique de mesure.
	myData.pClock = g_timer_new ();
	myData.pPeriodicTask = gldi_task_new (myConfig.iCheckInterval,
		(GldiGetDataAsyncFunc) cd_netspeed_get_data,
		(GldiUpdateSyncFunc) cd_netspeed_update_from_data,
		myApplet);
	myData.bAcquisitionOK = TRUE;
	gldi_task_launch (myData.pPeriodicTask);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
			CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
		}
		
		_set_data_renderer (myApplet, TRUE);
		
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_ICON)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		}
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_LABEL)
		{
			if (myConfig.defaultTitle) // has another default name
				CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
			else
				CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
		}
		
		gldi_task_change_frequency_and_relaunch (myData.pPeriodicTask, myConfig.iCheckInterval);
	}
	else {  // on redessine juste l'icone.
		//CD_APPLET_RELOAD_MY_DATA_RENDERER (NULL);
		if (myConfig.iDisplayType == CD_NETSPEED_GRAPH)
			CD_APPLET_SET_MY_DATA_RENDERER_HISTORY_TO_MAX;
		
		/**if (! gldi_task_is_running (myData.pPeriodicTask))
			cd_netspeed_update_from_data (myApplet);*/
	}
CD_APPLET_RELOAD_END
