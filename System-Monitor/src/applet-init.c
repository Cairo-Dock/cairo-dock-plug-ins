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

#include <stdlib.h>

#ifdef HAVE_SENSORS
#include <sensors/sensors.h>
#endif

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-top.h"
#include "applet-sensors.h"
#include "applet-monitor.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("System Monitor"),
	2, 0, 5,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This applet shows you the CPU load, RAM usage, graphic card temperature, etc.\n"
	"Middle click on the icon to get some valuable info.\n"
	"Left click on the icon to get a list of the most resources using programs.\n"
	"You can instanciate this applet several times to show different values each time."),
	"parAdOxxx_ZeRo and Fabounet")


static gboolean _unthreaded_task (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_sysmonitor_get_data (myApplet);
	cd_sysmonitor_update_from_data (myApplet);
	CD_APPLET_LEAVE (TRUE);
	//return TRUE;
}

#define _add_graph_color(x) \
	if (x) {\
		memcpy (&fHighColor[3*i], myConfig.fHigholor, 3*sizeof (double));\
		memcpy (&fLowColor[3*i], myConfig.fLowColor, 3*sizeof (double));\
		i ++; }
static void _set_data_renderer (GldiModuleInstance *myApplet)
{
	if (myConfig.iDisplayType == CD_SYSMONITOR_BAR)
		return; /// TODO

	CairoDataRendererAttribute *pRenderAttr = NULL;  // attributes for the global data-renderer.
	CairoGaugeAttribute aGaugeAttr;  // gauge attributes.
	CairoGraphAttribute aGraphAttr;  // graph attributes.
	double fHighColor[CD_SYSMONITOR_NB_MAX_VALUES*3]; // attributes for the graph
	double fLowColor[CD_SYSMONITOR_NB_MAX_VALUES*3];  // (but have to be declared here to force GCC to keep data)
	int iNbValues = myConfig.bShowCpu + myConfig.bShowRam + myConfig.bShowSwap + myConfig.bShowNvidia + myConfig.bShowCpuTemp + myConfig.bShowFanSpeed;
	if (myConfig.iDisplayType == CD_SYSMONITOR_GAUGE)
	{
		memset (&aGaugeAttr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aGaugeAttr);
		pRenderAttr->cModelName = "gauge";
		pRenderAttr->iRotateTheme = myConfig.iRotateTheme;
		aGaugeAttr.cThemePath = myConfig.cGThemePath;
	}
	else if (myConfig.iDisplayType == CD_SYSMONITOR_GRAPH)
	{
		memset (&aGraphAttr, 0, sizeof (CairoGraphAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aGraphAttr);
		pRenderAttr->cModelName = "graph";
		int w, h;
		CD_APPLET_GET_MY_ICON_EXTENT (&w, &h);
		pRenderAttr->iMemorySize = (w > 1 ? w : 32);  // fWidth peut etre <= 1 en mode desklet au chargement.
		aGraphAttr.iType = myConfig.iGraphType;
		aGraphAttr.bMixGraphs = myConfig.bMixGraph;
		int i;
		for (i = 0; i < iNbValues; i ++)
		{
			memcpy (&fHighColor[3*i], myConfig.fHigholor, 3*sizeof (double));
			memcpy (&fLowColor[3*i], myConfig.fLowColor, 3*sizeof (double));
		}
		aGraphAttr.fHighColor = fHighColor;
		aGraphAttr.fLowColor = fLowColor;
		memcpy (aGraphAttr.fBackGroundColor, myConfig.fBgColor, 4*sizeof (double));
	}

	pRenderAttr->iLatencyTime = myConfig.iCheckInterval * 1000 * myConfig.fSmoothFactor;
	pRenderAttr->iNbValues = iNbValues;
	if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
	{
		pRenderAttr->bWriteValues = TRUE;
		pRenderAttr->format_value = (CairoDataRendererFormatValueFunc)cd_sysmonitor_format_value;
		pRenderAttr->pFormatData = myApplet;
	}
	const gchar *labels[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
	int i = 0;
	if (myConfig.bShowCpu)
		labels[i++] = "CPU";
	if (myConfig.bShowRam)
		labels[i++] = "RAM";
	if (myConfig.bShowSwap)
		labels[i++] = "SWAP";
	if (myConfig.bShowNvidia)
		labels[i++] = "GPU";
	if (myConfig.bShowCpuTemp)
		labels[i++] = "TEMP";
	if (myConfig.bShowFanSpeed)
		labels[i++] = "FAN";
	pRenderAttr->cLabels = (gchar **)labels;
	CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON (pRenderAttr);
}

CD_APPLET_INIT_BEGIN
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
	}
	
	// Initialisation du rendu.
	_set_data_renderer (myApplet);
	
	// Initialisation de la tache periodique de mesure.
	myData.pClock = g_timer_new ();
	if (myConfig.bShowNvidia || (myConfig.bShowCpu && myConfig.bShowRam))
		myData.pPeriodicTask = gldi_task_new (myConfig.iCheckInterval,
			(GldiGetDataAsyncFunc) cd_sysmonitor_get_data,
			(GldiUpdateSyncFunc) cd_sysmonitor_update_from_data,
			myApplet);
	else
		myData.pPeriodicTask = gldi_task_new (myConfig.iCheckInterval,
			(GldiGetDataAsyncFunc) NULL,
			(GldiUpdateSyncFunc) _unthreaded_task,
			myApplet);
	myData.bAcquisitionOK = TRUE;
	gldi_task_launch_delayed (myData.pPeriodicTask, 0.);  // launch in idle.
	
	// On gere l'appli "moniteur systeme".
	if (myConfig.cSystemMonitorClass)
		CD_APPLET_MANAGE_APPLICATION (myConfig.cSystemMonitorClass);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;

	gchar *cNull = NULL;
	CD_APPLET_MANAGE_APPLICATION (cNull);
	
#ifdef HAVE_SENSORS
	cd_sysmonitor_clean_sensors ();
#endif

CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
			CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
		}
		
		_set_data_renderer (myApplet);
		
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_LABEL)
		{
			if (myConfig.defaultTitle) // has another default name
				CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
			else
				CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
		}
		
		myData.bAcquisitionOK = TRUE;
		myData.fPrevCpuPercent = 0;
		myData.fPrevRamPercent = 0;
		myData.fPrevSwapPercent = 0;
		myData.fPrevGpuTempPercent = 0;
		myData.fPrevCpuTempPercent = 0;
		myData.fPrevFanSpeedPercent = 0;
		myData.iTimerCount = 0;
		gldi_task_change_frequency_and_relaunch (myData.pPeriodicTask, myConfig.iCheckInterval);
		
		CD_APPLET_MANAGE_APPLICATION (myConfig.cSystemMonitorClass);
	}
	else {  // on redessine juste l'icone.
		///CD_APPLET_RELOAD_MY_DATA_RENDERER (NULL);
		if (myConfig.iDisplayType == CD_SYSMONITOR_GRAPH)
			CD_APPLET_SET_MY_DATA_RENDERER_HISTORY_TO_MAX;
		
		/**CairoDockLabelDescription *pOldLabelDescription = myConfig.pTopTextDescription;
		myConfig.pTopTextDescription = cairo_dock_duplicate_label_description (&myDialogsParam.dialogTextDescription);
		memcpy (myConfig.pTopTextDescription->fColorStart, pOldLabelDescription->fColorStart, 3*sizeof (double));
		memcpy (myConfig.pTopTextDescription->fColorStop, pOldLabelDescription->fColorStop, 3*sizeof (double));
		myConfig.pTopTextDescription->bVerticalPattern = TRUE;
		cairo_dock_free_label_description (pOldLabelDescription);
		
		if (! gldi_task_is_running (myData.pPeriodicTask))
		{
			myData.fPrevCpuPercent = 0;
			myData.fPrevRamPercent = 0;
			myData.fPrevSwapPercent = 0;
			myData.fPrevGpuTempPercent = 0;
			cd_sysmonitor_update_from_data (myApplet);
		}*/
	}
CD_APPLET_RELOAD_END
