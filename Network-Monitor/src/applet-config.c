/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/


#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "delay", 10);
	myConfig.fSmoothFactor = CD_CONFIG_GET_DOUBLE ("Configuration", "smooth");
	
	GString *sKeyName = g_string_new ("");
	int i;
	for (i = 0; i < CONNECTION_NB_QUALITY; i ++) {
		g_string_printf (sKeyName, "icon_%d", i);
		myConfig.cUserImage[i] = CD_CONFIG_GET_STRING ("Configuration", sKeyName->str);
	}
	g_string_free (sKeyName, TRUE);
	myConfig.cUserCommand = CD_CONFIG_GET_STRING ("Configuration", "command");
	
	myConfig.quickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "signal_type", 1);
	
	myConfig.iEffect = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "effect", 0);
	myConfig.iDisplayType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "renderer", 0);
	
	myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
	
	//myConfig.fAlpha = CD_CONFIG_GET_DOUBLE ("Configuration", "watermark alpha");
	//if (myConfig.fAlpha != 0)
	//	myConfig.cWatermarkImagePath = CD_CONFIG_GET_FILE_PATH ("Configuration", "watermark image", MY_APPLET_ICON_FILE);
	myConfig.bESSID	= CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "essid", TRUE);
	
	myConfig.iGraphType = CD_CONFIG_GET_INTEGER ("Configuration", "graphic type");
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "low color", myConfig.fLowColor);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "high color", myConfig.fHigholor);
	CD_CONFIG_GET_COLOR ("Configuration", "bg color", myConfig.fBgColor);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cGThemePath);
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cUserCommand);
	
	int i;
	for (i = 0; i < CONNECTION_NB_QUALITY; i ++)
		g_free (myConfig.cUserImage[i]);
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_task (myData.pTask);
	
	CD_APPLET_REMOVE_MY_DATA_RENDERER;
	
	int i;
	for (i = 0; i < CONNECTION_NB_QUALITY; i ++)
		cairo_surface_destroy (myData.pSurfaces[i]);
	
	g_free (myData.cESSID);
	g_free (myData.cInterface);
	g_free (myData.cAccessPoint);
	
CD_APPLET_RESET_DATA_END
