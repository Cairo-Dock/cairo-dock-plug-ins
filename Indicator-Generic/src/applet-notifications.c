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
#include "applet-notifications.h"


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	GtkMenu *pMenu = cd_indicator3_get_menu (myData.pEntry);
	if (pMenu)
		cairo_dock_popup_menu_on_icon (GTK_WIDGET (pMenu), myApplet->pIcon, myContainer);
	else // should not happen except if the daemon crash?
	{
		gchar *cErrorMessage = g_strdup_printf (D_("This indicator service did not reply.\n"
			"Please check that '%s' is correctly installed and its daemon is running."),
			myConfig.cIndicatorName);
		cairo_dock_show_temporary_dialog_with_icon (cErrorMessage, myApplet->pIcon, myContainer, 8000., "same icon");
		g_free (cErrorMessage);
	}
	
CD_APPLET_ON_CLICK_END
