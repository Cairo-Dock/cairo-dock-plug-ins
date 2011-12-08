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

#if (INDICATOR_OLD_NAMES == 0)
#include "dbus-shared-names.h"
#else
#include "dbus-shared-names-old.h"
#endif

#include "common-defs.h"
#include "volume-widget.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-sound.h"
#include "applet-menu.h"
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN (N_("Sound Menu"),
	2, 4, 0,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("A menu to control the volume of each current audio sources.\n"
	"It requires the Sound service, which is available on Ubuntu by default."),
	"Fabounet")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_ALLOW_EMPTY_TITLE
CD_APPLET_DEFINE_END



//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	myData.pIndicator = cd_indicator_new (myApplet,
		INDICATOR_SOUND_DBUS_NAME,
		INDICATOR_SOUND_SERVICE_DBUS_OBJECT_PATH,
		INDICATOR_SOUND_DBUS_INTERFACE,
		INDICATOR_SOUND_MENU_DBUS_OBJECT_PATH,
		0);	
	myData.pIndicator->on_connect 		= cd_sound_on_connect;
	myData.pIndicator->on_disconnect 		= cd_sound_on_disconnect;
	myData.pIndicator->get_initial_values 	= cd_sound_get_initial_values;
	myData.pIndicator->add_menu_handler 	= cd_sound_add_menu_handler;
	
	// data renderer
	if (myConfig.iVolumeEffect == VOLUME_EFFECT_GAUGE)
	{
		CairoDataRendererAttribute *pRenderAttr = NULL;  // les attributs du data-renderer global.
		CairoGaugeAttribute attr;  // les attributs de la jauge.
		memset (&attr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "gauge";
		attr.cThemePath = myConfig.cGThemePath;
		
		CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON (pRenderAttr);
	}
	else if (myConfig.iVolumeEffect == VOLUME_EFFECT_BAR)
	{
		cd_sound_load_surface (myApplet);
	}
	
	// mouse events
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	
	// keyboard events
	myData.pKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortkey,
		D_("Show/hide the Sound menu"),
		"Configuration", "shortkey",
		(CDBindkeyHandler) cd_sound_on_keybinding_pull);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	// mouse events
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	
	// keyboard events
	cd_keybinder_unbind (myData.pKeyBinding);
	
	cd_indicator_destroy (myData.pIndicator);
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	if (myConfig.iVolumeEffect == VOLUME_EFFECT_ICONS)
		cd_indicator_reload_icon (myData.pIndicator);  // on remet l'icone (si avant on n'avait pas d'icone, on a mis un chemin. Il ne prendra pas en compte un changement de theme d'icone, donc on remet l'icone originale).
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		
		if (myConfig.iVolumeEffect == VOLUME_EFFECT_GAUGE)
		{
			CairoDataRendererAttribute *pRenderAttr = NULL;  // les attributs du data-renderer global.
			CairoGaugeAttribute attr;  // les attributs de la jauge.
			memset (&attr, 0, sizeof (CairoGaugeAttribute));
			pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
			pRenderAttr->cModelName = "gauge";
			pRenderAttr->iRotateTheme = myConfig.iRotateTheme;
			attr.cThemePath = myConfig.cGThemePath;

			if (cairo_dock_get_icon_data_renderer (myIcon))
				CD_APPLET_RELOAD_MY_DATA_RENDERER (pRenderAttr);
			else
				CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON (pRenderAttr);
		}
		else if (myConfig.iVolumeEffect == VOLUME_EFFECT_BAR)
		{
			cd_sound_load_surface (myApplet);
			/*if (myData.iCurrentState == MUTED)
			{
				CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cMuteIcon, MY_APPLET_SHARE_DATA_DIR"/mute.svg");
				CD_APPLET_REDRAW_MY_ICON;
			}
			else
			{
				double fVolume = volume_widget_get_current_volume (myData.volume_widget);
				CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_BAR (myData.pSurface, fVolume * .01);
			}*/
		}
		
		update_accessible_desc (myApplet);
		
		cd_keybinder_rebind (myData.pKeyBinding, myConfig.cShortkey, NULL);
	}
CD_APPLET_RELOAD_END
