
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iCheckInterval = CD_CONFIG_GET_INTEGER ("Configuration", "delay");
	
	myConfig.cInterface = CD_CONFIG_GET_STRING ("Configuration", "interface");
	if (myConfig.cInterface == NULL)
		myConfig.cInterface = g_strdup ("eth0");
	myConfig.iStringLen = strlen (myConfig.cInterface);
	
	myConfig.iInfoDisplay = CD_CONFIG_GET_INTEGER ("Configuration", "info display");

	myConfig.cGThemePath = CD_CONFIG_GET_GAUGE_THEME ("Configuration", "theme");
	myConfig.fAlpha = CD_CONFIG_GET_DOUBLE ("Configuration", "watermark alpha");
	if (myConfig.fAlpha != 0)
	{
		myConfig.cWatermarkImagePath = CD_CONFIG_GET_FILE_PATH ("Configuration", "watermark image", MY_APPLET_ICON_FILE);
	}
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cInterface);
	g_free (myConfig.cWatermarkImagePath);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	
	if (myData.dbus_proxy_nm != NULL)
		g_object_unref (myData.dbus_proxy_nm);
	
	//Adieu la jauge...
	cairo_dock_free_gauge(myData.pGauge);
	
	g_timer_destroy (myData.pClock);
CD_APPLET_RESET_DATA_END

