#include <string.h>

#include "powermanager-struct.h"
#include "powermanager-config.h"

AppletConfig myConfig;
AppletData myData;


CD_APPLET_GET_CONFIG_BEGIN
	
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	
	myConfig.iCheckInterval = 1000 * CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "check interval", 10);
	
	myConfig.quickInfoType = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "quick_info_type", MY_APPLET_TIME);
	
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	
	g_free (myConfig.defaultTitle);
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	
CD_APPLET_RESET_DATA_END

