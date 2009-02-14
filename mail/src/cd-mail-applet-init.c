/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "cd-mail-applet-config.h"
#include "cd-mail-applet-notifications.h"
#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-init.h"

CD_APPLET_INCLUDE_MY_VARS

CD_APPLET_PRE_INIT_BEGIN ("mail", 2, 0, 0, CAIRO_DOCK_CATEGORY_ACCESSORY)
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	pInterface->load_custom_widget = cd_mail_load_custom_widget;
	pInterface->save_custom_widget = cd_mail_save_custom_widget;
CD_APPLET_PRE_INIT_END

static void _load_theme (GError **erreur)
{
	//\_______________ On charge le theme si necessaire, avec en priorite les images utilisateur.
	if (myConfig.cThemePath != NULL && (myConfig.cNoMailUserImage == NULL || myConfig.cHasMailUserImage == NULL))
	{
		GError *tmp_erreur = NULL;
		GDir *dir = g_dir_open (myConfig.cThemePath, 0, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return ;
		}
		
		const gchar *cElementName;
		gchar *cElementPath;
		while ((cElementName = g_dir_read_name (dir)) != NULL)
		{
			cElementPath = g_strdup_printf ("%s/%s", myConfig.cThemePath, cElementName);
			cd_message ("  Mail theme item: %s\n", cElementPath);
			if (strncmp (cElementName, "no_mail", 7) == 0 && myConfig.cNoMailUserImage == NULL)
			{
				myConfig.cNoMailUserImage = cElementPath;
			}
			else if (strncmp (cElementName, "has_mail", 8) == 0 && myConfig.cHasMailUserImage == NULL)
			{
				myConfig.cHasMailUserImage = cElementPath;
			}
			else if (strncmp (cElementName, "new_mail_sound", 14) == 0 && myConfig.cNewMailUserSound == NULL)
			{
				myConfig.cNewMailUserSound = cElementPath;
			}
			else
			{
				g_free (cElementPath);
			}
		}
		g_dir_close (dir);
	}
	if (myConfig.cNoMailUserImage == NULL || myConfig.cHasMailUserImage == NULL || myConfig.cNewMailUserSound == NULL)
	{
		cd_warning ("Attention : couldn't find images, this theme is not valid");
	}
}

CD_APPLET_INIT_BEGIN
	
	GError *erreur = NULL;
	_load_theme (&erreur);
	if (erreur != NULL)
	{
		cd_warning ("mail : %s", erreur->message);
		g_error_free (erreur);
		return;
	}
	
	if (myIcon->acName == NULL && myDock)
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (MAIL_DEFAULT_NAME);
	}
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;

	cd_mail_update_status( NULL );

CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;


	//\_________________ On libere toutes nos ressources.
//    xfce_mailwatch_destroy(myData.mailwatch);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED )
	{
		GError *erreur = NULL;
		_load_theme (&erreur);
		if (erreur != NULL)
		{
			cd_warning ("mail : when trying to load theme : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		
		if (myIcon->acName == NULL && myDock)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (MAIL_DEFAULT_NAME);
		}
		/// prendre en compte les parametres qui ont pu changer...
	}
	else
	{
	}

	cd_mail_update_status( NULL );
	
CD_APPLET_RELOAD_END
