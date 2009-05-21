/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include "stdlib.h"

#include "applet-config.h"
#include "applet-decorator-comics.h"
#include "applet-decorator-modern.h"
#include "applet-decorator-3Dplane.h"
#include "applet-decorator-tooltip.h"
#include "applet-decorator-curly.h"
#include "applet-renderer-text.h"
#include "applet-struct.h"
#include "applet-init.h"

CD_APPLET_PRE_INIT_BEGIN (N_("dialog rendering"),
	2,0,0,
	CAIRO_DOCK_CATEGORY_THEME,
	N_("This plug-in provides some dialog decorators for dialog bubbles."),
	"Fabrice Rey (Fabounet)")
	//\_______________ On definit notre interface.
	//pInterface->reloadModule = reload;
	pInterface->reset_config = CD_APPLET_RESET_CONFIG_FUNC;
	pInterface->reset_data = CD_APPLET_RESET_DATA_FUNC;
	pInterface->read_conf_file = CD_APPLET_READ_CONFIG_FUNC;

	//\_______________ On enregistre les decorateurs (on le fait maintenant au cas ou un dialogue serait affiche pendant le chargement initial).
	cd_decorator_register_comics ();
	cd_decorator_register_modern ();
	cd_decorator_register_3Dplane ();
	cd_decorator_register_tooltip ();  // By ChAnGFu*
	cd_decorator_register_curly ();
	
	//\_______________ On enregistre les moteurs de rendu (on le fait maintenant au cas ou un dialogue serait cree avec pendant le chargement initial).
	rendering_register_text_dialog_renderer ();

	CD_APPLET_ATTACH_TO_INTERNAL_MODULE ("Dialogs");
CD_APPLET_PRE_INIT_END


/*CD_APPLET_INIT_BEGIN
	//\_______________ On enregistre les decorateurs.
	cd_decorator_register_comics ();
	cd_decorator_register_modern ();
	cd_decorator_register_3Dplane ();
	cd_decorator_register_tooltip ();  // By ChAnGFu
	cd_decorator_register_curly ();
	
	if (! cairo_dock_is_loading ())  // plug-in active a la main (en-dehors du chargement du theme).
		cairo_dock_update_dialog_decorator_list_for_gui ();
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On enregistre les decorateurs.
	cairo_dock_remove_dialog_decorator (MY_APPLET_DECORATOR_COMICS_NAME);
	cairo_dock_remove_dialog_decorator (MY_APPLET_DECORATOR_MODERN_NAME);
	cairo_dock_remove_dialog_decorator (MY_APPLET_DECORATOR_3DPLANE_NAME);
	cairo_dock_remove_dialog_decorator (MY_APPLET_DECORATOR_TOOLTIP_NAME);
	
	cairo_dock_update_dialog_decorator_list_for_gui ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		// rien a faire.
	}
CD_APPLET_RELOAD_END
*/
