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
#include "applet-mixer.h"
#include "applet-draw.h"
#include "applet-init.h"


CD_APPLET_DEFINE_BEGIN ("AlsaMixer",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_APPLET_SYSTEM,
	N_("This applet lets you control the sound volume from the dock.\n"
	"Scroll up/down on the icon to increase/decrease the volume.\n"
	"Click on icon to show/hide volume control (you can bind a keyboard shortcut for it)\n"
	"You can also hide the dialog by clicking on it.\n"
	"Middle-click to set or unset to mute, double-click to raise the channels mixer.\n"
	"This applet works with the Alsa sound driver."),
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

CD_APPLET_INIT_BEGIN
	// scale widget visibility in desklet
	if (myDesklet)
	{
		int iScaleWidth = (myDesklet->container.iHeight > 64 ? 15 : 0);
		gpointer pConfig[4] = {GINT_TO_POINTER (0), GINT_TO_POINTER (0), GINT_TO_POINTER (iScaleWidth), GINT_TO_POINTER (iScaleWidth)};
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Simple", pConfig);
		
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
	else
	{
		mixer_load_surfaces ();
	}
	
	// listen to the sound card
	mixer_init (myConfig.card_id);
	
	mixer_get_controlled_element ();
	
	if (myData.pControledElement == NULL)
	{
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cBrokenIcon, "broken.svg");
	}
	else
	{
		if (myDesklet)
		{
			GtkWidget *box = gtk_hbox_new (FALSE, 0);
			myData.pScale = mixer_build_widget (FALSE);
			gtk_box_pack_end (GTK_BOX (box), myData.pScale, FALSE, FALSE, 0);
			gtk_container_add (GTK_CONTAINER (myDesklet->container.pWidget), box);
			gtk_widget_show_all (box);
			
			if (myConfig.bHideScaleOnLeave && ! myDesklet->container.bInside)
				gtk_widget_hide (myData.pScale);
			g_signal_connect (G_OBJECT (myDesklet->container.pWidget),
				"enter-notify-event",
				G_CALLBACK (_cd_mixer_on_enter),
				NULL);
			g_signal_connect (G_OBJECT (myDesklet->container.pWidget),
				"leave-notify-event",
				G_CALLBACK (_cd_mixer_on_leave),
				NULL);
		}
		
		mixer_element_update_with_event (myData.pControledElement, 1);
		myData.iSidCheckVolume = g_timeout_add (1000, (GSourceFunc) mixer_check_events, (gpointer) NULL);
	}
	
	// mouse events
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_REGISTER_FOR_DOUBLE_CLICK_EVENT;
	
	// keyboard events
	cd_keybinder_bind (myConfig.cShortcut, (CDBindkeyHandler) mixer_on_keybinding_pull, (gpointer)NULL);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_UNREGISTER_FOR_DOUBLE_CLICK_EVENT;
	
	//\_________________ On stoppe le timer.
	if (myData.iSidCheckVolume != 0)
	{
		g_source_remove (myData.iSidCheckVolume);
		myData.iSidCheckVolume = 0;
	}
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myConfig.iVolumeEffect != VOLUME_EFFECT_GAUGE)
		mixer_load_surfaces ();
	
	//\_______________ On recharge le mixer si necessaire.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet)
		{
			int iScaleWidth = (myDesklet->container.iHeight > 64 ? 15 : 0);
			gpointer pConfig[4] = {GINT_TO_POINTER (0), GINT_TO_POINTER (0), GINT_TO_POINTER (iScaleWidth), GINT_TO_POINTER (iScaleWidth)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Simple", pConfig);
		}
		
		if (myData.iSidCheckVolume != 0)
		{
			g_source_remove (myData.iSidCheckVolume);
			myData.iSidCheckVolume = 0;
		}
		
		mixer_stop ();
		g_free (myData.cErrorMessage);
		myData.cErrorMessage = NULL;
		g_free (myData.mixer_card_name);
		myData.mixer_card_name = NULL;
		g_free (myData.mixer_device_name);
		myData.mixer_device_name= NULL;
		
		if (myConfig.iVolumeDisplay != VOLUME_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL);
		
		mixer_init (myConfig.card_id);
		mixer_get_controlled_element ();
		
		if (myData.pControledElement == NULL)
		{
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cBrokenIcon, "broken.svg");
		}
		else
		{
			if (myConfig.iVolumeEffect == VOLUME_EFFECT_GAUGE)
			{
				CairoDataRendererAttribute *pRenderAttr = NULL;  // les attributs du data-renderer global.
				CairoGaugeAttribute attr;  // les attributs de la jauge.
				memset (&attr, 0, sizeof (CairoGaugeAttribute));
				pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
				pRenderAttr->cModelName = "gauge";
				attr.cThemePath = myConfig.cGThemePath;
				
				if (cairo_dock_get_icon_data_renderer (myIcon))
					CD_APPLET_RELOAD_MY_DATA_RENDERER (pRenderAttr);
				else
					CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON (pRenderAttr);
			}
			
			mixer_element_update_with_event (myData.pControledElement, 1);
			if (myData.iSidCheckVolume == 0)
				myData.iSidCheckVolume = g_timeout_add (1000, (GSourceFunc) mixer_check_events, (gpointer) NULL);
		}
		
		cd_keybinder_bind (myConfig.cShortcut, (CDBindkeyHandler) mixer_on_keybinding_pull, (gpointer)NULL);
		
		if (myDesklet)
		{
			if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED)
			{
				cairo_dock_dialog_unreference (myData.pDialog);
				myData.pDialog = NULL;
				
				GtkWidget *box = gtk_hbox_new (FALSE, 0);
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
	}
	else
	{
		///\_______________ On redessine notre icone.
		if (myData.pControledElement != NULL)
		{
			mixer_element_update_with_event (myData.pControledElement, 0);
		}
		
		if (myDesklet && myDesklet->container.iHeight <= 64)
			gtk_widget_hide (myData.pScale);
		
		CD_APPLET_RELOAD_MY_DATA_RENDERER (NULL);
	}
CD_APPLET_RELOAD_END
