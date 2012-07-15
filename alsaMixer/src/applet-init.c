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

#include "applet-struct.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-generic.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN ("AlsaMixer",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This applet lets you control the sound volume from the dock.\n"
	"Scroll up/down on the icon to increase/decrease the volume.\n"
	"Click on icon to show/hide the volume control (you can bind a keyboard shortcut for it)\n"
	"Middle-click to set or unset to mute, double-click to raise the channels mixer.\n"
	"The applet can either use the Ubuntu Sound-menu or the Alsa driver."),
	"Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	CD_APPLET_ALLOW_EMPTY_TITLE
	CD_APPLET_REDEFINE_TITLE (N_("Sound Control"))
	pInterface->load_custom_widget = cd_mixer_load_custom_widget;
CD_APPLET_DEFINE_END


static gboolean _cd_mixer_on_enter (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	gpointer data)
{
	if (myData.pScale && myDesklet && myDesklet->container.iHeight > 64)
	{
		gtk_widget_show (myData.pScale);
	}
}
gboolean _cd_mixer_on_leave (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	gpointer data)
{
	if (myData.pScale && myDesklet && myDesklet->container.iHeight > 64)
	{
		if (! myDesklet->container.bInside)
			gtk_widget_hide (myData.pScale);
	}
}

static void _set_data_renderer (void)
{
	switch (myConfig.iVolumeEffect)
	{
		case VOLUME_EFFECT_GAUGE:
		{
			CairoDataRendererAttribute *pRenderAttr;  // les attributs du data-renderer global.
			CairoGaugeAttribute attr;  // les attributs de la jauge.
			memset (&attr, 0, sizeof (CairoGaugeAttribute));
			pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
			pRenderAttr->cModelName = "gauge";
			pRenderAttr->iRotateTheme = myConfig.iRotateTheme;
			attr.cThemePath = myConfig.cGThemePath;
			
			CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON (pRenderAttr);
		}
		break;
		case VOLUME_EFFECT_BAR:
		{
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cDefaultIcon, "default.svg");
			
			CairoDataRendererAttribute *pRenderAttr;  // les attributs du data-renderer global.
			CairoProgressBarAttribute attr;
			memset (&attr, 0, sizeof (CairoProgressBarAttribute));
			pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
			pRenderAttr->cModelName = "progressbar";
			pRenderAttr->iRotateTheme = myConfig.iRotateTheme;

			CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON (pRenderAttr);
		}
		break;
	}
}

CD_APPLET_INIT_BEGIN
	// set a desklet renderer
	if (myDesklet)
	{
		int iScaleWidth = (myDesklet->container.iHeight > 64 ? 15 : 0);
		gpointer pConfig[4] = {GINT_TO_POINTER (0), GINT_TO_POINTER (0), GINT_TO_POINTER (iScaleWidth), GINT_TO_POINTER (iScaleWidth)};
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Simple", pConfig);
		
		// scale widget visibility in desklet
		if (myConfig.bHideScaleOnLeave)
		{
			g_signal_connect (G_OBJECT (myDesklet->container.pWidget),
				"enter-notify-event",
				G_CALLBACK (_cd_mixer_on_enter),
				NULL);
			g_signal_connect (G_OBJECT (myDesklet->container.pWidget),
				"leave-notify-event",
				G_CALLBACK (_cd_mixer_on_leave),
				NULL);
		}
	}
	
	// data renderer
	_set_data_renderer ();
	
	// start the sound controler
	cd_start ();
	
	// mouse events
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_REGISTER_FOR_DOUBLE_CLICK_EVENT;
	
	// keyboard events
	myData.cKeyBinding = CD_APPLET_BIND_KEY (myConfig.cShortcut,
		D_("Show/hide the Sound menu"),  //  if no sound service, it's just a dialog though ...
		"Configuration", "shortkey",
		(CDBindkeyHandler) mixer_on_keybinding_pull);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ mouse events.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_UNREGISTER_FOR_DOUBLE_CLICK_EVENT;
	
	// keyboard events
	cd_keybinder_unbind (myData.cKeyBinding);
	
	// stop the current controler.
	cd_stop ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge le mixer si necessaire.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet)
		{
			int iScaleWidth = (myDesklet->container.iHeight > 64 ? 15 : 0);
			gpointer pConfig[4] = {GINT_TO_POINTER (0), GINT_TO_POINTER (0), GINT_TO_POINTER (iScaleWidth), GINT_TO_POINTER (iScaleWidth)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Simple", pConfig);
		}
		
		if (myConfig.iVolumeDisplay != VOLUME_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL);
		
		// reload the data renderer
		_set_data_renderer ();
		
		// reload the controler
		cd_reload ();
		
		// shortkey
		cd_keybinder_rebind (myData.cKeyBinding, myConfig.cShortcut, NULL);
		
		// scale
		if (myDesklet)
		{
			if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED)
			{
				cairo_dock_dialog_unreference (myData.pDialog);
				myData.pDialog = NULL;

				GtkWidget *box = _gtk_hbox_new (0);
				myData.pScale = mixer_build_widget (FALSE);
				gtk_box_pack_end (GTK_BOX (box), myData.pScale, FALSE, FALSE, 0);
				gtk_widget_show_all (box);
				gtk_container_add (GTK_CONTAINER (myDesklet->container.pWidget), box);
				if (myConfig.bHideScaleOnLeave && ! myDesklet->container.bInside)
					gtk_widget_hide (myData.pScale);
			}
			
			gulong iOnEnterCallbackID = g_signal_handler_find (myDesklet->container.pWidget,
				G_SIGNAL_MATCH_FUNC,
				0,
				0,
				NULL,
				_cd_mixer_on_enter,
				NULL);
			if (myConfig.bHideScaleOnLeave && iOnEnterCallbackID <= 0)
			{
				g_signal_connect (G_OBJECT (myDesklet->container.pWidget),
					"enter-notify-event",
					G_CALLBACK (_cd_mixer_on_enter),
					NULL);
				g_signal_connect (G_OBJECT (myDesklet->container.pWidget),
					"leave-notify-event",
					G_CALLBACK (_cd_mixer_on_leave),
					NULL);
			}
			else if (! myConfig.bHideScaleOnLeave && iOnEnterCallbackID > 0)
			{
				g_signal_handler_disconnect (G_OBJECT (myDesklet->container.pWidget), iOnEnterCallbackID);
				gulong iOnLeaveCallbackID = g_signal_handler_find (myDesklet->container.pWidget,
					G_SIGNAL_MATCH_FUNC,
					0,
					0,
					NULL,
					_cd_mixer_on_leave,
					NULL);
				g_signal_handler_disconnect (G_OBJECT (myDesklet->container.pWidget), iOnLeaveCallbackID);
			}
		}
		else
		{
			if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED && myData.pScale)
			{
				gtk_widget_destroy (myData.pScale);
				myData.pScale = NULL;
			}
			
			if (myIcon->cName == NULL)
			{
				CD_APPLET_SET_NAME_FOR_MY_ICON (myData.mixer_card_name);
			}
		}
	}
	else
	{
		///\_______________ On redessine notre icone.
		if (myDesklet && myDesklet->container.iHeight <= 64)
			gtk_widget_hide (myData.pScale);
		
		/**if (myConfig.iVolumeEffect != VOLUME_EFFECT_NONE)
			CD_APPLET_RELOAD_MY_DATA_RENDERER (NULL);
		
		cd_update_icon ();*/
	}
CD_APPLET_RELOAD_END
