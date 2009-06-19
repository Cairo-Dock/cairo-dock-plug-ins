#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-netspeed.h"


CD_APPLET_DEFINITION ("netspeed",
	1, 6, 2,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet shows you the bit rate of your internet connection and some stats about it.\n"
	"Left-click on the icon to have the total amount of data transfered\n"
	"Middle-click to (de)activate the network (needs NetworManager)"),
	"parAdOxxx_ZeRo");


CD_APPLET_INIT_BEGIN
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	//Initialisation de la jauge
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	if (myConfig.bUseGraphic)
	{
		myData.pGraph = cairo_dock_create_graph (myDrawContext,
			20, myConfig.iGraphType | CAIRO_DOCK_DOUBLE_GRAPH | (myConfig.bMixGraph ? CAIRO_DOCK_MIX_DOUBLE_GRAPH : 0),
			myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale,
			myConfig.fLowColor, myConfig.fHigholor, myConfig.fBgColor, myConfig.fLowColor2, myConfig.fHigholor2);
		if (myConfig.cWatermarkImagePath != NULL)
			cairo_dock_add_watermark_on_graph (myDrawContext, myData.pGraph, myConfig.cWatermarkImagePath, myConfig.fAlpha);
		CD_APPLET_RENDER_GRAPH (myData.pGraph);
	}
	else
	{
		myData.pGauge = cairo_dock_load_gauge(myDrawContext,myConfig.cGThemePath,myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
		if (myConfig.cWatermarkImagePath != NULL)
			cairo_dock_add_watermark_on_gauge (myDrawContext, myData.pGauge, myConfig.cWatermarkImagePath, myConfig.fAlpha);
		CD_APPLET_RENDER_GAUGE (myData.pGauge, 0.);
	}
	//Initialisation du timer de mesure.
	myData.pClock = g_timer_new ();
	myData.pTask = cairo_dock_new_task (myConfig.iCheckInterval,
		(CairoDockGetDataAsyncFunc) cd_netspeed_read_data,
		(CairoDockUpdateSyncFunc) cd_netspeed_update_from_data,
		myApplet);
	myData.bAcquisitionOK = TRUE;
	cairo_dock_launch_task (myData.pTask);
	
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
	if (myDesklet != NULL) {
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		cairo_dock_free_gauge(myData.pGauge);
		cairo_dock_free_graph (myData.pGraph);
		if (myConfig.bUseGraphic)
		{
			myData.pGauge = NULL;
			myData.pGraph = cairo_dock_create_graph (myDrawContext,
				20, myConfig.iGraphType | CAIRO_DOCK_DOUBLE_GRAPH | (myConfig.bMixGraph ? CAIRO_DOCK_MIX_DOUBLE_GRAPH : 0),
				myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale,
				myConfig.fLowColor, myConfig.fHigholor, myConfig.fBgColor, myConfig.fLowColor2, myConfig.fHigholor2);
			if (myConfig.cWatermarkImagePath != NULL)
				cairo_dock_add_watermark_on_graph (myDrawContext, myData.pGraph, myConfig.cWatermarkImagePath, myConfig.fAlpha);
		}
		else
		{
			myData.pGraph = NULL;
			myData.pGauge = cairo_dock_load_gauge(myDrawContext,
				myConfig.cGThemePath,
				myIcon->fWidth * fMaxScale,myIcon->fHeight * fMaxScale);
			if (myConfig.cWatermarkImagePath != NULL)
				cairo_dock_add_watermark_on_gauge (myDrawContext, myData.pGauge, myConfig.cWatermarkImagePath, myConfig.fAlpha);
		}
		
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_ICON)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		}
		if (myConfig.iInfoDisplay != CAIRO_DOCK_INFO_ON_LABEL)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
		}
		
		cairo_dock_relaunch_task_immediately (myData.pTask, myConfig.iCheckInterval);
	}
	else {  // on redessine juste l'icone.
		if (myData.pGauge != NULL)
			cairo_dock_reload_gauge (myDrawContext, myData.pGauge, myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
		else if (myData.pGraph != NULL)
			cairo_dock_reload_graph (myDrawContext, myData.pGraph, myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
		else if (myConfig.bUseGraphic)
			myData.pGraph = cairo_dock_create_graph (myDrawContext,
				20, myConfig.iGraphType | CAIRO_DOCK_DOUBLE_GRAPH | (myConfig.bMixGraph ? CAIRO_DOCK_MIX_DOUBLE_GRAPH : 0),
				myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale,
				myConfig.fLowColor, myConfig.fHigholor, myConfig.fBgColor, myConfig.fLowColor2, myConfig.fHigholor2);
		else
			myData.pGauge = cairo_dock_load_gauge(myDrawContext,
				myConfig.cGThemePath,
				myIcon->fWidth * fMaxScale, myIcon->fHeight * fMaxScale);
		if (myConfig.cWatermarkImagePath != NULL)
		{
			if (myData.pGauge != NULL)
				cairo_dock_add_watermark_on_gauge (myDrawContext, myData.pGauge, myConfig.cWatermarkImagePath, myConfig.fAlpha);
			else
				cairo_dock_add_watermark_on_graph (myDrawContext, myData.pGraph, myConfig.cWatermarkImagePath, myConfig.fAlpha);
		}
		
		cd_netspeed_update_from_data (myApplet);
	}
CD_APPLET_RELOAD_END
