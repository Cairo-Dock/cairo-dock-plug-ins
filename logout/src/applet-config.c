
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_GET_CONFIG_BEGIN
	myConfig.cUserAction = CD_CONFIG_GET_STRING ("Configuration", "user action");
	myConfig.cUserAction2 = CD_CONFIG_GET_STRING ("Configuration", "user action2");
	myConfig.bInvertButtons = CD_CONFIG_GET_BOOLEAN ("Configuration", "invert");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cUserAction);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	
CD_APPLET_RESET_DATA_END

