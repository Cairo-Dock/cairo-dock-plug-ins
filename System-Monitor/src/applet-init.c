#include "stdlib.h"

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
	"Left click on the icon to get a list of the most ressources using programs."),
	"parAdOxxx_ZeRo & Fabounet")

#define CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON(pAttr) cairo_dock_add_new_data_renderer_on_icon (myIcon, myContainer, myDrawContext, pAttr)
#define CD_APPLET_RELOAD_MY_DATA_RENDERER(pAttr) cairo_dock_reload_data_renderer_on_icon (myIcon, myContainer, myDrawContext, pAttr)
#define CAIRO_DATA_RENDERER_ATTRIBUTE(pAttr) ((CairoDataRendererAttribute *) pAttr)

static gboolean _unthreaded_measure (CairoDockModuleInstance *myApplet)
{
	cd_sysmonitor_get_data (myApplet);
	cd_sysmonitor_update_from_data (myApplet);
	return TRUE;
}

static void _set_data_renderer (CairoDockModuleInstance *myApplet, gboolean bReload)
{
	CairoDataRendererAttribute *pRenderAttr;  // les attributs du data-renderer global.
	int iNbValues = myConfig.bShowCpu + myConfig.bShowRam + myConfig.bShowSwap + myConfig.bShowNvidia;
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
		CairoGraph2Attribute attr;  // les attributs du graphe.
		memset (&attr, 0, sizeof (CairoGraph2Attribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "graph";
		pRenderAttr->iMemorySize = 20;
		attr.iType = myConfig.iGraphType;
		attr.iRadius = 5;
		attr.bMixGraphs = myConfig.iGraphType;
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
		
	}
	if (pRenderAttr != NULL)
	{
		pRenderAttr->iLatencyTime = myConfig.iCheckInterval * 1000 * myConfig.fSmoothFactor;
		pRenderAttr->iNbValues = iNbValues;
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
	
	// Initialisation du timer de mesure.
	myData.pClock = g_timer_new ();
	myData.pMeasureTimer = cairo_dock_new_measure_timer (myConfig.iCheckInterval,
		NULL,
		NULL,  // cd_cpusage_read_data
		(CairoDockUpdateTimerFunc) _unthreaded_measure,  // cd_cpusage_update_from_data
		myApplet);
	myData.bAcquisitionOK = TRUE;
	cairo_dock_launch_measure (myData.pMeasureTimer);
	
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
	
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		
		cd_sysmonitor_stop_top_dialog (myApplet);

		_set_data_renderer (myApplet, TRUE);
		
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_LABEL)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		}
		
		myData.bAcquisitionOK = TRUE;
		cairo_dock_relaunch_measure_immediately (myData.pMeasureTimer, myConfig.iCheckInterval);
		
		g_free (myData.pTopList);
		myData.pTopList = NULL;
		if (myData.pTopMeasureTimer != NULL)
			cairo_dock_change_measure_frequency (myData.pTopMeasureTimer, myConfig.iProcessCheckInterval);
		
		if (myConfig.cSystemMonitorClass)
			CD_APPLET_MANAGE_APPLICATION (myConfig.cSystemMonitorClass, myConfig.bStealTaskBarIcon);
	}
	else {  // on redessine juste l'icone.
		CD_APPLET_RELOAD_MY_DATA_RENDERER (NULL);
		
		CairoDockLabelDescription *pOldLabelDescription = myConfig.pTopTextDescription;
		myConfig.pTopTextDescription = cairo_dock_duplicate_label_description (&myDialogs.dialogTextDescription);
		memcpy (myConfig.pTopTextDescription->fColorStart, pOldLabelDescription->fColorStart, 3*sizeof (double));
		memcpy (myConfig.pTopTextDescription->fColorStop, pOldLabelDescription->fColorStop, 3*sizeof (double));
		myConfig.pTopTextDescription->bVerticalPattern = TRUE;
		cairo_dock_free_label_description (pOldLabelDescription);
		
		if (! cairo_dock_measure_is_running (myData.pMeasureTimer))
			cd_sysmonitor_update_from_data (myApplet);
	}
CD_APPLET_RELOAD_END
