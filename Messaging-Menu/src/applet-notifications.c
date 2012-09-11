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

#include "applet-struct.h"
#ifndef INDICATOR_MESSAGES_12_10
#include "applet-menu.h"
#else
#include "applet-indicator3.h"
#endif
#include "applet-notifications.h"

/*
 * An horrible hack to remove double separators in the menu.
 * I don't know why we have this bug... hope we'll remove it soon!
 */

/// REMOVE ME WHEN IT'S POSSIBLE! :)
#ifdef FORCE_REMOVE_DOUBLE_SEPARATORS
static void _remove_double_separators (GtkContainer *pContainer)
{
	if (pContainer == NULL)
		return;

	gboolean bPrevIsSeparator = TRUE; // to remove the first entry if it's a separator
	GList *ic;
	GtkWidget *pIcon;
	for (ic = gtk_container_get_children (pContainer); ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (GTK_IS_SEPARATOR_MENU_ITEM (pIcon))
		{
			if (bPrevIsSeparator)
				gtk_widget_destroy (pIcon);
			bPrevIsSeparator = TRUE;
		}
		else
			bPrevIsSeparator = FALSE;
	}
	g_list_free (ic);
}
#endif

static inline void _show_menu (void)
{
	#ifdef FORCE_REMOVE_DOUBLE_SEPARATORS
	_remove_double_separators (GTK_CONTAINER (myData.pIndicator->pMenu));
	#endif

	#ifndef INDICATOR_MESSAGES_12_10
	if (! cd_indicator_show_menu (myData.pIndicator))
	#else
	GtkMenu *pMenu = cd_indicator3_get_menu (myData.pEntry);
	if (pMenu)
		cairo_dock_popup_menu_on_icon (GTK_WIDGET (pMenu), myIcon, myContainer);
	else
	#endif
		cairo_dock_show_temporary_dialog_with_icon (D_("The Messaging service did not reply.\nPlease check that it is correctly installed."), myIcon, myContainer, 4000., "same icon");
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	_show_menu ();
CD_APPLET_ON_CLICK_END

/*
//\___________ Same as ON_CLICK, but with middle-click.
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	
	
CD_APPLET_ON_MIDDLE_CLICK_END


//\___________ Same as ON_CLICK, but with scroll. Moreover, CD_APPLET_SCROLL_UP tels you is the user scrolled up, CD_APPLET_SCROLL_DOWN the opposite.
CD_APPLET_ON_SCROLL_BEGIN
	
	
CD_APPLET_ON_SCROLL_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN

CD_APPLET_ON_BUILD_MENU_END
*/

void cd_messaging_on_keybinding_pull (const gchar *keystring, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	_show_menu ();
	CD_APPLET_LEAVE();
}
