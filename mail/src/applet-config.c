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
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.

	myConfig.cNoMailUserImage = cairo_dock_generate_file_path (CD_CONFIG_GET_STRING ("Configuration", "no mail image"));
	myConfig.cHasMailUserImage = cairo_dock_generate_file_path (CD_CONFIG_GET_STRING ("Configuration", "has mail image"));
	myConfig.cNewMailUserSound = cairo_dock_generate_file_path (CD_CONFIG_GET_STRING ("Configuration", "new mail sound"));

    myConfig.cMailApplication = CD_CONFIG_GET_STRING ("Configuration", "mail application");

  	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "Default");

    if( myConfig.cThemePath == NULL )
  		cd_warning ("Attention : couldn't find theme path, or this theme is not valid");

    myConfig.timeEndOfSound = 0;

    if( !myData.mailwatch )
    {
        myData.iNbUnreadMails = 0;
        myData.bNewMailFound = FALSE;

        myData.mailwatch = xfce_mailwatch_new();
        xfce_mailwatch_load_config(myData.mailwatch, pKeyFile);
    }

CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
    g_free( myConfig.cNoMailUserImage );
    g_free( myConfig.cHasMailUserImage );
    g_free( myConfig.cNewMailUserSound );
    g_free (myConfig.cMailApplication);    
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (myIcon->pSubDock != NULL)
	{
		CD_APPLET_DESTROY_MY_SUBDOCK;
	}
  myData.iNbUnreadMails = 0;
  myData.bNewMailFound = FALSE;
CD_APPLET_RESET_DATA_END
