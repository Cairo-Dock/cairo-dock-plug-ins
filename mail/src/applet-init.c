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


CD_APPLET_DEFINITION ("mail", 1, 5, 4, CAIRO_DOCK_CATEGORY_ACCESSORY)

CD_APPLET_INIT_BEGIN (erreur)
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT

    xfce_mailwatch_signal_connect(myData.mailwatch,
            XFCE_MAILWATCH_SIGNAL_NEW_MESSAGE_COUNT_CHANGED,
            mailwatch_new_messages_changed_cb, NULL);

    xfce_mailwatch_force_update(myData.mailwatch);

CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT


	//\_________________ On libere toutes nos ressources.
    xfce_mailwatch_destroy(myData.mailwatch);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED )
	{
		/// prendre en compte les parametres qui ont pu changer...
	}
	else
	{
	}

    xfce_mailwatch_force_update(myData.mailwatch);  /// utile ?

CD_APPLET_RELOAD_END
