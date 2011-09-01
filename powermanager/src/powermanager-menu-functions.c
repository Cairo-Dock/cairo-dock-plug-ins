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
#include <glib/gi18n.h>

#include "powermanager-upower.h"
#include "powermanager-draw.h"
#include "powermanager-struct.h"
#include "powermanager-menu-functions.h"


CD_APPLET_ON_CLICK_BEGIN
	cairo_dock_remove_dialog_if_any (myIcon);
	cd_powermanager_bubble ();
CD_APPLET_ON_CLICK_END

static void power_config (void) {  /// a mettre dans les plug-ins d'integration.
	GError *erreur = NULL;
	g_spawn_command_line_async ("gnome-power-preferences", &erreur);

	if (erreur != NULL)
	{
		cd_warning ("PM : %s", erreur->message);
		g_error_free (erreur);
	}
}

static void power_stat (void)
{
	GError *erreur = NULL;
	g_spawn_command_line_async ("gnome-power-statistics", &erreur);

	if (erreur != NULL)
	{
		cd_warning ("PM : %s", erreur->message);
		g_error_free (erreur);
	}
}

CD_APPLET_ON_BUILD_MENU_BEGIN
	// Sub-Menu
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	gchar *cResult = cairo_dock_launch_command_sync ("which gnome-power-preferences"); // not available on Gnome3 => gnome-control-center => Energy
	if (cResult != NULL && *cResult == '/')  /// TODO: other DE...
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Set up power management"), MY_APPLET_SHARE_DATA_DIR"/default-battery.svg", power_config, CD_APPLET_MY_MENU);
	cResult = cairo_dock_launch_command_sync ("which gnome-power-statistics");
	if (cResult != NULL && *cResult == '/')
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Power statistics"), MY_APPLET_SHARE_DATA_DIR"/default-battery.svg", power_stat, CD_APPLET_MY_MENU);
	g_free (cResult);
	if (cd_power_can_hibernate ())
		CD_APPLET_ADD_IN_MENU (D_("Hibernate"), cd_power_hibernate, pSubMenu);
	if (cd_power_can_suspend ())
		CD_APPLET_ADD_IN_MENU (D_("Suspend"), cd_power_suspend, pSubMenu);
	CD_APPLET_ADD_SEPARATOR (pSubMenu);
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END
