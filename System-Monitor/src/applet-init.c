#include <stdlib.h>

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-top.h"
#include "applet-monitor.h"


CD_APPLET_DEFINITION (N_("System Monitor"),
	2, 0, 5,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet shows you the CPU load, RAM usage, graphic card temperature, etc.\n"
	"Middle click on the icon to get some valuable info.\n"
	"Left click on the icon to get a list of the most ressources using programs.\n"
	"You can instanciate this applet several times to show different values each time."),
	"parAdOxxx_ZeRo & Fabounet")


static gboolean _unthreaded_task (CairoDockModuleInstance *myApplet)
{
	cd_sysmonitor_get_data (myApplet);
	cd_sysmonitor_update_from_data (myApplet);
	return TRUE;
}

static void _set_data_renderer (CairoDockModuleInstance *myApplet, gboolean bReload)
{
	CairoDataRendererAttribute *pRenderAttr;  // les attributs du data-renderer global.
	if (myConfig.iDisplayType == CD_SYSMONITOR_GAUGE)
	{
		CairoGaugeAttribute attr;  // les attributs de la jauge.
		memset (&attr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "gauge";
		attr.cThemePath = myConfig.cGThemePath;
	}
	else if (myConfig.iDisplayType == CD_SYSMONITOR_GRAPH)
	{
		CairoGraphAttribute attr;  // les attributs du graphe.
		memset (&attr, 0, sizeof (CairoGraphAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "graph";
		pRenderAttr->iMemorySize = (myIcon->fWidth > 1 ? myIcon->fWidth : 32);  // fWidht peut etre <= 1 en mode desklet au chargement.
		g_print ("pRenderAttr->iMemorySize : %d\n", pRenderAttr->iMemorySize);
		attr.iType = myConfig.iGraphType;
		attr.iRadius = 10;
		attr.bMixGraphs = myConfig.bMixGraph;
		double fHighColor[CD_SYSMONITOR_NB_MAX_VALUES*3];
		double fLowColor[CD_SYSMONITOR_NB_MAX_VALUES*3];
		int i = 0;
		if (myConfig.bShowCpu)
		{
			memcpy (&fHighColor[3*i], myConfig.fHigholor, 3*sizeof (double));
			memcpy (&fLowColor[3*i], myConfig.fLowColor, 3*sizeof (double));
			i ++;
		}
		if (myConfig.bShowRam)
		{
			memcpy (&fHighColor[3*i], myConfig.fHigholor, 3*sizeof (double));
			memcpy (&fLowColor[3*i], myConfig.fLowColor, 3*sizeof (double));
			i ++;
		}
		if (myConfig.bShowSwap)
		{
			memcpy (&fHighColor[3*i], myConfig.fHigholor, 3*sizeof (double));
			memcpy (&fLowColor[3*i], myConfig.fLowColor, 3*sizeof (double));
			i ++;
		}
		if (myConfig.bShowNvidia)
		{
			memcpy (&fHighColor[3*i], myConfig.fHigholor, 3*sizeof (double));
			memcpy (&fLowColor[3*i], myConfig.fLowColor, 3*sizeof (double));
			i ++;
		}
		attr.fHighColor = fHighColor;
		attr.fLowColor = fLowColor;
		memcpy (attr.fBackGroundColor, myConfig.fBgColor, 4*sizeof (double));
	}
	else if (myConfig.iDisplayType == CD_SYSMONITOR_BAR)
	{
		/// A FAIRE...
	}
	if (pRenderAttr != NULL)
	{
		pRenderAttr->iLatencyTime = myConfig.iCheckInterval * 1000 * myConfig.fSmoothFactor;
		pRenderAttr->iNbValues = myConfig.bShowCpu + myConfig.bShowRam + myConfig.bShowSwap + myConfig.bShowNvidia;
		//pRenderAttr->bWriteValues = TRUE;
		if (! bReload)
			CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON (pRenderAttr);
		else
			CD_APPLET_RELOAD_MY_DATA_RENDERER (pRenderAttr);
	}
}

CD_APPLET_INIT_BEGIN
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	// Initialisation du rendu.
	_set_data_renderer (myApplet, FALSE);
	
	// Initialisation de la tache periodique de mesure.
	myData.pClock = g_timer_new ();
	myData.pPeriodicTask = cairo_dock_new_task (myConfig.iCheckInterval,
		(CairoDockGetDataAsyncFunc) NULL,  // cd_sysmonitor_get_data
		(CairoDockUpdateSyncFunc) _unthreaded_task,  // cd_sysmonitor_update_from_data
		myApplet);
	myData.bAcquisitionOK = TRUE;
	cairo_dock_launch_task (myData.pPeriodicTask);
	
	// On gere l'appli "moniteur systeme".
	if (myConfig.cSystemMonitorClass)
		CD_APPLET_MANAGE_APPLICATION (myConfig.cSystemMonitorClass, myConfig.bStealTaskBarIcon);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	if (myConfig.cSystemMonitorClass)
		CD_APPLET_MANAGE_APPLICATION (myConfig.cSystemMonitorClass, FALSE);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		
		cd_sysmonitor_stop_top_dialog (myApplet);
		
		_set_data_renderer (myApplet, TRUE);
		
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_LABEL)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		}
		
		myData.bAcquisitionOK = TRUE;
		myData.fPrevCpuPercent = 0;
		myData.fPrevRamPercent = 0;
		myData.fPrevSwapPercent = 0;
		myData.fPrevGpuTempPercent = 0;
		cairo_dock_relaunch_task_immediately (myData.pPeriodicTask, myConfig.iCheckInterval);
		
		g_free (myData.pTopList);
		myData.pTopList = NULL;
		if (myData.pTopTask != NULL)
			cairo_dock_change_task_frequency (myData.pTopTask, myConfig.iProcessCheckInterval);
		
		if (myConfig.cSystemMonitorClass)
			CD_APPLET_MANAGE_APPLICATION (myConfig.cSystemMonitorClass, myConfig.bStealTaskBarIcon);
	}
	else {  // on redessine juste l'icone.
		CD_APPLET_RELOAD_MY_DATA_RENDERER (NULL);
		if (myConfig.iDisplayType == CD_SYSMONITOR_GRAPH)
			CD_APPLET_SET_MY_DATA_RENDERER_HISTORY_TO_MAX;
		
		CairoDockLabelDescription *pOldLabelDescription = myConfig.pTopTextDescription;
		myConfig.pTopTextDescription = cairo_dock_duplicate_label_description (&myDialogs.dialogTextDescription);
		memcpy (myConfig.pTopTextDescription->fColorStart, pOldLabelDescription->fColorStart, 3*sizeof (double));
		memcpy (myConfig.pTopTextDescription->fColorStop, pOldLabelDescription->fColorStop, 3*sizeof (double));
		myConfig.pTopTextDescription->bVerticalPattern = TRUE;
		cairo_dock_free_label_description (pOldLabelDescription);
		
		if (! cairo_dock_task_is_running (myData.pPeriodicTask))
		{
			myData.fPrevCpuPercent = 0;
			myData.fPrevRamPercent = 0;
			myData.fPrevSwapPercent = 0;
			myData.fPrevGpuTempPercent = 0;
			cd_sysmonitor_update_from_data (myApplet);
		}
	}
CD_APPLET_RELOAD_END
