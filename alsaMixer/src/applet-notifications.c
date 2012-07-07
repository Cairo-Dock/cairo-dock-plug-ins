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

#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-generic.h"
#include "applet-notifications.h"

static const gchar *s_cMixerCmd = NULL;
static gboolean bMixerChecked = FALSE;

CD_APPLET_ON_CLICK_BEGIN
	cd_show_hide ();
CD_APPLET_ON_CLICK_END

static void _check_mixer_cmd (void)
{
	gchar *cResult = cairo_dock_launch_command_sync ("which gnome-control-center");  // Gnome3
	if (cResult != NULL && *cResult == '/')
		s_cMixerCmd = "gnome-control-center sound";
	else
	{
		g_free (cResult);
		cResult = cairo_dock_launch_command_sync ("which gnome-volume-control");  // Gnome2
		if (cResult != NULL && *cResult == '/')  /// TODO: other DE...
			s_cMixerCmd = "gnome-volume-control -p applications";
	}  /// TODO: handle other DE ...
	g_free (cResult);
}

static void _mixer_show_advanced_mixer (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	GError *erreur = NULL;
	if (myConfig.cShowAdvancedMixerCommand != NULL)
	{
		g_spawn_command_line_async (myConfig.cShowAdvancedMixerCommand, &erreur);
	}
	else if (s_cMixerCmd != NULL)
	{
		g_spawn_command_line_async (s_cMixerCmd, &erreur);
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
	
	if (!myConfig.cShowAdvancedMixerCommand && !bMixerChecked)
	{
		bMixerChecked = TRUE;
		_check_mixer_cmd ();
	}
	
	if (myConfig.cShowAdvancedMixerCommand || s_cMixerCmd)
	{
		cLabel = g_strdup_printf ("%s (%s)", D_("Adjust channels"), D_("double-click"));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GTK_STOCK_PREFERENCES, _mixer_show_advanced_mixer, CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
	
	cLabel = g_strdup_printf ("%s (%s)", (myData.bIsMute ? D_("Unmute") : D_("Mute")), D_("middle-click"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, MY_APPLET_SHARE_DATA_DIR"/emblem-mute.svg", cd_toggle_mute, CD_APPLET_MY_MENU);
	g_free (cLabel);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	cd_toggle_mute ();
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_DOUBLE_CLICK_BEGIN
	if (!myConfig.cShowAdvancedMixerCommand && !bMixerChecked)
	{
		bMixerChecked = TRUE;
		_check_mixer_cmd (); // looking for s_cMixerCmd
	}
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
