
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"


extern AppletConfig myConfig;
extern AppletData myData;

CD_APPLET_CONFIG_BEGIN
	reset_config ();
	
	myConfig.cUserAction = CD_CONFIG_GET_STRING ("Configuration", "user action");
CD_APPLET_CONFIG_END


void reset_config (void)
{
	g_free (myConfig.cUserAction);
	myConfig.cUserAction = NULL;
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	memset (&myData, 0, sizeof (AppletData));
}
