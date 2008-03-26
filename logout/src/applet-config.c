
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"


extern AppletConfig myConfig;
extern AppletData myData;

CD_APPLET_GET_CONFIG_BEGIN
	myConfig.cUserAction = CD_CONFIG_GET_STRING ("Configuration", "user action");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cUserAction);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	
CD_APPLET_RESET_DATA_END

