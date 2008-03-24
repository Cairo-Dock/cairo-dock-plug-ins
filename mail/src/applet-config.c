/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"

extern AppletConfig myConfig;
extern AppletData myData;
extern Icon *myIcon;

CD_APPLET_CONFIG_BEGIN
	reset_config ();
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

CD_APPLET_CONFIG_END


void reset_config (void)
{
    g_free( myConfig.cNoMailUserImage );
    g_free( myConfig.cHasMailUserImage );

	memset (&myConfig, 0, sizeof (AppletConfig));
}

void reset_data (void)
{
	if (myIcon->pSubDock != NULL)
	{
		cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
		myIcon->pSubDock = NULL;
	}

	memset (&myData, 0, sizeof (AppletData));
}
