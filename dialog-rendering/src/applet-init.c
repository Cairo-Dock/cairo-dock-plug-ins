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
#include "applet-decorator-comics.h"
#include "applet-decorator-modern.h"
#include "applet-decorator-3Dplane.h"
#include "applet-decorator-tooltip.h"
#include "applet-decorator-curly.h"
#include "applet-renderer-text.h"
#include "applet-struct.h"
#include "applet-init.h"

CD_APPLET_DEFINE2_BEGIN ("dialog rendering",
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_THEME,
	"This plug-in provides some decorators for dialog bubbles.",
	"Fabrice Rey (Fabounet)")
	//\_______________ On definit notre interface.
	//pInterface->reloadModule = reload;
	//pInterface->reset_config = CD_APPLET_RESET_CONFIG_FUNC;
	//pInterface->reset_data = CD_APPLET_RESET_DATA_FUNC;
	//pInterface->read_conf_file = CD_APPLET_READ_CONFIG_FUNC;
	pInterface->initModule = CD_APPLET_INIT_FUNC;
	pInterface->stopModule = CD_APPLET_STOP_FUNC;

	//\_______________ On enregistre les moteurs de rendu (on le fait maintenant au cas ou un dialogue serait cree avec pendant le chargement initial).
	rendering_register_text_dialog_renderer ();

	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
	CD_APPLET_EXTEND_MANAGER ("Dialogs");
CD_APPLET_DEFINE2_END


CD_APPLET_INIT_BEGIN
	//\_______________ On enregistre les decorateurs.
	cd_decorator_register_comics ();
	cd_decorator_register_modern ();
	cd_decorator_register_tooltip ();  // By ChAnGFu
	cd_decorator_register_curly ();
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On desenregistre les decorateurs.
	cairo_dock_remove_dialog_decorator (MY_APPLET_DECORATOR_COMICS_NAME);
	cairo_dock_remove_dialog_decorator (MY_APPLET_DECORATOR_MODERN_NAME);
	cairo_dock_remove_dialog_decorator (MY_APPLET_DECORATOR_TOOLTIP_NAME);
	cairo_dock_remove_dialog_decorator (MY_APPLET_DECORATOR_CURLY_NAME);
CD_APPLET_STOP_END


/*CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		// rien a faire.
	}
CD_APPLET_RELOAD_END
*/
