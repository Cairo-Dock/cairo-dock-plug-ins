
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER ("Configuration", "delay");
	
	myConfig.iInfoDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "info display");
	myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
	
	myConfig.bUseGraphic = CD_CONFIG_GET_BOOLEAN ("Configuration", "use graphic");
	myConfig.iGraphType = CD_CONFIG_GET_INTEGER ("Configuration", "graphic type");
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "low color", myConfig.fLowColor);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "high color", myConfig.fHigholor);
	CD_CONFIG_GET_COLOR ("Configuration", "bg color", myConfig.fBgColor);
	
	myConfig.fAlpha = CD_CONFIG_GET_DOUBLE ("Configuration", "watermark alpha");
	if (myConfig.fAlpha != 0)
	{
		myConfig.cWatermarkImagePath = CD_CONFIG_GET_FILE_PATH ("Configuration", "watermark image", MY_APPLET_ICON_FILE);
	}
	
	myConfig.iNbDisplayedProcesses = CD_CONFIG_GET_INTEGER ("Configuration", "top");
	myConfig.iProcessCheckInterval = CD_CONFIG_GET_INTEGER ("Configuration", "top delay");
	
	myConfig.pTopTextDescription = cairo_dock_duplicate_label_description (&myDialogs.dialogTextDescription);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "top color start", myConfig.pTopTextDescription->fColorStart);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "top color stop", myConfig.pTopTextDescription->fColorStop);
	myConfig.pTopTextDescription->bVerticalPattern = TRUE;
	
	myConfig.cSystemMonitorCommand = CD_CONFIG_GET_STRING ("Configuration", "sys monitor");
	myConfig.fUserHZ = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "HZ", 100);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	cairo_dock_free_label_description (myConfig.pTopTextDescription);
	g_free (myConfig.cWatermarkImagePath);
	g_free (myConfig.cSystemMonitorCommand);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	g_timer_destroy (myData.pClock);
	
	//cairo_dock_free_gauge (myData.pGauge);
	//cairo_dock_free_graph (myData.pGraph);
	
	cairo_dock_free_measure_timer (myData.pTopMeasureTimer);
	if (myData.pTopClock != NULL)
		g_timer_destroy (myData.pTopClock);
	g_free (myData.pTopList);
	if (myData.pProcessTable != NULL)
		g_hash_table_destroy (myData.pProcessTable);
	cairo_surface_destroy (myData.pTopSurface);
CD_APPLET_RESET_DATA_END

