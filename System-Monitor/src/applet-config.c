
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

#define CD_APPLET_REMOVE_MY_DATA_RENDERER cairo_dock_remove_data_renderer_on_icon (myIcon)

CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER ("Configuration", "delay");
	myConfig.fSmoothFactor = CD_CONFIG_GET_DOUBLE ("Configuration", "smooth");
	
	myConfig.bShowCpu = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "show cpu", TRUE);
	myConfig.bShowRam = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "show ram", TRUE);
	myConfig.bShowSwap = CD_CONFIG_GET_BOOLEAN ("Configuration", "show swap");
	myConfig.bShowNvidia = CD_CONFIG_GET_BOOLEAN ("Configuration", "show nvidia");
	myConfig.bShowFreeMemory = CD_CONFIG_GET_BOOLEAN ("Configuration", "show free");
	
	myConfig.iInfoDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "info display");
	myConfig.iDisplayType = CD_CONFIG_GET_INTEGER ("Configuration", "renderer");
	
	myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
	
	myConfig.iGraphType = CD_CONFIG_GET_INTEGER ("Configuration", "graphic type");
	myConfig.bMixGraph = CD_CONFIG_GET_BOOLEAN ("Configuration", "mix graph");
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
	g_free (myDialogs.dialogTextDescription.cFont);
	myDialogs.dialogTextDescription.cFont = g_strdup ("Mono");  // on prend une police a chasse fixe.
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "top color start", myConfig.pTopTextDescription->fColorStart);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "top color stop", myConfig.pTopTextDescription->fColorStop);
	myConfig.pTopTextDescription->bVerticalPattern = TRUE;
	myConfig.bTopInPercent = CD_CONFIG_GET_BOOLEAN ("Configuration", "top in percent");
	
	myConfig.cSystemMonitorCommand = CD_CONFIG_GET_STRING ("Configuration", "sys monitor");
	myConfig.bStealTaskBarIcon = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "inhibate appli", TRUE);
	if (myConfig.bStealTaskBarIcon)
	{
		myConfig.cSystemMonitorClass = CD_CONFIG_GET_STRING ("Configuration", "sys monitor class");
		if (myConfig.cSystemMonitorClass == NULL)
		{
			if (myConfig.cSystemMonitorCommand != NULL)
				myConfig.cSystemMonitorClass = g_strdup (myConfig.cSystemMonitorCommand);  // couper au 1er espace ...
			else if (g_iDesktopEnv == CAIRO_DOCK_GNOME)
				myConfig.cSystemMonitorClass = g_strdup ("gnome-system-monitor");
			else if (g_iDesktopEnv == CAIRO_DOCK_XFCE)
				myConfig.cSystemMonitorClass = g_strdup ("xfce-system-monitor");
			else if (g_iDesktopEnv == CAIRO_DOCK_KDE)
				myConfig.cSystemMonitorClass = g_strdup ("kde-system-monitor");
		}
	}
	myConfig.fUserHZ = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "HZ", 100);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	cairo_dock_free_label_description (myConfig.pTopTextDescription);
	g_free (myConfig.cWatermarkImagePath);
	g_free (myConfig.cSystemMonitorCommand);
	g_free (myConfig.cSystemMonitorClass);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	g_timer_destroy (myData.pClock);
	
	CD_APPLET_REMOVE_MY_DATA_RENDERER;
	
	cairo_dock_free_measure_timer (myData.pTopMeasureTimer);
	if (myData.pTopClock != NULL)
		g_timer_destroy (myData.pTopClock);
	g_free (myData.pTopList);
	if (myData.pProcessTable != NULL)
		g_hash_table_destroy (myData.pProcessTable);
	cairo_surface_destroy (myData.pTopSurface);
CD_APPLET_RESET_DATA_END

