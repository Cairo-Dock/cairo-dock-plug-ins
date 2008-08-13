
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-rame.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

CD_APPLET_GET_CONFIG_BEGIN

	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER ("Configuration", "delay");
	myConfig.bShowSwap =  CD_CONFIG_GET_BOOLEAN ("Configuration", "show swap");
	myConfig.iInfoDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "info display");
	myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
	myConfig.fAlpha = CD_CONFIG_GET_DOUBLE ("Configuration", "filligran alpha");
	if (myConfig.fAlpha != 0)
	{
		myConfig.cFilligranImagePath = CD_CONFIG_GET_FILE_PATH ("Configuration", "filligran image", MY_APPLET_ICON_FILE);
	}
	
	
	myConfig.iNbDisplayedProcesses = CD_CONFIG_GET_INTEGER ("Configuration", "top");
	myConfig.bTopInPercent = CD_CONFIG_GET_BOOLEAN ("Configuration", "top in percent");
	
	myConfig.pTopTextDescription = cairo_dock_duplicate_label_description (&g_dialogTextDescription);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "top color start", myConfig.pTopTextDescription->fColorStart);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "top color stop", myConfig.pTopTextDescription->fColorStop);
	myConfig.pTopTextDescription->bVerticalPattern = TRUE;
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	cairo_dock_free_label_description (myConfig.pTopTextDescription);
	g_free (myConfig.cFilligranImagePath);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN	
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	
	//Adieu la jauge...
	cairo_dock_free_gauge(myData.pGauge);
	
	cairo_dock_free_measure_timer (myData.pTopMeasureTimer);
	cairo_dock_dialog_unreference (myData.pTopDialog);
	cairo_surface_destroy (myData.pTopSurface);
	cd_rame_clean_all_processes ();
	g_free (myData.pTopList);
	g_free (myData.pPreviousTopList);
CD_APPLET_RESET_DATA_END

