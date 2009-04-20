/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Christophe Chapuis (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-widget.h"

static GList *s_pUriList = NULL;


void cd_weblet_free_uri_list (void)
{
	if (s_pUriList == NULL)
		return ;
	g_list_foreach (s_pUriList, (GFunc) g_free, NULL);
	g_list_free (s_pUriList);
	s_pUriList = NULL;
}


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	if (myDock)
	{
		if (myData.dialog == NULL)
			weblet_build_and_show (myApplet);
		else
			cairo_dock_unhide_dialog (myData.dialog);
	}
CD_APPLET_ON_CLICK_END

static void _cd_weblets_open_URI (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	gint index_URI = GPOINTER_TO_INT(data[1]);
	cd_message( "weblets: opening predefined URI %d (%s).", index_URI, myConfig.cListURI[index_URI] );
	
	// on se souvient de la derniere URL chargee.
	g_free (myConfig.cURI_to_load);
	myConfig.cURI_to_load = g_strdup (myConfig.cListURI[index_URI]);
	cairo_dock_update_conf_file (myApplet->cConfFilePath,
		G_TYPE_STRING,
		"Configuration",
		"weblet URI",
		myConfig.cURI_to_load,
		G_TYPE_INVALID);

	// on rafraichit le tout !
	cairo_dock_relaunch_measure_immediately (myData.pRefreshTimer, myConfig.iReloadTimeout);
	
	cd_weblet_free_uri_list ();
}

static void _cd_weblets_reload_webpage (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	// on rafraichit le tout !
	cairo_dock_relaunch_measure_immediately (myData.pRefreshTimer, myConfig.iReloadTimeout);
}


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	gint i = 0;
	CD_APPLET_ADD_IN_MENU("Reload webpage", _cd_weblets_reload_webpage, CD_APPLET_MY_MENU);
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		if( myConfig.cListURI != NULL )
		{
			cd_weblet_free_uri_list ();
			gpointer *data;
			while (myConfig.cListURI[i] != NULL)
			{
				data = g_new (gpointer, 2);
				data[0] = myApplet;
				data[1] = GINT_TO_POINTER (i);
				CD_APPLET_ADD_IN_MENU_WITH_DATA (myConfig.cListURI[i], _cd_weblets_open_URI, pSubMenu, data);
				s_pUriList = g_list_prepend (s_pUriList, data);
				i++;
			}
		}
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END
