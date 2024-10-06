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

#include "rendering-config.h"
#include "rendering-desklet-tree.h"
#include "rendering-desklet-caroussel.h"
#include "rendering-desklet-simple.h"
//#include "rendering-desklet-controler.h"
//#include "rendering-desklet-mediaplayer.h"
#include "rendering-desklet-slide.h"
#include "rendering-desklet-viewport.h"
#include "rendering-desklet-panel.h"
#include "rendering-desklet-decorations.h"
#include "rendering-init.h"


CD_APPLET_DEFINE2_BEGIN ("desklet rendering",
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_THEME,
	"This module provides different views for your desklets.",
	"Fabounet (Fabrice Rey)")
	//\_______________ On definit notre interface.
	//pInterface->read_conf_file = read_conf_file;
	//pInterface->reset_config = reset_config;
	//pInterface->reset_data = reset_data;

	//\_______________ On enregistre les vues.
	rendering_register_tree_desklet_renderer ();
	rendering_register_caroussel_desklet_renderer ();
	rendering_register_simple_desklet_renderer ();
	//rendering_register_controler_desklet_renderer ();
	//rendering_register_mediaplayer_desklet_renderer ();  // By ChAnGFu
	rendering_register_slide_desklet_renderer ();  // By ChAnGFu
	rendering_register_viewport_desklet_renderer ();  // By Tofe&Fab
	rendering_register_panel_desklet_renderer ();
	
	//\_______________ On enregistre les decorations.
	cd_rendering_register_desklet_decorations ();
	
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
	CD_APPLET_EXTEND_MANAGER ("Desklets");
CD_APPLET_DEFINE2_END


/*CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		
		///cairo_dock_set_all_views_to_default ();
	}
CD_APPLET_RELOAD_END*/

