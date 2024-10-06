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
#include <libxklavier/xklavier.h>
#include <gdk/gdkx.h>

#include "applet-config.h"
#include "applet-xklavier.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-init.h"


CD_APPLET_DEFINITION2 (N_("keyboard indicator"),
	CAIRO_DOCK_MODULE_SUPPORTS_X11,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This applet lets you control the keyboard layout.\n\
	It can also display the current num and caps lock.\n\
	Left-click to switch to the next layout\n\
	Scroll up/down to select the previous/next layout\n\
	Right-click gives you access to the list of available layouts."),
	"Fabounet (Fabrice Rey)")

static void _load_bg_image (void)
{
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	cairo_dock_unload_image_buffer (&myData.bgImage);
	cairo_dock_load_image_buffer (&myData.bgImage,
		myConfig.cBackgroundImage,
		iWidth,
		iHeight,
		0);
}

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
static gboolean _init (gpointer data)
{
	cd_debug ("INIT XKBD");
	g_return_val_if_fail (myApplet != NULL, FALSE);
	Display *pDisplay = gdk_x11_get_default_xdisplay ();
	cd_xkbd_init (pDisplay);
	return FALSE;
}

static gboolean on_style_changed (GldiModuleInstance *myApplet)
{
	cd_debug ("Keyboard Indic: style is changing");
	
	if (myConfig.textDescription.cFont == NULL)  // default font -> reload our text description
	{
		gldi_text_description_set_font (&myConfig.textDescription, NULL);
		myConfig.textDescription.iSize = (int) myIcon->image.iHeight * myConfig.fTextRatio;
	}
	cd_xkbd_update_icon (NULL, NULL, TRUE);  // redraw in case the font or the text color has changed
	
	return GLDI_NOTIFICATION_LET_PASS;
}

CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
	}
	
	myConfig.textDescription.iSize = (int) myIcon->image.iHeight * myConfig.fTextRatio;
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	gldi_object_register_notification (&myDesktopMgr,
		NOTIFICATION_KBD_STATE_CHANGED,
		(GldiNotificationFunc) cd_xkbd_keyboard_state_changed,
		GLDI_RUN_AFTER,
		myApplet);
	gldi_object_register_notification (&myStyleMgr,
		NOTIFICATION_STYLE_CHANGED,
		(GldiNotificationFunc) on_style_changed,
		GLDI_RUN_AFTER, myApplet);
	
	// shortkey
	myData.pKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortkey,
		D_("Switch keyboard language"),
		"Configuration", "shortkey",
		(CDBindkeyHandler) cd_xkbd_on_keybinding_pull);
	
	_load_bg_image ();


	if (cairo_dock_is_loading ())
		g_timeout_add_seconds (1, _init, NULL);
	else
		_init (NULL);
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	gldi_object_remove_notification (&myDesktopMgr,
                NOTIFICATION_KBD_STATE_CHANGED,
		(GldiNotificationFunc) cd_xkbd_keyboard_state_changed,
		myApplet);
	CD_APPLET_REMOVE_TRANSITION_ON_MY_ICON;

	cd_xkbd_stop ();

	gldi_object_unref (GLDI_OBJECT(myData.pKeyBinding));
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	myConfig.textDescription.iSize = (int) myIcon->image.iHeight * myConfig.fTextRatio;
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		
		CD_APPLET_REMOVE_TRANSITION_ON_MY_ICON;  // prudence.
		_load_bg_image ();

		// emblems
		gboolean bCustomEmblems = (myConfig.cEmblemCapsLock || myConfig.cEmblemNumLock); // has emblem
		if (myData.cEmblemCapsLock || myData.cEmblemNumLock)  // current emblems may have changed -> reset them
		{
			g_free (myData.cEmblemCapsLock);
			g_free (myData.cEmblemNumLock);
			myData.cEmblemCapsLock = NULL;
			myData.cEmblemNumLock = NULL;
			bCustomEmblems = TRUE; // had emblem
		}
		if (!myConfig.bShowKbdIndicator || bCustomEmblems)  // no more indicators or emblems need to be reloaded
		{
			CD_APPLET_REMOVE_OVERLAY_ON_MY_ICON (CAIRO_OVERLAY_UPPER_RIGHT);
			CD_APPLET_REMOVE_OVERLAY_ON_MY_ICON (CAIRO_OVERLAY_UPPER_LEFT);
			myData.iPreviousIndic = 0;
		}
		
		//\_____________ On declenche le redessin de l'icone.
		cd_xkbd_force_redraw ();
		
		gldi_shortkey_rebind (myData.pKeyBinding, myConfig.cShortkey, NULL);
	}
	else
	{
		cd_xkbd_force_redraw ();
	}
CD_APPLET_RELOAD_END
