/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-mixer.h"
#include "applet-notifications.h"


CD_APPLET_ABOUT (D_("This is the AlsaMixer applet\n made by Fabounet (Fabrice Rey) for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	mixer_show_hide_dialog ();
CD_APPLET_ON_CLICK_END


static void _mixer_show_advanced_mixer (GtkMenuItem *menu_item, gpointer data)
{
	GError *erreur = NULL;
	if (myConfig.cShowAdvancedMixerCommand != NULL)
	{
		g_spawn_command_line_async (myConfig.cShowAdvancedMixerCommand, &erreur);
	}
	else
	{
		g_spawn_command_line_async ("gnome-volume-control", &erreur);
	}
	
	if (erreur != NULL)
	{
		cd_warning ("Attention : when trying to execute '%s' : %s", myConfig.cShowAdvancedMixerCommand, erreur->message);
		g_error_free (erreur);
	}
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_IN_MENU(_("Adjsut channels"), _mixer_show_advanced_mixer, pSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	mixer_switch_mute ();
CD_APPLET_ON_MIDDLE_CLICK_END


void mixer_on_keybinding_pull (const char *keystring, gpointer user_data)
{
	mixer_show_hide_dialog ();
}

CD_APPLET_ON_SCROLL_BEGIN
	int iVolume = mixer_get_mean_volume ();  // [0;100]
	if (CD_APPLET_SCROLL_DOWN)
	{
		iVolume = MAX (iVolume - myConfig.iScrollVariation, 0);
	}
	else if (CD_APPLET_SCROLL_UP)
	{
		iVolume = MIN (iVolume + myConfig.iScrollVariation, 100);
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	mixer_set_volume (iVolume);
CD_APPLET_ON_SCROLL_END
