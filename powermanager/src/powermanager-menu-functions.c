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

#include "powermanager-dbus.h"
#include "powermanager-draw.h"
#include "powermanager-struct.h"
#include "powermanager-menu-functions.h"


CD_APPLET_ON_CLICK_BEGIN
	cairo_dock_remove_dialog_if_any (myIcon);
	cd_powermanager_bubble();
CD_APPLET_ON_CLICK_END

void power_config(void) {  /// a mettre dans les plug-ins d'integration.
	GError *erreur = NULL;
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME || g_iDesktopEnv == CAIRO_DOCK_XFCE)
	{
		g_spawn_command_line_async ("gnome-power-preferences", &erreur);
	}
	else if (g_iDesktopEnv == CAIRO_DOCK_KDE)
	{
		//Ajouter les lignes de KDE
	}
	if (erreur != NULL)
	{
		cd_warning ("PM : %s", erreur->message);
		g_error_free (erreur);
	}
}


CD_APPLET_ON_BUILD_MENU_BEGIN
	//on rajoute un sous menu, sinon ce n'est pas esthétique
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	CD_APPLET_ADD_IN_MENU (D_("Set up power management"), power_config, pSubMenu);
	if (myData.dbus_enable)
	{
		CD_APPLET_ADD_IN_MENU (D_("Halt"), power_halt, pSubMenu);
		CD_APPLET_ADD_IN_MENU (D_("Hibernate"), power_hibernate, pSubMenu);
		CD_APPLET_ADD_IN_MENU (D_("Suspend"), power_suspend, pSubMenu);
		CD_APPLET_ADD_IN_MENU (D_("Reboot"), power_reboot, pSubMenu);
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END
