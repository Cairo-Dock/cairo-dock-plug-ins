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

/// a mettre dans les plug-ins d'integration.
static void power_launch_cmd (GtkMenuItem *menu_item, const gchar *cCommand)
{
	GError *erreur = NULL;
	g_spawn_command_line_async (cCommand, &erreur);

	if (erreur != NULL)
	{
		cd_warning ("PM : %s", erreur->message);
		g_error_free (erreur);
	}
}

CD_APPLET_ON_BUILD_MENU_BEGIN
	gboolean bAddSeparator = FALSE;
	// Power preferences
	static gboolean bPowerPrefChecked = FALSE;
	static const gchar *cPowerPrefCmd = NULL;
	if (!bPowerPrefChecked)
	{
		bPowerPrefChecked = TRUE;
		gchar *cResult = cairo_dock_launch_command_sync ("which gnome-control-center");  // Gnome3
		if (cResult != NULL && *cResult == '/')
		{
			cPowerPrefCmd = "gnome-control-center power";
		}
		else
		{
			g_free (cResult);
			cResult = cairo_dock_launch_command_sync ("which gnome-power-preferences");  // Gnome2
			if (cResult != NULL && *cResult == '/')  /// TODO: other DE...
				cPowerPrefCmd = "gnome-power-preferences";
		}  /// TODO: handle other DE ...
		g_free (cResult);
	}
	if (cPowerPrefCmd)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Set up power management"), MY_APPLET_SHARE_DATA_DIR"/default-charge.svg", power_launch_cmd, CD_APPLET_MY_MENU, (gpointer)cPowerPrefCmd);
		bAddSeparator = TRUE;
	}
	
	// Power statistics
	static gboolean bPowerStatsChecked = FALSE;
	static const gchar *cPowerStatsCmd = NULL;
	if (!bPowerStatsChecked)
	{
		bPowerStatsChecked = TRUE;
		gchar *cResult = cairo_dock_launch_command_sync ("which gnome-power-statistics");
		if (cResult != NULL && *cResult == '/')  /// TODO: other DE...
			cPowerStatsCmd = "gnome-power-statistics";
		g_free (cResult);
	}
	if (cPowerStatsCmd)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Power statistics"), MY_APPLET_SHARE_DATA_DIR"/default-charge.svg", power_launch_cmd, CD_APPLET_MY_MENU, (gpointer)cPowerStatsCmd);
		bAddSeparator = TRUE;
	}
	
	if (bAddSeparator)
		CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
	
	// Power actions (Hibernate/Suspend)
	#ifdef CD_UPOWER_AVAILABLE  // if Upower is available, we should be able to suspend; if not, then it's probably just a problem with consolekit, which should be fixed by the user; so show the items to give the user a hint about the problem.
	pMenuItem = CD_APPLET_ADD_IN_MENU (D_("Hibernate"), cd_power_hibernate, CD_APPLET_MY_MENU);
	if (! cd_power_can_hibernate ())
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	pMenuItem = CD_APPLET_ADD_IN_MENU (D_("Suspend"), cd_power_suspend, CD_APPLET_MY_MENU);
	if (! cd_power_can_suspend ())
		gtk_widget_set_sensitive (pMenuItem, FALSE);
	#else
	if (cd_power_can_hibernate ())
		CD_APPLET_ADD_IN_MENU (D_("Hibernate"), cd_power_hibernate, CD_APPLET_MY_MENU);
	if (cd_power_can_suspend ())
		CD_APPLET_ADD_IN_MENU (D_("Suspend"), cd_power_suspend, CD_APPLET_MY_MENU);
	#endif
CD_APPLET_ON_BUILD_MENU_END
