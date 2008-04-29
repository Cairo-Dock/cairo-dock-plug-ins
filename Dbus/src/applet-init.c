/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-dbus.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("Dbus", 1, 5, 4, CAIRO_DOCK_CATEGORY_DESKTOP)


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
void init (GKeyFile *pKeyFile, Icon *pIcon, CairoContainer *pContainer, gchar *cConfFilePath, GError **erreur)
{
	read_conf_file (pKeyFile, cConfFilePath);
	cd_dbus_launch_service ();
}


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
void stop (void)
{
	cd_dbus_stop_service ();
	
	reset_data ();
	reset_config ();
}


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
gboolean reload (GKeyFile *pKeyFile, gchar *cConfFilePath, CairoContainer *pNewContainer)
{
	
	return TRUE;
}
