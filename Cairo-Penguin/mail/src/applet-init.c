/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"

AppletConfig myConfig;
AppletData myData;


CD_APPLET_DEFINITION ("mail", 1, 5, 0, CAIRO_DOCK_CATEGORY_ACCESSORY)


static void _load_surfaces (void) {
	GString *sImagePath = g_string_new ("");

	//Chargement de l'image "pas de mail"
		g_string_printf (sImagePath, "%s/cd_mail_nomail.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pNoMailSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);

	//Chargement de l'image "il y a un des mails"
		g_string_printf (sImagePath, "%s/cd_mail_newmail.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pHasMailSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);

	g_string_free (sImagePath, TRUE);
}

CD_APPLET_INIT_BEGIN (erreur)
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT

	//\_______________ On charge en priorite les images utilisateur.
    myData.pNoMailSurface = NULL;
    myData.pHasMailSurface = NULL;

	_load_surfaces();

	if (myConfig.cNoMailUserImage != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cNoMailUserImage);
		myData.pNoMailSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath)
		g_free (cUserImagePath);
	}
	if (myConfig.cHasMailUserImage != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cHasMailUserImage);
		myData.pHasMailSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath)
		g_free (cUserImagePath);
	}
    if( myData.pNoMailSurface )
    {
        CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pNoMailSurface);
    }

    xfce_mailwatch_signal_connect(myData.mailwatch,
            XFCE_MAILWATCH_SIGNAL_NEW_MESSAGE_COUNT_CHANGED,
            mailwatch_new_messages_changed_cb, NULL);

    xfce_mailwatch_force_update(myData.mailwatch);

CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT


	//\_________________ On libere toutes nos ressources.
    xfce_mailwatch_destroy(myData.mailwatch);

	reset_data ();
	reset_config ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
        //\_______________ On charge en priorite les images utilisateur.
        myData.pNoMailSurface = NULL;
        myData.pHasMailSurface = NULL;

        _load_surfaces();

        if (myConfig.cNoMailUserImage != NULL)
        {
            gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cNoMailUserImage);
            myData.pNoMailSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath)
            g_free (cUserImagePath);
        }
        if (myConfig.cHasMailUserImage != NULL)
        {
            gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cHasMailUserImage);
            myData.pHasMailSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath)
            g_free (cUserImagePath);
        }
        if( myData.pNoMailSurface )
        {
            CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pNoMailSurface)
        }
	}
	else
	{

	}
    CD_APPLET_REDRAW_MY_ICON

CD_APPLET_RELOAD_END
