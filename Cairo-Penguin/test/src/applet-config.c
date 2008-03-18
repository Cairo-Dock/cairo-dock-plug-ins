
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_CONFIG_BEGIN (NULL, NULL)
	reset_config ();
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	
CD_APPLET_CONFIG_END


void reset_config (void)
{
	
	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	
	memset (&myData, 0, sizeof (AppletData));
}
