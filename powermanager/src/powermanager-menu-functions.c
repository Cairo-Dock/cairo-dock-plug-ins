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
	gldi_dialogs_remove_on_icon (myIcon);
	cd_powermanager_bubble ();
CD_APPLET_ON_CLICK_END

static void _power_launch_settings (G_GNUC_UNUSED GtkMenuItem *menu_item, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	
	if (myData.cPowerPrefCmd)
	{
		
		cairo_dock_launch_command_argv_full2 (myData.cPowerPrefCmd, NULL,
			GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE, myData.pPowerPrefApp);
	}
	
	CD_APPLET_LEAVE ();
}

static void _power_launch_stats (G_GNUC_UNUSED GtkMenuItem *menu_item, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	
	if (myData.pPowerStatsApp)
		gldi_app_info_launch (myData.pPowerStatsApp, NULL);
	
	CD_APPLET_LEAVE ();
}

static const char * const s_gnome_power_settings[] = {"gnome-control-center", "power", NULL};

CD_APPLET_ON_BUILD_MENU_BEGIN
	gboolean bAddSeparator = FALSE;
	// Power preferences
	if (!myData.bPowerMenuChecked)
	{
		myData.bPowerMenuChecked = TRUE;
		
		// preferences
		gchar *cResult = cairo_dock_launch_command_sync ("which gnome-control-center");  // Gnome3
		if (cResult != NULL && *cResult == '/')
		{
			myData.cPowerPrefCmd = s_gnome_power_settings;
			GDesktopAppInfo *tmp = g_desktop_app_info_new ("org.gnome.Settings.desktop");
			myData.pPowerPrefApp = tmp ? G_APP_INFO (tmp) : NULL;
		}  /// TODO: handle other DE ... (should be moved to *-integration?)
		g_free (cResult);
		
		// stats
		cResult = cairo_dock_register_class ("org.gnome.powerstats");
		if (cResult)
		{
			myData.pPowerStatsApp = cairo_dock_get_class_app_info (cResult);
			if (myData.pPowerStatsApp) gldi_object_ref (GLDI_OBJECT (myData.pPowerStatsApp));
			g_free (cResult);
		}
	}
	if (myData.cPowerPrefCmd)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Set up power management"), MY_APPLET_SHARE_DATA_DIR"/default-charge.svg", _power_launch_settings, CD_APPLET_MY_MENU, NULL);
		bAddSeparator = TRUE;
	}
	if (myData.pPowerStatsApp)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Power statistics"), MY_APPLET_SHARE_DATA_DIR"/default-charge.svg", _power_launch_stats, CD_APPLET_MY_MENU, NULL);
		bAddSeparator = TRUE;
	}
	
	if (bAddSeparator)
		CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);

CD_APPLET_ON_BUILD_MENU_END
