/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-bookmarks.h"
#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (_D("This is the shortcuts applet\n made by Fabounet for Cairo-Dock"))


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
	cairo_dock_show_hide_desktop (! bDesktopIsVisible);
CD_APPLET_ON_MIDDLE_CLICK_END


static void _cd_shortcuts_remove_bookmark (GtkMenuItem *menu_item, gchar *cURI)
{
	cd_shortcuts_remove_one_bookmark (cURI);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	if (CD_APPLET_CLICKED_ICON == myIcon)
	{
		CD_APPLET_ADD_SUB_MENU ("shortcuts", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
	}
	else if (CD_APPLET_CLICKED_ICON != NULL && CD_APPLET_CLICKED_ICON->iType == 10)
	{
		cd_message (" menu sur %s(%s)\n", CD_APPLET_CLICKED_ICON->acName, CD_APPLET_CLICKED_ICON->cBaseURI);
		CD_APPLET_ADD_IN_MENU_WITH_DATA (_D("Remove this bookmark"), _cd_shortcuts_remove_bookmark, CD_APPLET_MY_MENU, CD_APPLET_CLICKED_ICON->cBaseURI)
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_DROP_DATA_BEGIN
	if (myIcon == NULL || myIcon->pSubDock == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	cd_message ("  nouveau signet : %s\n", CD_APPLET_RECEIVED_DATA);
	gchar *cName=NULL, *cURI=NULL, *cIconName=NULL;
	gboolean bIsDirectory;
	int iVolumeID = 0;
	double fOrder;
	if (cairo_dock_fm_get_file_info (CD_APPLET_RECEIVED_DATA,
		&cName,
		&cURI,
		&cIconName,
		&bIsDirectory,
		&iVolumeID,
		&fOrder,
		0))
	{
		if (! iVolumeID && ! bIsDirectory)
		{
			cd_message ("ce n'est pas un signet\n");
		}
		else
		{
			cd_shortcuts_add_one_bookmark (cURI);
		}
	}
	else
	{
		cd_message ("couldn't get info about '%s', we won't add it\n", CD_APPLET_RECEIVED_DATA);
	}
	g_free (cName);
	g_free (cURI);
	g_free (cIconName);
CD_APPLET_ON_DROP_DATA_END

