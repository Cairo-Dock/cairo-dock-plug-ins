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
#include <math.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-generic.h"
#include "applet-notifications.h"


CD_APPLET_ON_CLICK_BEGIN
	cd_show_hide ();
CD_APPLET_ON_CLICK_END


static void _mixer_show_advanced_mixer (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	GError *erreur = NULL;
	if (myConfig.cShowAdvancedMixerCommand != NULL)
	{
		g_spawn_command_line_async (myConfig.cShowAdvancedMixerCommand, &erreur);
	}
	else
	{
		gchar *cResult = cairo_dock_launch_command_sync ("which gnome-volume-control");
		if (cResult != NULL && *cResult == '/')
		{
			g_spawn_command_line_async ("gnome-volume-control -p applications", &erreur); // gnome 2
		}
		else  /// TODO: we need to handle the other DE too ...
		{
			g_spawn_command_line_async ("gnome-control-center sound", &erreur); // Gnome 3
		}
		g_free (cResult);
	}
	
	if (erreur != NULL)
	{
		cd_warning ("Attention : when trying to execute '%s' : %s", myConfig.cShowAdvancedMixerCommand, erreur->message);
		g_error_free (erreur);
	}
	CD_APPLET_LEAVE();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	gchar *cLabel;
	
	cLabel = g_strdup_printf ("%s (%s)", D_("Adjust channels"), D_("double-click"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GTK_STOCK_PREFERENCES, _mixer_show_advanced_mixer, CD_APPLET_MY_MENU);
	g_free (cLabel);
	
	cLabel = g_strdup_printf ("%s (%s)", (myData.bIsMute ? D_("Unmute") : D_("Mute")), D_("middle-click"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, MY_APPLET_SHARE_DATA_DIR"/emblem-mute.svg", cd_toggle_mute, CD_APPLET_MY_MENU);
	g_free (cLabel);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	cd_toggle_mute ();
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_DOUBLE_CLICK_BEGIN
	_mixer_show_advanced_mixer (NULL, NULL);
CD_APPLET_ON_DOUBLE_CLICK_END


CD_APPLET_ON_SCROLL_BEGIN
	double delta;
	if (CD_APPLET_SCROLL_UP)
		delta = myConfig.iScrollVariation;
	else
		delta = - myConfig.iScrollVariation;
	
	int iVolume = cd_get_volume ();
	
	iVolume = MAX (0, MIN (iVolume + delta, 100));
	
	cd_set_volume (iVolume);
CD_APPLET_ON_SCROLL_END


void mixer_on_keybinding_pull (const char *keystring, gpointer user_data)
{
	CD_APPLET_ENTER;
	cd_show_hide ();
	CD_APPLET_LEAVE();
}
