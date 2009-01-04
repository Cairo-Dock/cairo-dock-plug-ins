/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

CD_APPLET_GET_CONFIG_BEGIN
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.cDevice = CD_CONFIG_GET_STRING ("Configuration", "device");
	myConfig.cDefaultName = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "name_default", myConfig.cDevice);
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER ("Configuration", "interval");
	myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
	myConfig.fAlpha = CD_CONFIG_GET_DOUBLE ("Configuration", "watermark alpha");
	myConfig.iInfoDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "info display");
	myConfig.iPercentDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "percentage");
	if (myConfig.fAlpha != 0)
	{
		myConfig.cWatermarkImagePath = CD_CONFIG_GET_FILE_PATH ("Configuration", "watermark image", MY_APPLET_ICON_FILE);
	}
	myConfig.pTopTextDescription = cairo_dock_duplicate_label_description (&myDialogs.dialogTextDescription);
	myConfig.pTopTextDescription->bVerticalPattern = TRUE;
	
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cDevice);
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cWatermarkImagePath);
	cairo_dock_free_label_description (myConfig.pTopTextDescription);
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	g_timer_destroy (myData.pClock);
	
	cairo_dock_free_gauge (myData.pGauge);
	g_free(myData.cType);
	
CD_APPLET_RESET_DATA_END
