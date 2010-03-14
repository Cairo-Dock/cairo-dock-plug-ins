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

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("Log-out"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_DESKTOP,
	N_("A very simple applet that adds an icon to log out from your session\n"
	"Left click to log out, middle click to shutdown\n"
	"You can invert this order if you prefer to shutdown on left-click."),
	"Fabounet (Fabrice Rey)")


CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME)  // on prend le controle de l'icone de la fenetre.
		CD_APPLET_MANAGE_APPLICATION ("gnome-session");  // en fait depuis Gnome 2.28 seulement, avant c'etait x-session-manager.
	else if (g_iDesktopEnv == CAIRO_DOCK_XFCE)
		CD_APPLET_MANAGE_APPLICATION ("x-session-manager");
	/// trouver celui de KDE ...
	
	//\_______________ On enregistre nos notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	
	//\_______________ On (re)lance l'eteignage programme.
	cd_logout_set_timer ();
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	
	CD_APPLET_MANAGE_APPLICATION (NULL);  // on relache le controle de l'icone de la fenetre.
	
	if (myData.iSidTimer != 0)
		g_source_remove (myData.iSidTimer);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	}
CD_APPLET_RELOAD_END
