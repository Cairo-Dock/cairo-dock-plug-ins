
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.iCheckInterval = 1000 * CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "delay", 10);
	myConfig.dCheckInterval = myConfig.iCheckInterval;
	myConfig.cDefault = g_strdup_printf ("%s/default.png", MY_APPLET_SHARE_DATA_DIR);
	myConfig.cUnknown = g_strdup_printf ("%s/unknown.png", MY_APPLET_SHARE_DATA_DIR);
	myConfig.cOk = g_strdup_printf ("%s/ok.png", MY_APPLET_SHARE_DATA_DIR);
	myConfig.cBad = g_strdup_printf ("%s/bad.png", MY_APPLET_SHARE_DATA_DIR);
	
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	g_free(myConfig.cDefault);
	g_free(myConfig.cUnknown);
	g_free(myConfig.cOk);
	g_free(myConfig.cBad);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN	
	g_source_remove (myData.checkTimer);
	myData.checkTimer = 0;
	cairo_surface_destroy (myData.pDefault);
	cairo_surface_destroy (myData.pUnknown);
	cairo_surface_destroy (myData.pOk);
	cairo_surface_destroy (myData.pBad);
CD_APPLET_RESET_DATA_END

