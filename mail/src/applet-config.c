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

	myConfig.cNoMailUserImage = CD_CONFIG_GET_STRING ("Configuration", "no mail image");
	myConfig.cHasMailUserImage = CD_CONFIG_GET_STRING ("Configuration", "has mail image");

    myConfig.cMailApplication = CD_CONFIG_GET_STRING ("Configuration", "mail application");

    if( myConfig.cNoMailUserImage == NULL || strcmp(myConfig.cNoMailUserImage,"") == 0  )
    {
        g_free( myConfig.cNoMailUserImage );
        myConfig.cNoMailUserImage = g_strdup_printf ("%s/cd_mail_nomail.svg", MY_APPLET_SHARE_DATA_DIR);
    }
    if( myConfig.cHasMailUserImage == NULL || strcmp(myConfig.cHasMailUserImage,"") == 0  )
    {
        g_free( myConfig.cHasMailUserImage );
        myConfig.cHasMailUserImage = g_strdup_printf ("%s/cd_mail_newmail.svg", MY_APPLET_SHARE_DATA_DIR);
    }

    if( !myData.mailwatch )
    {
        myData.mailwatch = xfce_mailwatch_new();
        xfce_mailwatch_load_config(myData.mailwatch, pKeyFile);
    }

CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
    g_free( myConfig.cNoMailUserImage );
    g_free( myConfig.cHasMailUserImage );
    g_free (myConfig.cMailApplication);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (myIcon->pSubDock != NULL)
	{
		CD_APPLET_DESTROY_MY_SUBDOCK
	}
CD_APPLET_RESET_DATA_END
