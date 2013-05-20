/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
			gldi_dialog_unhide (myData.dialog);
	}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myDock && myData.dialog != NULL)
	{
		gldi_dialog_hide (myData.dialog);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


static void _cd_weblets_set_current_URI (GldiModuleInstance *myApplet, const gchar *cURI)
{
	g_return_if_fail (cURI != NULL);
	
	// on se souvient de la derniere URL chargee.
	g_free (myConfig.cURI_to_load);
	myConfig.cURI_to_load = g_strdup (cURI);
	cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
		G_TYPE_STRING,
		"Configuration",
		"weblet URI",
		myConfig.cURI_to_load,
		G_TYPE_INVALID);

	// on rafraichit le tout !
	cairo_dock_relaunch_task_immediately (myData.pRefreshTimer, myConfig.iReloadTimeout);
}

static void _cd_weblets_open_URI (GtkMenuItem *menu_item, gpointer *data)
{
	GldiModuleInstance *myApplet = data[0];
	gint index_URI = GPOINTER_TO_INT(data[1]);
	cd_message( "weblets: opening predefined URI %d (%s).", index_URI, myConfig.cListURI[index_URI] );
	
	_cd_weblets_set_current_URI (myApplet, myConfig.cListURI[index_URI]);
	
	cd_weblet_free_uri_list ();
}

static void _cd_weblets_reload_webpage (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	// on rafraichit le tout !
	cairo_dock_relaunch_task_immediately (myData.pRefreshTimer, myConfig.iReloadTimeout);
}

//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	gint i = 0;
	CD_APPLET_ADD_IN_MENU_WITH_STOCK ("Reload webpage", GTK_STOCK_REFRESH, _cd_weblets_reload_webpage, CD_APPLET_MY_MENU);
	
	if( myConfig.cListURI != NULL )
	{
		cd_weblet_free_uri_list ();
		gpointer *data;
		while (myConfig.cListURI[i] != NULL)
		{
			data = g_new (gpointer, 2);
			data[0] = myApplet;
			data[1] = GINT_TO_POINTER (i);
			CD_APPLET_ADD_IN_MENU_WITH_DATA (myConfig.cListURI[i], _cd_weblets_open_URI, CD_APPLET_MY_MENU, data);
			s_pUriList = g_list_prepend (s_pUriList, data);
			i++;
		}
	}
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_DROP_DATA_BEGIN
	_cd_weblets_set_current_URI (myApplet, CD_APPLET_RECEIVED_DATA);
CD_APPLET_ON_DROP_DATA_END
