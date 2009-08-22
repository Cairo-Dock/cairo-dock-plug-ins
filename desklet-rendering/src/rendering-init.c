/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include "stdlib.h"

#include "rendering-config.h"
#include "rendering-desklet-tree.h"
#include "rendering-desklet-caroussel.h"
#include "rendering-desklet-simple.h"
#include "rendering-desklet-controler.h"
#include "rendering-desklet-mediaplayer.h"
#include "rendering-desklet-slide.h"
#include "rendering-desklet-decorations.h"
#include "rendering-init.h"


CD_APPLET_PRE_INIT_BEGIN (N_("desklet rendering"),
	2,0,0,
	CAIRO_DOCK_CATEGORY_THEME,
	N_("This module provides different views for your desklets."),
	"Fabounet (Fabrice Rey)")
	//\_______________ On definit notre interface.
	//pInterface->read_conf_file = read_conf_file;
	//pInterface->reset_config = reset_config;
	//pInterface->reset_data = reset_data;

	//\_______________ On enregistre les vues.
	rendering_register_tree_desklet_renderer ();
	rendering_register_caroussel_desklet_renderer ();
	rendering_register_simple_desklet_renderer ();
	rendering_register_controler_desklet_renderer ();
	rendering_register_mediaplayer_desklet_renderer ();  // By ChAnGFu
	rendering_register_slide_desklet_renderer ();  // By ChAnGFu
	
	//\_______________ On enregistre les decorations.
	cd_rendering_register_desklet_decorations ();

	CD_APPLET_ATTACH_TO_INTERNAL_MODULE ("Desklets");
CD_APPLET_PRE_INIT_END


/*CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		
		///cairo_dock_set_all_views_to_default ();
	}
CD_APPLET_RELOAD_END*/

