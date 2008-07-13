/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-widget.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the weblets applet\n made by Christophe Chapuis for Cairo-Dock"))


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	
	
CD_APPLET_ON_CLICK_END

static void _cd_weblets_open_URI (GtkMenuItem *menu_item, gpointer *data)
{
	gint index_URI = GPOINTER_TO_UINT(data);

	g_free (myConfig.cURI_to_load);
#if 0
  // note de Christope: juste une idee de code, qui en soit ne marche pas car pKeyFile n'existe pas ici.
  // Pour avoir un code qui marche, on peut s'inspirer de l'applet mail (dans les notifications).
  // Cependant: changer la configuration de l'applet est-il vraiment ce qu'on veut ?
	g_key_file_set_string (pKeyFile, "Configuration", "weblet URI", myConfig.cListURI[index_URI]);
	myConfig.cURI_to_load = CD_CONFIG_GET_STRING ("Configuration", "weblet URI");
#else
	myConfig.cURI_to_load = g_strdup(myConfig.cListURI[index_URI]);
#endif

	// on rafraichit le tout !
	if( myData.pRefreshTimer != NULL )
	{
		cairo_dock_relaunch_measure_immediately (myData.pRefreshTimer, myConfig.iReloadTimeout);
	}
	else
	{
		// mise en place du timer
		myData.pRefreshTimer = cairo_dock_new_measure_timer (myConfig.iReloadTimeout,
			NULL,
			NULL,
			cd_weblets_refresh_page);
	}
	cairo_dock_launch_measure (myData.pRefreshTimer); // ceci lance au moins une fois le chargement de la page
	if( myConfig.iReloadTimeout == 0 )
	{
		// oublions le, il ne sert deja plus a rien...
		myData.pRefreshTimer = NULL;
	}
}


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	gint i = 0;
	CD_APPLET_ADD_SUB_MENU ("weblets", pSubMenu, CD_APPLET_MY_MENU)
		if( myConfig.cListURI != NULL )
		{
			while (myConfig.cListURI[i] != NULL)
			{
				CD_APPLET_ADD_IN_MENU_WITH_DATA (myConfig.cListURI[i], _cd_weblets_open_URI, pSubMenu, i)
				i++;
			}
		}
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
